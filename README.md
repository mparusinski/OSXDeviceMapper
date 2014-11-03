OSXDeviceMapper (name may be subject to change) is a project that aims at
creating a [Device Mapper](https://en.wikipedia.org/wiki/Device_mapper) 
like tool for Mac OS X.

More precisely the goal of the project is to create a Kernel Extension
that offers extensive block device mapping functionality. In addition
to creating a Kernel Extension this project aims at providing a clear
API to interface the Kernel Extension as well a series of command
line tools to control the device mapping kernel extension. Unlike the 
Linux equivalent Device Mapper this tool will also provide a vnode disk 
device functionality (commonly referred on Linux as loop device).

This project is BSD-licensed and such can be used within open-source 
projects as well as commercial projects free of charge. This tool is 
useful if your project needs easy block device creation, if you develop
a filesystem and needs many devices for testing, or if you plan to 
create a port of tc-play (a BSD-licensed implementation of Truecrypt)
on OSX. The latter is the reason why I started this project.
