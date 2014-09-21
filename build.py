#!/usr/bin/python

import sys
from subprocess import call, check_call, CalledProcessError

projectName = 'OSXDeviceMapper'

mode = 'Release'
if len(sys.argv) > 1:
    mode = sys.argv[1]
destinationFolder = None
if len(sys.argv) > 2:
    destinationFolder = sys.argv[2]

try:
    print 'BUILDING KEXT'
    check_call(['xcodebuild', '-configuration', mode])
except CalledProcessError:
    print "Build not successful"
    sys.exit(-1)

try:
    print 'CHECKING LIBRARIES ARE LINKED'
    check_call(['kextlibs', '-xml', 'build/'+mode+'/'+projectName+'.kext'])
except CalledProcessError:
    print "Some libraries has not being linked with kext"
    sys.exit(-1)


try:
    print 'RUNNING DRY SIMULATION'
    check_call(['sudo', 'cp', '-Rv', 'build/'+mode+'/'+projectName+'.kext',
        '/tmp'])
    check_call(['kextutil', '-tn', '/tmp/'+projectName+'.kext'])
except CalledProcessError:
    print "Unsuccessful at checking if kext is OK"

if destinationFolder:
    try:
        print 'COPYING KEXT TO DESTINATION'
        check_call(['cp', '-Rv', 'build/'+mode+'/'+projectName+'.kext',
            destinationFolder])
    except CalledProcessError:
        print 'Unsuccessful at moving kext to ' + destinationFolder
