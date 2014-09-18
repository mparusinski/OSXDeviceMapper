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

#ifndef _V_NODE_DISK_DEVICE_
#define _V_NODE_DISK_DEVICE_

#include <sys/fcntl.h>
#include <sys/vnode.h>

#include <IOKit/IOService.h>
#include <IOKit/storage/IOBlockStorageDevice.h>

#define LOOPDEVICE_FILE_PATH "/tmp/vnodedevice"
#define LOOPDEVICE_BLOCK_SIZE 32
#define LOOPDEVICE_BLOCK_NUM 16

// TODO: Add a project defines header and rename the class to something more appropriate
#define DEVELOPER "Michal Parusinski"
#define PROJECT "OSXDeviceMapper"
#define COMPONENT "VNodeDiskDevice"
#define VERSION "0.1"

class com_parusinskimichal_VNodeDiskDevice : public IOBlockStorageDevice
{
    OSDeclareDefaultStructors(com_parusinskimichal_VNodeDiskDevice)

public:
    virtual bool init(OSDictionary *dictionary = 0);

    virtual void free(void);

    virtual IOService *probe(IOService *provider, SInt32 *score);

    virtual bool start(IOService *provider);

    virtual void stop(IOService *provider);

    virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer,
        UInt64 block, UInt64 nblks, IOStorageAttributes *attributes,
        IOStorageCompletion *completion);

    virtual IOReturn doEjectMedia(void);

    virtual IOReturn doFormatMedia( UInt64 byteCapacity);

    virtual UInt32 doGetFormatCapacities( UInt64 *capacities,
        UInt32 capacitiesMaxCount) const;

    virtual IOReturn doSynchronizeCache(void);

    virtual IOReturn doUnmap( IOBlockStorageDeviceExtent *extents,
        UInt32 extentsCount, UInt32 options = 0);

    virtual char * getAdditionalDeviceInfoString(void);

    virtual char * getProductString(void);

    virtual char * getRevisionString(void);

    virtual char * getVendorString(void);

    virtual IOReturn getWriteCacheState(bool *enabled);

    virtual IOReturn reportBlockSize(UInt64 *blockSize);

    virtual IOReturn reportEjectability(bool *isEjectable);

    virtual IOReturn reportMaxValidBlock(UInt64 *maxBlock);

    virtual IOReturn reportMediaState(bool *mediaPresent, bool *changedState = 0);

    virtual IOReturn reportRemovability(bool *isRemovable);

    virtual IOReturn reportWriteProtection(bool *isWriteProtected);

    virtual IOReturn requestIdle(void);

    virtual IOReturn setWriteCacheState(bool enabled);

private:
    struct vnode * m_loop_file;

};

#endif  // _V_NODE_DISK_DEVICE_
