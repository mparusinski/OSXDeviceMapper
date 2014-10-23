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
    IOLog("Probing the driver\n");
    return result;
}

bool com_parusinskimichal_OSXDeviceMapper::start(IOService *provider)
// starting business logic
{
    IOLog("Starting the driver\n");
    if (!super::start(provider))
        return false;

   m_vnodedisk = NULL;
   m_vnodedisk = new com_parusinskimichal_VNodeDiskDevice; // sets the reference count to 1

   if (!m_vnodedisk)
       return false;

   OSDictionary * dictionary = 0;
   if (!m_vnodedisk->init(dictionary)) {
       m_vnodedisk->release();
       return false;
   }

   if (!m_vnodedisk->setupVNode()) {
       m_vnodedisk->release();
       return false;
   }

   if (!m_vnodedisk->attach(this)) { // retains both this and m_vnodedisk (reference count + 1)
      m_vnodedisk->release();
      return false;
   }

   m_vnodedisk->registerService(kIOServiceSynchronous);

   return true;
}

void com_parusinskimichal_OSXDeviceMapper::stop(IOService *provider)
// ending business logic
{
    IOLog("Stopping the driver\n");
    ejectVNode();
    super::stop(provider);
}

void com_parusinskimichal_OSXDeviceMapper::ejectVNode() {
   if (m_vnodedisk == NULL) // To avoid any strange surprises
       return;

   m_vnodedisk->retain(); // prevent any strange surprises

   if (!m_vnodedisk->terminate(kIOServiceRequired)) // first unregister the service
       IOLog("Error at terminating device\n");

    m_vnodedisk->detach(this); // this should release the vnode
    m_vnodedisk->closeVNode();
    m_vnodedisk->release();
}

