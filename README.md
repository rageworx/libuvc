# `libuvc` modified version for MinGW-W64 ( no option for CMake )

This forked project is not an original, if you finding original source, visit to https://github.com/ktossell/libuvc

# About libuvc 

`libuvc` is a cross-platform library for USB video devices, built atop `libusb`.
It enables fine-grained control over USB video devices exporting the standard USB Video Class
(UVC) interface, enabling developers to write drivers for previously unsupported devices,
or just access UVC devices in a generic fashion.

## Getting and Building libuvc for MinGW-W64

Prerequisites: Make sure `libusb` and `libjpeg` configured and install on MSYS.

To build, you can just run these shell commands:

    make -f Makefile.mingw

To install your system, copy these:

    cp lib/libuvc.a /usr/local/lib
	cp -rf include/* /usr/local/include
	
 * MinGW-W64 doesn't support sudo copying.
	
## Developing with libuvc

The documentation for `libuvc` can currently be found at https://int80k.com/libuvc/doc/.

Happy hacking!
