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

OSDefineMetaClassAndStructors(com_parusinskimichal_VNodeDiskController, IOService)

#define super IOService

bool com_parusinskimichal_VNodeDiskController::init(OSDictionary *dict)
{
  if (super::init(dict)) {
    IOLog("Initializing VNodeDisk controller\n");
    return true;
  } else {
    IOLog("Unsucessfuly initialised parent\n");
    return false;
  }
}

void com_parusinskimichal_VNodeDiskController::free(void)
{
  IOLog("Freeing the VNodeDisk controller\n");
  super::free();
}

IOService *com_parusinskimichal_VNodeDiskController::probe(IOService *provider,
  SInt32 *score)
{
  IOService *result = super::probe(provider, score);
  IOLog("Probing the VNodeDisk controller\n");
  return result;
}

bool com_parusinskimichal_VNodeDiskController::start(IOService *provider)
{
  IOLog("Starting the VNodeDisk controller\n");
  if (!super::start(provider))
    return false;
  return true;
}

void com_parusinskimichal_VNodeDiskController::stop(IOService *provider)
{
  IOLog("Stopping the VNodeDisk controller\n");
  super::stop(provider);
}

