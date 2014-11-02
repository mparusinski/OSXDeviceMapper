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
    m_vnodeController = NULL;
    return true;
  } else {
    IOLog("Unsucessfuly initialised parent\n");
    return false;
  }
}

void OSXDeviceMapperClass::free(void)
{
  IOLog("Freeing the driver\n");
  if (!m_vnodeController)
    m_vnodeController->release();
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

  m_vnodeController = new VNodeDiskControllerClass;
  if (!m_vnodeController)
    return false;

  if (!m_vnodeController->init(0))
    return false;

  m_vnodeController->attach(this);

  m_vnodeController->createVNodeWithFilePathAndBlockSizeAndBlockNum(
    "/tmp/vnodedevice", 4096, 256);

  return true;
}

void OSXDeviceMapperClass::stop(IOService *provider)
{
  IOLog("Stopping the driver\n");
  if (m_vnodeController)
    m_vnodeController->deleteAllVNodes();

  m_vnodeController->detach(this);
  m_vnodeController->release();

  super::stop(provider);
}
