OSXDeviceMapper (name will change at some point) is a project that aims 
at a creating a [Device Mapper](https://en.wikipedia.org/wiki/Device_mapper) 
like tool for Mac OS X.

More precisely the goal of the project is to create a Kernel Extension
that offers extensive block device mapping functionality. In addition
to creating a Kernel Extension this project aims at providing a clear
API to interface the Kernel Extension as well a series of command
line tools to control the device mapping Kernel Extension. Unlike the 
Linux equivalent Device Mapper this tool will also provide a vnode disk 
device functionality (commonly referred on Linux as loop device).

This project is BSD-licensed and such can be used within open-source 
projects as well as commercial projects free of charge. This tool is 
useful if your project needs easy block device creation, if you develop
a filesystem and needs many devices for testing, or if you plan to 
create a port of tc-play (a BSD-licensed implementation of Truecrypt)
on OSX. The latter is the reason why I started this project.

# Compiling/Installing from source

## Warning

OS X Yosemite requires all kernel extension to be signed using a 
Developer ID account enabled for kernel development. This means
that forking this project and hacking on it will require you to 
purchase an Apple Developer ID and request the account to be
enabled for Kernel Extension development.

## Requirements

You should only compile and install from source in the following cases:
* You want to integrate OSXDeviceMapper (name will change at some point)
  in your project commercial or open-source.
* You wish to contribute to the OSXDeviceMapper project. In which case
  I advise you to get in touch with me.
* You need for some reason to modify the way OSXDeviceMapper works.
* You want to test OSXDeviceMapper for bugs and/or security issues. 
  If so please report all those issues, but take into account the 
  support section.

An apple computer running either OS X Mavericks or OS X Yosemite.
The project is currently been developed on Yosemite but should
work also on OS X Mavericks.

You also need to have an Apple Developer ID certified for Kernel 
Extension development. This requires you to purchase a Developer
ID account as well as request Apple to sign your Developer ID
for Kernel Extension development.

On your Mac will you need to install XCode and download command line
tools for OS X.

## Compilation 

Compilation from source can be done either through XCode or using
the build.py tool. The latter uses XCode behind the scenes.

### XCode

Using XCode is the simplest but requires to run all of XCode 
for development (which is quite ressources hungry).

Load the project OSXDeviceMapper.xcodeproj in XCode and compile
from XCode.

### build.py

This tool was created to enable compilation without having XCode running, 
which is quite useful when running an OS X virtual machine, a browser, 
a fancy code editor, a terminal and iTunes and system memory becomes 
scarces. The tool also verify the Kext is properly linked and verifies it 
is loadable. In other words it goes further than compiling using XCode.

Syntax:

```shell
./build.py (Debug|Release) /destination/path/for/kext
```

Both arguments are optional. 
* The first option tells whether to create a Debug build
  or a release build. It defaults to Release
* The second argument if specified tells the build script
  to copy the build artifacts to the specified folder.

If you run into any problems you have to create the following
folder 'build/Debug' and 'build/Release'.

## Testing

To test the kernel extension you need a second machine either physical 
or a virtual machine. This kernel extension so far has been tested on 
a Yosemite virtual machine running on VMWare Fusion. Technically this
can be done on any OS X machine but bear in mind any bugs may restart 
the machine.

1 - You first need to copy the Kext to the test machine.
2 - Copy the kernel extension over to the /tmp folder as this will
    guarantee the appropriate permissions on the kernel extension.
    ```shell
    sudo cp -Rv /path/to/kext /tmp/
    ```
3 - Load the kext using the following command
    ```shell
    sudo kextload /tmp/OSXDeviceMapper.kext
    ```
4 - Unload the kext using the following command
    ```shell
    sudo kextunload /tmp/OSXDeviceMapper.kext
    ```

At any time of the test machine you can run 
```shell
sudo dmesg
```
to check error messages produced by the kernel extension. Bear in mind
error, warning and info messages from other system components will show
up as well.

## Installing

At the moment there is no supported way of installing this kernel extension
and I highly recommend not trying to install this kernel extension at this 
moment. The kernel extension has not yet been subject to rigorous testing.

# Support

So far we following subcomponents are planned, however this list may be
subject to change:

Feature | Support status | Owner
--------| ---------------|------
API | Not started | No one
Cache hybrid devices | Not started | No one
Command line tools | Not started | No one
Delay devices | Not started | No one
Crypt devices | Not started | No one
Era devices | Not started | No one
Error simulating devices | Not started | No one
Linear mapped devices| Not started | No one
Mirrored devices | Not started | No one
Multipath devices | Not started | No one
RAID devices | Not planned | N/A
Snapshot devices | Not planned | N/A
Striped devices | Not planned | N/A
Unreliable simulating devices | Not started | No one
Virtual VNode devices | Ongoing development | Michal Parusinski
Zero devices | Not started | No one
