/*

 Copyright (c) 2014, Michal Parusinski
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <IOKit/IOLib.h>
#include <IOKit/IOReturn.h>
#include <sys/types.h>
#include "OSXDeviceMapper.h"

#include "VNodeDiskDevice.h"

OSDefineMetaClassAndStructors(com_parusinskimichal_VNodeDiskDevice, IOBlockStorageDevice)

#define super IOBlockStorageDevice

bool com_parusinskimichal_VNodeDiskDevice::init(OSDictionary *dict)
// constructor equivalent
{
    if (super::init(dict)) {
        IOLog("Initializing device\n");
        m_additional_information = (char *) "";
        m_product_string = (char *) (PROJECT "_" COMPONENT);
        m_revision_string = (char *) VERSION;
        m_vendor_string = (char *) DEVELOPER;
        m_loop_file = NULL;
        return true;
    } else {
        IOLog("Unsucessfuly initialised parent\n");
        return false;
    }
}

void com_parusinskimichal_VNodeDiskDevice::free(void)
// destructor equivalent
{
    IOLog("Freeing the device\n");
    super::free();
}

IOService *com_parusinskimichal_VNodeDiskDevice::probe(IOService *provider,
    SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing the device\n");
    return result;
}

bool com_parusinskimichal_VNodeDiskDevice::setupVNode() {
   int vap_error = -1;
   struct vnode_attr vap;

   if (m_loop_file != NULL)
       return true;

   vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);

   int vnode_error = vnode_open(LOOPDEVICE_FILE_PATH, (FREAD | FWRITE), 0, 0,
       &m_loop_file, vfs_context);

   if (vnode_error || m_loop_file == NULL) {
       IOLog("Error when opening file %s: error %d\n",
           LOOPDEVICE_FILE_PATH, vnode_error);
       goto failure;
   }
   
   if (!vnode_isreg(m_loop_file)) {
       IOLog("Error when opening file %s: not a regular file\n", LOOPDEVICE_FILE_PATH);
       vnode_close(m_loop_file, (FREAD | FWRITE), vfs_context);
       goto failure;
   }

   VATTR_INIT(&vap);
   VATTR_WANTED(&vap, va_data_size);
   vap_error = vnode_getattr(m_loop_file, &vap, vfs_context);
   if (vap_error) {
       IOLog("Error when retrieving vnode's attributes with error code %d\n", vap_error);
       goto failure;
   }

   if (vap.va_data_size < LOOPDEVICE_BLOCK_SIZE * LOOPDEVICE_BLOCK_NUM) {
       IOLog("Error file %s is too small, actual size is %llu\n", LOOPDEVICE_FILE_PATH, vap.va_data_size);
       goto failure;
   }

   vfs_context_rele(vfs_context);
   return true;

failure:
   vfs_context_rele(vfs_context);
   return false;
}

bool com_parusinskimichal_VNodeDiskDevice::start(IOService *provider)
{
    IOLog("Starting the device\n");
    if (!super::start(provider))
        return false;

    return true;
}


void com_parusinskimichal_VNodeDiskDevice::closeVNode() {
   if (m_loop_file != NULL) {
       IOLog("Closing the file node\n");
       vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);
       vnode_close(m_loop_file, 0, vfs_context);
       vfs_context_rele(vfs_context);
       m_loop_file = NULL;
   }
}

void com_parusinskimichal_VNodeDiskDevice::stop(IOService *provider)
{
    IOLog("Stopping the device\n");
    super::stop(provider);
}

IOReturn com_parusinskimichal_VNodeDiskDevice::doAsyncReadWrite(IOMemoryDescriptor *buffer,
    UInt64 block, UInt64 nblks, IOStorageAttributes *attributes,
    IOStorageCompletion *completion)
{    
   if (m_loop_file == NULL)
       return kIOReturnIOError;

   IOReturn returnMessage = kIOReturnSuccess;

   if (block >= LOOPDEVICE_BLOCK_NUM) {
       IOLog("Attempting to write outside vnode disk\n");
       return kIOReturnOverrun;
   }

   IODirection direction = buffer->getDirection();
   if ((direction != kIODirectionIn) && (direction != kIODirectionOut)) {
       IOLog("No valid direction of transfer: required either in or out\n");
       kIOReturnIOError;
   }

   UInt64 real_nblks = nblks;
   if ((block + real_nblks - 1) >= LOOPDEVICE_BLOCK_NUM) {
       IOLog("Adjusting block number of written\n");
       real_nblks = LOOPDEVICE_BLOCK_NUM - block;
   }
   IOByteCount actualByteCount = real_nblks * LOOPDEVICE_BLOCK_SIZE;
   off_t byteOffset = block * LOOPDEVICE_BLOCK_SIZE;

   vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);
   proc_t proc = vfs_context_proc(vfs_context);
   kauth_cred_t cr = vfs_context_ucred(vfs_context);

   int aresid = -1;

   char * raw_buffer = NULL;
   raw_buffer = (char *) IOMalloc(sizeof(char) * actualByteCount);
   if (raw_buffer == NULL) {
       IOLog("Unable to allocate buffer\n");
       goto cleanup;
   };

   if (direction == kIODirectionIn) {
       IOLog("Reading from disk\n");

       // TODO: Remove warning (unsigned long long) -> int
       int read_error = vn_rdwr(UIO_READ, m_loop_file, (caddr_t) raw_buffer,
           (int) actualByteCount, byteOffset, UIO_SYSSPACE, 0, cr, &aresid, proc);

       IOLog("Data was read\n");

       if (read_error || aresid > 0) {
           returnMessage = kIOReturnIOError;
           goto cleanup;
       }

       buffer->writeBytes(block * LOOPDEVICE_BLOCK_SIZE, raw_buffer, actualByteCount);

   } else { // (direction == kIODirectionOut)
       IOLog("Writing to disk\n");

        buffer->readBytes(block * LOOPDEVICE_BLOCK_SIZE, raw_buffer, actualByteCount); // first arg is offset

        // TODO: Remove warning (unsigned long long) -> int
       int write_error = vn_rdwr(UIO_WRITE, m_loop_file, (caddr_t) raw_buffer,
           (int) actualByteCount, byteOffset, UIO_SYSSPACE, 0, cr, &aresid, proc);

       if (write_error || aresid > 0)
           goto cleanup;
   }

cleanup:
   vfs_context_rele(vfs_context);
   if (raw_buffer)
       IOFree(raw_buffer, sizeof(char) * actualByteCount);

   actualByteCount = actualByteCount > aresid ? actualByteCount - aresid : 0;

   completion->action(completion->target, completion->parameter, kIOReturnSuccess, actualByteCount);
   return returnMessage;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::doEjectMedia(void)
{
    // Should this function not operate as expected one can try
    com_parusinskimichal_OSXDeviceMapper *  kext_driver = (com_parusinskimichal_OSXDeviceMapper *) getProvider();
    kext_driver->ejectVNode();
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::doFormatMedia( UInt64 byteCapacity)
{
    // Media is not formatted for now
    IOLog("formatting media is not supported\n");
    return kIOReturnSuccess; // TODO: This might cause an issue
}

UInt32 com_parusinskimichal_VNodeDiskDevice::doGetFormatCapacities( UInt64 *capacities,
    UInt32 capacitiesMaxCount) const
{
    IOLog("Checking block capacity\n");
    if (capacities == NULL || capacitiesMaxCount < 1) {
        IOLog("Array of capacities is empty\n");
        return 0;
    }

    capacities[0] = LOOPDEVICE_BLOCK_NUM * LOOPDEVICE_BLOCK_SIZE;

    return 1;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::doSynchronizeCache(void)
{
    // no hardware cache to flush
    IOLog("no caching supported\n");
    return kIOReturnSuccess;
}

char * com_parusinskimichal_VNodeDiskDevice::getAdditionalDeviceInfoString(void)
{
    return m_additional_information;
}

char * com_parusinskimichal_VNodeDiskDevice::getProductString(void)
{
    return m_product_string;
}

char * com_parusinskimichal_VNodeDiskDevice::getRevisionString(void)
{
    return m_revision_string;
}

char * com_parusinskimichal_VNodeDiskDevice::getVendorString(void)
{
    return m_vendor_string;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::getWriteCacheState(bool *enabled)
{
    *enabled = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportBlockSize(UInt64 *blockSize)
{
    *blockSize = LOOPDEVICE_BLOCK_SIZE;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportEjectability(bool *isEjectable)
{
    *isEjectable = true;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportMaxValidBlock(UInt64 *maxBlock)
{
    *maxBlock = LOOPDEVICE_BLOCK_NUM - 1;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportMediaState(bool *mediaPresent,
    bool *changedState)
{
    *mediaPresent = true;
    *changedState = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportRemovability(bool *isRemovable)
{
    *isRemovable = true;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::reportWriteProtection(bool *isWriteProtected)
{
    *isWriteProtected = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_VNodeDiskDevice::setWriteCacheState(bool enabled)
{
    return kIOReturnUnsupported;
}
