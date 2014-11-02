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

OSDefineMetaClassAndStructors(VNodeDiskDeviceClass, IOBlockStorageDevice)

#define super IOBlockStorageDevice

bool VNodeDiskDeviceClass::init(OSDictionary *dict)
{
  m_filePath = NULL;
  m_vnode = NULL;
  OSNumber * blockSizeObject = NULL;
  OSNumber * blockNumObject = NULL;

  if (super::init(dict) && dict != NULL) {
    IOLog("Initializing device\n");
    m_additionalInformation = (char *) "";
    m_productString = (char *) (PROJECT "_" COMPONENT);
    m_revisionString = (char *) VERSION;
    m_vendorString = (char *) DEVELOPER;

    m_filePath = OSDynamicCast(OSString, dict->getObject("VNode File Path"));
    if (!m_filePath) {
      IOLog("Missing file path parameter in VNode Disk\n");
      return false;
    }
    m_filePath->retain();

    blockSizeObject = OSDynamicCast(OSNumber, dict->getObject("Block Size"));
    if (!blockSizeObject) {
      IOLog("Missing block size parameter\n");
      return false;
    }
    m_blockSize = blockSizeObject->unsigned64BitValue();

    blockNumObject = OSDynamicCast(OSNumber, dict->getObject("Block num"));
    if (!blockNumObject) {
      IOLog("Missing block num parameter\n");
      return false;
    }
    m_blockNum = blockNumObject->unsigned64BitValue();

    return true;
  } else {
    IOLog("Unsucessfuly initialised VNode Disk Device\n");
    return false;
  }
}

void VNodeDiskDeviceClass::free(void)
{
  IOLog("Freeing the device\n");
  if (m_filePath)
    m_filePath->release();
  super::free();
}

IOService *VNodeDiskDeviceClass::probe(IOService *provider,
  SInt32 *score)
{
  IOService *result = super::probe(provider, score);
  IOLog("Probing the device\n");
  return result;
}

bool VNodeDiskDeviceClass::setupVNode() {
  int vapError = -1;
  struct vnode_attr vap;

  if (m_vnode != NULL)
    return true;

  vfs_context_t vfsContext = vfs_context_create((vfs_context_t) 0);

  int vnodeError = vnode_open(m_filePath->getCStringNoCopy(), (FREAD | FWRITE), 0, 0,
    &m_vnode, vfsContext);

  if (vnodeError || m_vnode == NULL) {
    IOLog("Error when opening file %s: error %d\n", 
      m_filePath->getCStringNoCopy(), vnodeError);
    goto failure;
  }

  if (!vnode_isreg(m_vnode)) {
    IOLog("Error when opening file %s: not a regular file\n", 
      m_filePath->getCStringNoCopy());
    vnode_close(m_vnode, (FREAD | FWRITE), vfsContext);
    goto failure;
  }

  VATTR_INIT(&vap);
  VATTR_WANTED(&vap, va_data_size);
  vapError = vnode_getattr(m_vnode, &vap, vfsContext);
  if (vapError) {
    IOLog("Error when retrieving vnode's attributes with error code %d\n", vapError);
    goto failure;
  }

  if (vap.va_data_size < m_blockSize * m_blockNum) {
    IOLog("Error file %s is too small, actual size is %llu\n", 
      m_filePath->getCStringNoCopy(), vap.va_data_size);
    goto failure;
  }

  vfs_context_rele(vfsContext);
  return true;

failure:
  vfs_context_rele(vfsContext);
  return false;
}

bool VNodeDiskDeviceClass::start(IOService *provider)
{
  IOLog("Starting the device\n");
  if (!super::start(provider))
    return false;

  return true;
}


void VNodeDiskDeviceClass::closeVNode() 
{
  if (m_vnode != NULL) {
    IOLog("Closing the file node\n");
    vfs_context_t vfsContext = vfs_context_create((vfs_context_t) 0);
    vnode_close(m_vnode, 0, vfsContext);
    vfs_context_rele(vfsContext);
    m_vnode = NULL;
  }
}

void VNodeDiskDeviceClass::stop(IOService *provider)
{
  IOLog("Stopping the device\n");
  super::stop(provider);
}

VNodeDiskDeviceClass * 
VNodeDiskDeviceClass::withFilePathAndBlockSizeAndBlockNum(
    const char * filePath, const UInt64 blockSize, const UInt64 blockNum)
{
  OSDictionary * vnodeParams = NULL;
  OSString * filePathObject = NULL;
  OSNumber * blockSizeObject = NULL;
  OSNumber * blockNumberObject = NULL;
  VNodeDiskDeviceClass * instance = NULL;

  vnodeParams  = OSDictionary::withCapacity(4);
  if (!vnodeParams)
    goto error;

  filePathObject    = OSString::withCString(filePath);
  if (!filePathObject)
    goto error;

  blockSizeObject   = OSNumber::withNumber(blockSize, 64);
  if (!blockSizeObject) 
    goto error;

  blockNumberObject = OSNumber::withNumber(blockNum, 64);
  if (!blockNumberObject)
    goto error;

  if (!vnodeParams->setObject(OSString::withCString("VNode File Path"), filePathObject))
    goto error;

  if (!vnodeParams->setObject(OSString::withCString("Block Size"), blockSizeObject))
    goto error;

  if (!vnodeParams->setObject(OSString::withCString("Block num"), blockNumberObject))
    goto error;

  instance = new VNodeDiskDeviceClass;
  if (!instance)
    goto error;

  if (!instance->init(vnodeParams))
    goto error;

  return instance;

error:
  if (!instance)
    instance->release();

  if (!blockNumberObject)
    blockNumberObject->release();

  if (!blockSizeObject)
    blockSizeObject->release();

  if (!filePathObject)
    filePathObject->release();

  if (!vnodeParams)
    vnodeParams->release();

  return NULL;
}

IOReturn VNodeDiskDeviceClass::doAsyncReadWrite(
  IOMemoryDescriptor *buffer, UInt64 block, UInt64 nblks, 
  IOStorageAttributes *attributes, IOStorageCompletion *completion)
{    
  if (m_vnode == NULL)
    return kIOReturnIOError;

  IOReturn returnMessage = kIOReturnSuccess;
  if (block >= m_blockNum) {
    IOLog("Attempting to write outside vnode disk\n");
    return kIOReturnOverrun;
  }

  IODirection direction = buffer->getDirection();
  if ((direction != kIODirectionIn) && (direction != kIODirectionOut)) {
    IOLog("No valid direction of transfer: required either in or out\n");
    kIOReturnIOError;
  }

  UInt64 realNblks = nblks;
  if ((block + realNblks - 1) >= m_blockNum) {
    IOLog("Adjusting block number of written\n");
    realNblks = m_blockNum - block;
  }

  IOByteCount actualByteCount = realNblks * m_blockSize;
  off_t byteOffset = block * m_blockSize;

  vfs_context_t vfsContext = vfs_context_create((vfs_context_t) 0);
  proc_t proc = vfs_context_proc(vfsContext);
  kauth_cred_t cr = vfs_context_ucred(vfsContext);

  int aresid = -1;

  char * rawBuffer = NULL;
  rawBuffer = (char *) IOMalloc(sizeof(char) * actualByteCount);
  if (rawBuffer == NULL) {
    IOLog("Unable to allocate buffer\n");
    goto cleanup;
  }

  if (direction == kIODirectionIn) {
    IOLog("Reading from disk\n");

    // TODO: Remove warning (unsigned long long) -> int
    int readError = vn_rdwr(UIO_READ, m_vnode, (caddr_t) rawBuffer,
      (int) actualByteCount, byteOffset, UIO_SYSSPACE, 0, cr, &aresid, proc);

    IOLog("Data was read\n");
    if (readError || aresid > 0) {
      returnMessage = kIOReturnIOError;
      goto cleanup;
    }

    buffer->writeBytes(block * m_blockSize, rawBuffer, actualByteCount);
  } else { // (direction == kIODirectionOut)
    IOLog("Writing to disk\n");

    buffer->readBytes(block * m_blockSize, rawBuffer, actualByteCount); // first arg is offset

    // TODO: Remove warning (unsigned long long) -> int
    int writeError = vn_rdwr(UIO_WRITE, m_vnode, (caddr_t) rawBuffer,
      (int) actualByteCount, byteOffset, UIO_SYSSPACE, 0, cr, &aresid, proc);

    if (writeError || aresid > 0)
      goto cleanup;
  }

cleanup:
  vfs_context_rele(vfsContext);
  if (rawBuffer)
    IOFree(rawBuffer, sizeof(char) * actualByteCount);

  actualByteCount = actualByteCount > aresid ? actualByteCount - aresid : 0;

  completion->action(completion->target, completion->parameter, kIOReturnSuccess, 
    actualByteCount);
  return returnMessage;
}

IOReturn VNodeDiskDeviceClass::doEjectMedia(void)
{
  OSXDeviceMapperClass * kextDriver = (OSXDeviceMapperClass *) getProvider();
  kextDriver->ejectVNode();
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::doFormatMedia( UInt64 byteCapacity)
{
  // Media is not formatted for now
  IOLog("formatting media is not supported\n");
  return kIOReturnSuccess; // TODO: This might cause an issue
}

UInt32 VNodeDiskDeviceClass::doGetFormatCapacities( UInt64 *capacities,
  UInt32 capacitiesMaxCount) const
{
  IOLog("Checking block capacity\n");
  if (capacities == NULL || capacitiesMaxCount < 1) {
    IOLog("Array of capacities is empty\n");
    return 0;
  }

  capacities[0] = m_blockNum * m_blockSize;
  return 1;
}

IOReturn VNodeDiskDeviceClass::doSynchronizeCache(void)
{
  // no hardware cache to flush
  IOLog("no caching supported\n");
  return kIOReturnSuccess;
}

char * VNodeDiskDeviceClass::getAdditionalDeviceInfoString(void)
{
  return m_additionalInformation;
}

char * VNodeDiskDeviceClass::getProductString(void)
{
  return m_productString;
}

char * VNodeDiskDeviceClass::getRevisionString(void)
{
  return m_revisionString;
}

char * VNodeDiskDeviceClass::getVendorString(void)
{
  return m_vendorString;
}

IOReturn VNodeDiskDeviceClass::getWriteCacheState(bool *enabled)
{
  *enabled = false;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportBlockSize(UInt64 *blockSize)
{
  *blockSize = m_blockSize;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportEjectability(bool *isEjectable)
{
  *isEjectable = true;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportMaxValidBlock(UInt64 *maxBlock)
{
  *maxBlock = m_blockNum - 1;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportMediaState(bool *mediaPresent,
    bool *changedState)
{
  *mediaPresent = true;
  *changedState = false;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportRemovability(bool *isRemovable)
{
  *isRemovable = true;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::reportWriteProtection(bool *isWriteProtected)
{
  *isWriteProtected = false;
  return kIOReturnSuccess;
}

IOReturn VNodeDiskDeviceClass::setWriteCacheState(bool enabled)
{
  return kIOReturnUnsupported;
}
