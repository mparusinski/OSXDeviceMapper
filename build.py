#!/usr/bin/python

import sys
from subprocess import call, check_call, CalledProcessError

projectName = 'OSXDeviceMapper'

try:
    print 'BUILDING KEXT'
    check_call(['xcodebuild'])
except CalledProcessError:
    print "Build not successful"
    sys.exit(-1)


try:
    print 'CHECKING LIBRARIES ARE LINKED'
    check_call(['kextlibs', '-xml', 'build/Debug/'+projectName+'.kext'])
    check_call(['kextlibs', '-xml', 'build/Release/'+projectName+'.kext'])
except CalledProcessError:
    print "Some libraries has not being linked with kext"
    sys.exit(-1)


try:
    print 'RUNNING DRY SIMULATION'
    check_call(['sudo', 'cp', '-Rv', 'build/Debug/OSXDeviceMapper.kext',
        '/tmp'])
    check_call(['kextutil', '-tn', '/tmp/OSXDeviceMapper.kext'])
except CalledProcessError:
    print "Unsuccessful at checking if kext is OK"
