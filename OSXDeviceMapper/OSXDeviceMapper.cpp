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

OSDefineMetaClassAndStructors(com_parusinskimichal_OSXDeviceMapper, IOService)

#define super IOService

bool com_parusinskimichal_OSXDeviceMapper::init(OSDictionary *dict)
{
  if (super::init(dict)) {
    IOLog("Initializing driver\n");
    m_vnodeloaded = false;
    return true;
  } else {
    IOLog("Unsucessfuly initialised parent\n");
    return false;
  }
}

void com_parusinskimichal_OSXDeviceMapper::free(void)
{
  IOLog("Freeing the driver\n");
  super::free();
}

IOService *com_parusinskimichal_OSXDeviceMapper::probe(IOService *provider,
  SInt32 *score)
{
  IOService *result = super::probe(provider, score);
  IOLog("Probing the driver\n");
  return result;
}

bool com_parusinskimichal_OSXDeviceMapper::start(IOService *provider)
{
  IOLog("Starting the driver\n");
  if (!super::start(provider))
    return false;

  m_vnodedisk = NULL;
  m_vnodedisk = new com_parusinskimichal_VNodeDiskDevice; // sets the reference count to 1

  if (!m_vnodedisk)
    return false;

  OSDictionary * dictionary = 0;
  if (!m_vnodedisk->init(dictionary))
    goto bail;

  if (!m_vnodedisk->setupVNode())
    goto bail;

  if (!m_vnodedisk->attach(this)) // retains both this and m_vnodedisk (reference count + 1)
    goto bail;

  m_vnodedisk->registerService(kIOServiceSynchronous); // makes the vnode available to upper level drivers which retain it
  m_vnodeloaded = true;

  return true;
bail:
  m_vnodedisk->release();
  return false;
}

void com_parusinskimichal_OSXDeviceMapper::stop(IOService *provider)
{
  IOLog("Stopping the driver\n");
  ejectVNode();
  super::stop(provider);
}

void com_parusinskimichal_OSXDeviceMapper::ejectVNode() {
  IOLog("Trying to eject VNode\n");
  if (m_vnodedisk == NULL || !m_vnodeloaded)
    return;

  if (!m_vnodedisk->terminate(kIOServiceRequired)) // undoes register service
    IOLog("Error when unregistering the device\n");

  // m_vnodedisk->detach(this); // this seems unecessary
  m_vnodedisk->closeVNode();
  m_vnodeloaded = false;
  m_vnodedisk->release();
}
