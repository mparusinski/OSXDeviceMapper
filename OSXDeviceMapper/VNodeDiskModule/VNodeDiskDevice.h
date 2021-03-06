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

#include <IOKit/storage/IOBlockStorageDevice.h>

// TODO: Add a project defines header and rename the class to something more appropriate
#define COMPONENT "VNodeDiskDevice"

#define VNodeDiskDeviceClass com_parusinskimichal_VNodeDiskDevice

class VNodeDiskDeviceClass : public IOBlockStorageDevice
{
  OSDeclareDefaultStructors(VNodeDiskDeviceClass)

public:
  virtual bool init(OSDictionary *dictionary = 0);

  virtual void free(void);

  virtual IOService *probe(IOService *provider, SInt32 *score);

  virtual bool start(IOService *provider);

  virtual void stop(IOService *provider);

  bool setupVNode();

  void closeVNode();

  static VNodeDiskDeviceClass * withFilePathAndBlockSizeAndBlockNum(
    const char * filePath, const UInt64 blockSize, const UInt64 blockNum);

  virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt64 block, 
    UInt64 nblks, IOStorageAttributes *attributes, 
    IOStorageCompletion *completion);

  virtual IOReturn doEjectMedia(void);

  virtual IOReturn doFormatMedia( UInt64 byteCapacity);

  virtual UInt32 doGetFormatCapacities( UInt64 *capacities,
    UInt32 capacitiesMaxCount) const;

  virtual IOReturn doSynchronizeCache(void);

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

  virtual IOReturn setWriteCacheState(bool enabled);

private:
  struct vnode * m_vnode;
  OSString * m_filePath;
  char * m_additionalInformation;
  char * m_productString;
  char * m_revisionString;
  char * m_vendorString;
  UInt64 m_blockSize;
  UInt64 m_blockNum;

};

#endif  // _V_NODE_DISK_DEVICE_
