#!/usr/bin/python

import sys
from subprocess import call, check_call, CalledProcessError

projectName = 'OSXDeviceMapper'

try:
    check_call(['xcodebuild'])
except CalledProcessError:
    print "Build not successful"
    sys.exit(-1)


try:
    check_call(['kextlibs', '-xml', 'build/Debug/'+projectName+'.kext'])
    check_call(['kextlibs', '-xml', 'build/Release/'+projectName+'.kext'])
except CalledProcessError:
    print "Some libraries has not being linked with kext"
    sys.exit(-1)
