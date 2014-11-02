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
#include "VNodeDiskController.h"

OSDefineMetaClassAndStructors(VNodeDiskControllerClass, IOService)

#define super IOService

bool VNodeDiskControllerClass::init(OSDictionary *dict)
{
  m_VNodeLibraryByPath = NULL;
  if (super::init(dict)) {
    IOLog("Initializing VNodeDisk controller\n");

    m_VNodeLibraryByPath = OSDictionary::withCapacity(1);
    if (!m_VNodeLibraryByPath)
      return false;

    return true;
  } else {
    IOLog("Unsucessfuly initialised parent\n");
    return false;
  }
}

void VNodeDiskControllerClass::free(void)
{
  IOLog("Freeing the VNodeDisk controller\n");
  super::free();
}

IOService *VNodeDiskControllerClass::probe(IOService *provider,
  SInt32 *score)
{
  IOService *result = super::probe(provider, score);
  IOLog("Probing the VNodeDisk controller\n");
  return result;
}

bool VNodeDiskControllerClass::start(IOService *provider)
{
  IOLog("Starting the VNodeDisk controller\n");
  if (!super::start(provider))
    return false;
  return true;
}

void VNodeDiskControllerClass::stop(IOService *provider)
{
  IOLog("Stopping the VNodeDisk controller\n");
  super::stop(provider);
}

bool VNodeDiskControllerClass::createVNodeWithFilePathAndBlockSizeAndBlockNum(
    const char * filePath, UInt64 blockSize, UInt64 blockNum)
{
  OSString * filePathObject = NULL;
  VNodeDiskDeviceClass * vnodeDisk = NULL;

  IOLog("Adding VNode with file path %s, block size %llu, block num %llu\n", 
    filePath, blockSize, blockNum);

  // TODO: Check if file path is already used

  filePathObject = OSString::withCString(filePath);
  if (!filePathObject)
    goto bail;

  vnodeDisk = VNodeDiskDeviceClass::withFilePathAndBlockSizeAndBlockNum(
    filePath, blockSize, blockNum);

  if (!vnodeDisk)
    goto bail;

  if (!m_VNodeLibraryByPath->setObject(filePathObject, vnodeDisk)) { // remember this is retained
    m_VNodeLibraryByPath->removeObject(filePathObject);
    goto bail;
  }

  if (!vnodeDisk->setupVNode()) {
    m_VNodeLibraryByPath->removeObject(filePathObject);
    goto bail;
  }

  if (!vnodeDisk->attach(this)) { // retains both this and vnodeDisk (reference count + 1)
    vnodeDisk->closeVNode();
    m_VNodeLibraryByPath->removeObject(filePathObject);
    goto bail;
  }

  vnodeDisk->registerService(kIOServiceSynchronous); 

  return true;

bail:
  if (filePathObject) {
    filePathObject->release();
  }

  if (vnodeDisk) {
    vnodeDisk->release();
  }

  return false;
}

bool VNodeDiskControllerClass::deleteVNodeWithFilePath(const char * filePath)
{
  OSString * filePathObject = NULL;
  VNodeDiskDeviceClass * vnodeDisk = NULL;
  IOLog("Ejecting VNode with name %s\n", filePath);

  filePathObject = OSString::withCString(filePath);
  if (!filePathObject)
    return false;

  vnodeDisk = OSDynamicCast(VNodeDiskDeviceClass, 
    m_VNodeLibraryByPath->getObject(filePathObject));

  if (!deregisterVNode(vnodeDisk)) {
    IOLog("Error when unregistering the device %s\n", filePath);
    return false;
  }

  m_VNodeLibraryByPath->removeObject(filePathObject);

  return true;
}

bool VNodeDiskControllerClass::deleteAllVNodes()
{
  OSCollectionIterator * iterator = NULL;
  OSObject * object = NULL;
  OSString * filePathObject = NULL;
  VNodeDiskDeviceClass * vnodeDisk = NULL;
  IOLog("Remove all VNodes\n");

  iterator = OSCollectionIterator::withCollection(m_VNodeLibraryByPath);
  if (!iterator)
    return false;

  while (object = iterator->getNextObject()) { // my guess objects are strings
    filePathObject = OSDynamicCast(OSString, object);
    if (!filePathObject) // TODO: If this works remove check
      return false;

    // Can't remove elements from dictionary
    vnodeDisk = OSDynamicCast(VNodeDiskDeviceClass, 
      m_VNodeLibraryByPath->getObject(filePathObject));

    if (!deregisterVNode(vnodeDisk)) {
      IOLog("Error when unregistering the device %s\n", filePathObject->getCStringNoCopy());
      return false;
    }
  }

  iterator->release();
  m_VNodeLibraryByPath->flushCollection();

  return true;
}

bool VNodeDiskControllerClass::deregisterVNode(VNodeDiskDeviceClass * vnodeDisk)
{
  if (!vnodeDisk)
    return false;

  if (!vnodeDisk->terminate(kIOServiceRequired)) // undoes register service
    return false;

  vnodeDisk->detach(this); // this seems unecessary
  vnodeDisk->closeVNode();
  vnodeDisk->release();

  return true;
}


