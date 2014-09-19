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
#include "OSXDeviceMapper.h"

OSDefineMetaClassAndStructors(com_parusinskimichal_OSXDeviceMapper, IOBlockStorageDevice);

#define super IOBlockStorageDevice

bool com_parusinskimichal_OSXDeviceMapper::init(OSDictionary *dict)
// constructor equivalent
{
    if (super::init(dict)) {
        IOLog("Initializing driver\n");
        return true;
    } else {
        IOLog("Unsucessfuly initialised parent\n");
        return false;
    }
}

void com_parusinskimichal_OSXDeviceMapper::free(void)
// destructor equivalent
{
    IOLog("Freeing the driver\n");
    super::free();
}

IOService *com_parusinskimichal_OSXDeviceMapper::probe(IOService *provider,
    SInt32 *score)
// probing the device
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    return result;
}

bool com_parusinskimichal_OSXDeviceMapper::start(IOService *provider)
// starting business logic
{
    if (super::start(provider)) {
        m_loop_file = 0;
        vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);
        int vnode_error = vnode_open(LOOPDEVICE_FILE_PATH, 0, 0, 0,
            &m_loop_file, vfs_context);
        if (vnode_error) {
            IOLog("Error when opening file %s: error %d\n",
                LOOPDEVICE_FILE_PATH, vnode_error);
            return false;
        }

        // write a buffer:
        // 1 - create a buffer
        // 2 - ask kernel to write it
        char buffer[LOOPDEVICE_BUFFER_SIZE];
        for (int i = 0; i < LOOPDEVICE_BUFFER_SIZE - 2; i++) {
            buffer[i] = 'Z';
        }
        buffer[LOOPDEVICE_BUFFER_SIZE - 2] = '\n';
        buffer[LOOPDEVICE_BUFFER_SIZE - 1] = '\0';
        caddr_t buffer_addr = (caddr_t) buffer;

        kauth_cred_t cr = vfs_context_ucred(vfs_context);
        proc_t proc = vfs_context_proc(vfs_context);
        int aresid = -1;
        int write_error = vn_rdwr(UIO_WRITE, m_loop_file, buffer_addr,
            LOOPDEVICE_BUFFER_SIZE, 0, UIO_SYSSPACE, 0, cr, &aresid, proc);
        if (write_error) {
            IOLog("Error writing to file %s: error %d\n", LOOPDEVICE_FILE_PATH, write_error);
            return false;
        } else if (aresid > 0) {
            IOLog("Some characters were not written %s\n", LOOPDEVICE_FILE_PATH);
            return false;
        }

        vfs_context_rele(vfs_context);

        return true;
    } else {
        return false;
    }
}

void com_parusinskimichal_OSXDeviceMapper::stop(IOService *provider)
// ending business logic
{
    if (m_loop_file) {
        IOLog("Closing the file node\n");
        vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);
        vnode_close(m_loop_file, 0, vfs_context);
        vfs_context_rele(vfs_context);
    }
    super::stop(provider);
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doAsyncReadWrite(IOMemoryDescriptor *buffer,
    UInt64 block, UInt64 nblks, IOStorageAttributes *attributes,
    IOStorageCompletion *completion)
{
    // TODO: Implement this function
    //       1 - Check if read or write (in buffer)
    //       2 - Check if authorised (in attributes)
    //       3 - Identify data to read or write
    //       4 - Perform transfer
    //       5 - run completion route (check IOStorageCompletion)
    IOLog("reading/writing not supported yet\n!");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doEjectMedia(void)
{
    // TODO: Not sure, probably keep it unsupported
    IOLog("ejecting media is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doFormatMedia( UInt64 byteCapacity)
{
    // TODO: Not sure probably the following
    //       1 - Check if byteCapacity <= maxSupportedByteCapacity and other
    //           potential issues
    //       2 - Format to the appropriate size (if we choose to)
    IOLog("formatting the media to the set byte capacity is not supported yet\n");
    return kIOReturnUnsupported;
}

UInt32 com_parusinskimichal_OSXDeviceMapper::doGetFormatCapacities( UInt64 *capacities,
    UInt32 capacitiesMaxCount) const
{
    // TODO: What should I do here?
    IOLog("retrieving format capacities is not supported yet\n");
    return 0;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doSynchronizeCache(void)
{
    // TODO: Probably nothing as there is no hardware cache to flush
    //       Change return value
    IOLog("flushing hardware cache is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doUnmap(IOBlockStorageDeviceExtent *extents,
    UInt32 extentsCount, UInt32 options)
{
    // TODO: Delete unused data blocks from device
    //       Not sure what this mean in terms of the underlying file
    IOLog("deleting unusued data blocks from loop device is not support yet\n");
    return kIOReturnUnsupported;
}

char * com_parusinskimichal_OSXDeviceMapper::getAdditionalDeviceInfoString(void)
{
    // TODO: Find an additional information string for the loop device
    IOLog("loop device has no additional information\n");
    return "";
}

char * com_parusinskimichal_OSXDeviceMapper::getProductString(void)
{
    // TODO: Find a product string for the loop device
    IOLog("loop device has no product string\n");
    return "";
}

char * com_parusinskimichal_OSXDeviceMapper::getRevisionString(void)
{
    // TODO: Find a product revision string for the loop device
    // TODO: Find what is a product revision string
    IOLog("loop device has no revision string\n");
    return "";
}

char * com_parusinskimichal_OSXDeviceMapper::getVendorString(void)
{
    // TODO: Find a vendor string for the loop device
    IOLog("loop device has no vendor string\n");
    return "";
}

IOReturn com_parusinskimichal_OSXDeviceMapper::getWriteCacheState(bool *enabled)
{
    // TODO: Determine if a write cache is required
    IOLog("cache writing is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportBlockSize(UInt64 *blockSize)
{
    // TODO: Return the block size of the loop device
    IOLog("reporting the block size of the loop device is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportEjectability(bool *isEjectable)
{
    // TODO: Indicate the device is not ejectable
    IOLog("reporting whether loop device is ejectable is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportMaxValidBlock(UInt64 *maxBlock)
{
    // TODO: Report maximum valid block for the loop device
    IOLog("reporting maximum valid block for the loop device is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportMediaState(bool *mediaPresent,
    bool *changedState)
{
    // TODO: Implement this function.
    //       For a loop device this function is irrelevant (no media can be present)
    IOLog("reporting media state is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportRemovability(bool *isRemovable)
{
    // TODO: Investigate whether being removable make sense
    //       Probably yes
    IOLog("reporting removability of loop device is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportWriteProtection(bool *isWriteProtected)
{
    // TODO: Report whether the loop device is write only
    IOLog("reporting whether the loop device is write only is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::requestIdle(void)
{
    // TODO: Make the devce idle - Not valid for a loop device
    IOLog("requesting to make the device idle is not supported yet\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::setWriteCacheState(bool enabled)
{
    // TODO: Determine if a write cache is required
    return kIOReturnUnsupported;
}
