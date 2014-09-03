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

        // // write a buffer:
        // // 1 - create a buffer
        // // 2 - ask kernel to write it
        // char buffer[LOOPDEVICE_BUFFER_SIZE];
        // for (int i = 0; i < LOOPDEVICE_BUFFER_SIZE - 2; i++) {
        //     buffer[i] = 'Z';
        // }
        // buffer[LOOPDEVICE_BUFFER_SIZE - 2] = '\n';
        // buffer[LOOPDEVICE_BUFFER_SIZE - 1] = '\0';
        // caddr_t buffer_addr = (caddr_t) buffer;
        //
        // kauth_cred_t cr = vfs_context_ucred(vfs_context);
        // proc_t proc = vfs_context_proc(vfs_context);
        // int aresid = -1;
        // int write_error = vn_rdwr(UIO_WRITE, m_loop_file, buffer_addr,
        //     LOOPDEVICE_BUFFER_SIZE, 0, UIO_SYSSPACE, 0, cr, &aresid, proc);
        // if (write_error) {
        //     IOLog("Error writing to file %s: error %d\n", LOOPDEVICE_FILE_PATH, write_error);
        //     return false;
        // } else if (aresid > 0) {
        //     IOLog("Some characters were not written %s\n", LOOPDEVICE_FILE_PATH);
        //     return false;
        // }

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
    //       0 - Check if possible (block size checks)
    //       1 - Check if authorized
    //       2 - Check if read or write (in buffer)
    //       3 - Identify data to read or write
    //       4 - Perform transfer
    //       5 - run completion route (check IOStorageCompletion)

    if (block >= LOOPDEVICE_BLOCK_NUM) {
        return kIOReturnOverrun;
    }

    UInt64 real_nblks = nblks;

    if ((block + real_nblks) >= LOOPDEVICE_BLOCK_NUM) {
        real_nblks = LOOPDEVICE_BLOCK_NUM - block - 1;
    }

    // TODO: Figure out what to do with attributes

    IODirection direction = buffer->getDirection();
    if ((direction == kIODirectionIn) || (direction == kIODirectionOut)) {
        IOLog("No valid direction of transfer: required either in or out\n");
        kIOReturnIOError;
    }

    IOByteCount actualByteCount = real_nblks * LOOPDEVICE_BLOCK_SIZE;

    vfs_context_t vfs_context = vfs_context_create((vfs_context_t) 0);

    // TODO: These two line aren't pretty - Can they cause a risk?
    proc_t proc = vfs_context_proc(vfs_context);
    kauth_cred_t cr = vfs_context_ucred(vfs_context);

    int aresid = -1;

    // TODO: This is wrong because I need to know the offset in the file
    if (direction == kIODirectionIn) {
        // TODO: Read block from file
        char * raw_buffer[actualByteCount];

        int read_error = vn_rdwr(UIO_READ, m_loop_file, (caddr_t) raw_buffer,
            actualByteCount, 0, UIO_SYSSPACE, 0, cr, &aresid, proc);

        if (read_error) {
            IOLog("Error reading from loop device\n");
            actualByteCount = 0;
        } else if (aresid > 0) {
            // TODO: Check this
            IOLog("Some characters were not read\n");
            return kIOReturnIOError;
        } else {
            buffer->writeBytes(block * LOOPDEVICE_BLOCK_SIZE, raw_buffer, actualByteCount);
        }
    } else { // (direction == kIODirectionOut)
        // TODO: Test this thorougly
        char * raw_buffer[actualByteCount]; // this is technically unneccesary
        buffer->readBytes(block * LOOPDEVICE_BLOCK_SIZE, raw_buffer, actualByteCount); // first arg is offset

        int write_error = vn_rdwr(UIO_WRITE, m_loop_file, (caddr_t) raw_buffer,
            actualByteCount, 0, UIO_SYSSPACE, 0, cr, &aresid, proc); // TODO: Fix offset

        if (write_error) {
            IOLog("Error writing to loop device\n");
            actualByteCount = 0;
        }

        if (aresid > 0) {
            // technically this should not happen
            IOLog("Some characters have not been written\n");
            // TODO Get actual byte count and not return this
            return kIOReturnIOError;
        }
    }
    vfs_context_rele(vfs_context);

    completion->action(completion->target, completion->parameter, kIOReturnSuccess, actualByteCount);

    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doEjectMedia(void)
{
    // TODO: Not sure, probably keep it unsupported
    IOLog("ejecting media is not supported\n");
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doFormatMedia( UInt64 byteCapacity)
{
    // Media is not formatted for now
    IOLog("formatting media is not supported\n");
    return kIOReturnSuccess; // TODO: This might cause an issue
}

UInt32 com_parusinskimichal_OSXDeviceMapper::doGetFormatCapacities( UInt64 *capacities,
    UInt32 capacitiesMaxCount) const
{
    if (capacities == NULL || capacitiesMaxCount < 1) {
        return 0;
    }

    capacities[0] = LOOPDEVICE_BLOCK_NUM * LOOPDEVICE_BLOCK_SIZE;

    return 1;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doSynchronizeCache(void)
{
    // no hardware cache to flush
    IOLog("no caching supported\n");
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::doUnmap(IOBlockStorageDeviceExtent *extents,
    UInt32 extentsCount, UInt32 options)
{
    // TODO: Unsure about this! Check
    //       My guess is that the loop device will keep track of used disk blocks
    IOLog("doUnmap called (%u) extents and option %u\n", extentsCount, options);
    if (options > 0) {
        return kIOReturnUnsupported;
    }

    for (int i = 0; i < extentsCount; i++) {
        UInt64 blockStart = extents[i].blockStart;
        UInt64 blockCount = extents[i].blockCount;

        // TODO: Implement discard this extent
    }

    return kIOReturnSuccess;
}

char * com_parusinskimichal_OSXDeviceMapper::getAdditionalDeviceInfoString(void)
{
    return "";
}

char * com_parusinskimichal_OSXDeviceMapper::getProductString(void)
{
    return PROJECT "_" COMPONENT;
}

char * com_parusinskimichal_OSXDeviceMapper::getRevisionString(void)
{
    return VERSION;
}

char * com_parusinskimichal_OSXDeviceMapper::getVendorString(void)
{
    return DEVELOPER;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::getWriteCacheState(bool *enabled)
{
    *enabled = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportBlockSize(UInt64 *blockSize)
{
    // TODO: Adapt this function to the actual block size
    *blockSize = LOOPDEVICE_BLOCK_SIZE;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportEjectability(bool *isEjectable)
{
    *isEjectable = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportMaxValidBlock(UInt64 *maxBlock)
{
    *maxBlock = (LOOPDEVICE_BLOCK_NUM / LOOPDEVICE_BLOCK_SIZE) - 1;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportMediaState(bool *mediaPresent,
    bool *changedState)
{
    *mediaPresent = true;
    *changedState = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportRemovability(bool *isRemovable)
{
    // TODO: Make sure this is valid
    *isRemovable = true;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::reportWriteProtection(bool *isWriteProtected)
{
    // Loop device is read/write (read-only support not offered yet)
    *isWriteProtected = false;
    return kIOReturnSuccess;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::requestIdle(void)
{
    // Requesting a loop device to be idle is not supported (as it does not make sense)
    IOLog("unsupported function call \"requestIdle\"\n");
    return kIOReturnUnsupported;
}

IOReturn com_parusinskimichal_OSXDeviceMapper::setWriteCacheState(bool enabled)
{
    // TODO: Determine if a write cache is required
    return kIOReturnUnsupported;
}
