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

#ifndef _OSX_DEVICE_MAPPER_
#define _OSX_DEVICE_MAPPER_

#include <sys/fcntl.h>
#include <sys/vnode.h>

//#include <IOKit/IOService.h>
#include <IOKit/storage/IOBlockStorageDevice.h>

#define LOOPDEVICE_FILE_PATH "/tmp/loopdevice"
#define LOOPDEVICE_BUFFER_SIZE 32

class com_parusinskimichal_OSXDeviceMapper : public IOBlockStorageDevice
{
    OSDeclareDefaultStructors(com_parusinskimichal_OSXDeviceMapper);

public:
    virtual bool init(OSDictionary *dictionary = 0);

    virtual void free(void);

    virtual IOService *probe(IOService *provider, SInt32 *score);

    virtual bool start(IOService *provider);

    virtual void stop(IOService *provider);

    // TODO: Implement this
    virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer,
        UInt64 block, UInt64 nblks, IOStorageAttributes *attributes,
        IOStorageCompletion *completion);

    // TODO: Implement this
    virtual IOReturn doEjectMedia(void);

    // TODO: Implement this
    virtual IOReturn doFormatMedia( UInt64 byteCapacity);

    // TODO: Implement this
    virtual UInt32 doGetFormatCapacities( UInt64 *capacities,
        UInt32 capacitiesMaxCount) const;

    // TODO: Implement this
    virtual IOReturn doSynchronizeCache(void);

    // TODO: Implement this
    virtual IOReturn doUnmap( IOBlockStorageDeviceExtent *extents,
        UInt32 extentsCount, UInt32 options = 0);

    // TODO: Implement this
    virtual char * getAdditionalDeviceInfoString(void);

    // TODO: Implement this
    virtual char * getProductString(void);

    // TODO: Implement this
    virtual char * getRevisionString(void);

    // TODO: Implement this
    virtual char * getVendorString(void);

    // TODO: Implement this
    virtual IOReturn getWriteCacheState(bool *enabled);

    // TODO: Implement this
    virtual IOReturn reportBlockSize(UInt64 *blockSize);

    // TODO: Implement this
    virtual IOReturn reportEjectability(bool *isEjectable);

    // TODO: Implement this
    virtual IOReturn reportMaxValidBlock(UInt64 *maxBlock);

    // TODO: Implement this
    virtual IOReturn reportMediaState(bool *mediaPresent, bool *changedState = 0);

    // TODO: Implement this
    virtual IOReturn reportRemovability(bool *isRemovable);

    // TODO: Implement this
    virtual IOReturn reportWriteProtection(bool *isWriteProtected);

    // TODO: Implement this
    virtual IOReturn requestIdle(void);

    // TODO: Implement this
    virtual IOReturn setWriteCacheState(bool enabled);

private:
    struct vnode * m_loop_file;

    char [] m_productString = "OSXDeviceMapper_LoopDevice";
    char [] m_vendorString;
    char [] m_revisionString;
    char [] m_additionalInformationString;

};

#endif  // _OSX_DEVICE_MAPPER_
