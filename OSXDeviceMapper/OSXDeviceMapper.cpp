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

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#include <IOKit/IOLib.h>
#include "OSXDeviceMapper.h"

OSDefineMetaClassAndStructors(com_parusinskimichal_OSXDeviceMapper, IOService);

#define super IOService

IOService *com_parusinskimichal_OSXDeviceMapper::probe(IOService *provider, SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    IOLog("Probing\n");
    return result;
}

bool com_parusinskimichal_OSXDeviceMapper::init(OSDictionary *dict)
{
    if (!super::init(dict)) {
        return false;
    }
    
    m_loop_file = nullptr;
    
    return true;
}

void com_parusinskimichal_OSXDeviceMapper::free(void)
{
    super::free();
}

bool com_parusinskimichal_OSXDeviceMapper::start(IOService *provider)
{
    if (!super::start(provider)) {
        return false;
    }
    
    // Opening file
    if (!vnode_open(LOOPDEVICE_FILE_PATH,
                    FWRITE,
                    700,
                    VNODE_LOOKUP_NOFOLLOW | VNODE_LOOKUP_NOCROSSMOUNT,
                    m_loop_file,
                    m_vfs_context)){
        goto err;
    }
    
    return true;
    
err:
    // TODO Handle error
    
    return false;
}

void com_parusinskimichal_OSXDeviceMapper::stop(IOService *provider)
{
    // Close file
    if (!vnode_close(*m_loop_file, FWASWRITTEN, m_vfs_context)) { // TODO: FWASWRITTEN only if file was written = dirty
        goto err;
    }
    
    super::stop(provider);
    
err:
    return;
}
