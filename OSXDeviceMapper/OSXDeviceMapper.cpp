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

OSDefineMetaClassAndStructors(OSXDeviceMapperClass, IOService)

#define super IOService

bool OSXDeviceMapperClass::init(OSDictionary *dict)
{
  if (super::init(dict)) {
    IOLog("Initializing driver\n");
    m_vnodeLoaded = false;
    m_vnodeDisk = NULL;
    return true;
  } else {
    IOLog("Unsucessfuly initialised parent\n");
    return false;
  }
}

void OSXDeviceMapperClass::free(void)
{
  IOLog("Freeing the driver\n");
  super::free();
}

IOService *OSXDeviceMapperClass::probe(IOService *provider,
  SInt32 *score)
{
  IOService *result = super::probe(provider, score);
  IOLog("Probing the driver\n");
  return result;
}

bool OSXDeviceMapperClass::start(IOService *provider)
{
  IOLog("Starting the driver\n");
  if (!super::start(provider))
    return false;

  m_vnodeDisk = 
  	VNodeDiskDeviceClass::withFilePathAndBlockSizeAndBlockNum(
  		"/tmp/vnodedevice", 4096, 256
  	); // sets the reference count to 1

  if (!m_vnodeDisk)
    goto bail;

  if (!m_vnodeDisk->setupVNode())
    goto bail;

  if (!m_vnodeDisk->attach(this)) // retains both this and m_vnodeDisk (reference count + 1)
    goto bail;

  m_vnodeDisk->registerService(kIOServiceSynchronous); // makes the vnode available to upper level drivers which retain it
  m_vnodeLoaded = true;

  return true;

bail:
  if (m_vnodeDisk) {
  	m_vnodeDisk->release();
  }

  return false;
}

void OSXDeviceMapperClass::stop(IOService *provider)
{
  IOLog("Stopping the driver\n");
  ejectVNode();
  super::stop(provider);
}

void OSXDeviceMapperClass::ejectVNode() {
  IOLog("Trying to eject VNode\n");
  if (m_vnodeDisk == NULL || !m_vnodeLoaded)
    return;

  if (!m_vnodeDisk->terminate(kIOServiceRequired)) // undoes register service
    IOLog("Error when unregistering the device\n");

  // m_vnodedisk->detach(this); // this seems unecessary
  m_vnodeDisk->closeVNode();
  m_vnodeLoaded = false;
  m_vnodeDisk->release();
}
