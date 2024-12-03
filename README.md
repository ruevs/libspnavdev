spnavdev library
================
This is a direct 6DOF (Sixe Degeees Of Freedom) device (a.k.a. SpaceMouse) handling library written
in C, which is usable as a standalone library to interface with 6DOF devices without relying on
anything but a USB HID driver ([HIDAPI](https://github.com/libusb/hidapi) on Windows and Linux) and
some code to read/write from an UART (RS-232 port) for the serial SpaceMice.

It is easily portable to embedded systems, robot controllers, non-UNIX systems etc.

The goal of spnavdev is to handle all USB and serial 6DOF devices, across multiple platforms such
as: all UNIX systems, windows, DOS, and possibly even bare metal on certain systems.

### Supported devices

The library supports all USB devices from 3Dconnexion and should support all serial devices as
well. The following devices are tested and fully working - 6DOF input, buttons, LEDs (if present):

#### USB devices (all by 3Dconnexion):

Device | VID | PID | Note
------ | --- | --- | ----
CadMan USB | 0x046d | 0xc605 | 
SpaceBall 5000 USB | 0x046d | 0xc621 | 
SpaceTraveler USB | 0x046d | 0xc623 | 
SpacePilot SP1 USB | 0x046d | 0xc625 | The LCD screen is supported but the API is not finished
SpaceNavigator | 0x046d | 0xc626 | 
SpaceExplorer | 0x046d | 0xc627 | 

"New" devices (with VID = 0x256f) should work but I have not personally tested them.

#### Serial devices:

Device | Manufacturer | Note
------ | ------------ | ----
[Magellan SpaceMouse Classic](https://spacemice.org/index.php?title=Spacemouse_Classic) | LogiCad3D GmbH (1993) | Fw: "v  MAGELLAN  Version 5.79  by LOGITECH INC. 10/10/97"
[Spaceball 3003 FLX](https://spacemice.org/index.php?title=Spaceball_3003) | Spacetec IMC Ltd. (1996) | Fw: "Firmware version 2.62 created on 24-Oct-1997."
[Magellan SpaceMouse Plus](https://spacemice.org/index.php?title=Spacemouse_Plus) | LogiCad3D GmbH (1998), 3DConnexion (2001) | Fw: "v  MAGELLAN  Version 6.70  3Dconnexion GmbH 05/11/02"

If you own a SpaceMouse not listed above please test it and report your results in the issues. Just
run the test[.exe] program that is built in the examples directory when you [compile](#Building)
the library.

## Building 

Irrespective of the OS used, before building, check out the project and the necessary submodules:

```sh
git clone https://github.com/ruevs/libspnavdev.git
cd libspnavdev
git submodule update --init
```

### Building on Linux

You will need the usual build tools and CMake. On a Debian derivative (e.g. Ubuntu) these can be
installed with:

```sh
sudo apt install git build-essential cmake
```

On a RedHat derivative (e.g. Fedora) the dependencies can be installed with:

```sh
sudo dnf install git gcc-c++ cmake
```

Before building, [check out the project and the necessary submodules](#Building).

After that, build libspnavdev as following:

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Building on Windows

You will need [git][gitwin], [cmake][cmakewin] and a C compiler
(either Visual C++ or MinGW).

Before building, [check out the project and the necessary submodules](#Building).

#### Building with Visual Studio IDE

Create a directory `build` in the source tree and point cmake-gui to the source tree and that
directory. Press "Configure" and "Generate", then open `build\libspnavdev.sln` with
Visual C++ and build it.

Alternatively you can open the directory with Visual Studio directly. The modern versions have
built-in support for CMake projects and should handle it.

### Building with Visual Studio in a command prompt

First, ensure that `git` and `cl` (the Visual C++ compiler driver) are in your `%PATH%`; the latter
is usually done by invoking `vcvarsall.bat` from your Visual Studio install. Then, run the
following in cmd or PowerShell:

```bat
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
```

To build a 32 bit version that will run on Windows XP first add the [`v141_xp` Platform Toolset](https://learn.microsoft.com/en-us/cpp/build/configuring-programs-for-windows-xp)
to your Visual Studio installation and then run:

```bat
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -T v141_xp -A Win32
```

Open `build\libspnavdev.sln` with Visual C++ and build it.

#### Building with MinGW

It is also possible to build libspnavdev using [MinGW][mingw].

First, ensure that git and gcc are in your `$PATH`. Then, run the following in bash:

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

[gitwin]: https://git-scm.com/download/win
[cmakewin]: http://www.cmake.org/download/#latest
[mingw]: http://www.mingw.org/

History
-------
John Tsiombikas (the author of [spacenavd](https://github.com/FreeSpacenav/spacenavd)) [started writing this library](https://github.com/FreeSpacenav/libspnavdev)
in November 2020 with the inntention to:

> eventually replace the
> device handling code in spacenavd v2, and will also be usable as a standalone
> library to interface with 6dof devices without relying on spacenavd in cases
> where that is preferable (embedded applications, robot controllers, non-UNIX
> systems etc).

However apart from the first [two commits](https://github.com/FreeSpacenav/libspnavdev/commits/master/) -
where he set up the API but no functionality behind it - he never worked on it and eventually
[declared it abandoned](https://github.com/FreeSpacenav/libspnavdev/commit/427f98957b3691197a48a03315395c3b8df6101a).

In February 2021 I got a SpeceExplorer for debugging the [SolveSpace](https://github.com/FreeSpacenav/libspnavdev/commit/427f98957b3691197a48a03315395c3b8df6101a)
support for SpaceMice. I got interested in the low-level HID API of the 3Dconnexion devices and
decided to write my own driver/library. I quickly discovered that the protocol is easy and many had
done the same in the past. But there was no simple low-level library written in C that supported
all devices. That is when I found `libspnavdev`, which was only a "scaffold", and
[decided to implement support for all 3Dconnexion devices](https://github.com/FreeSpacenav/libspnavdev/pull/1#issuecomment-779057322).
What you see in this repository is the result of this work. As to why it was not merged upstream
[see here](https://github.com/FreeSpacenav/libspnavdev/pulls?q=is%3Apr+is%3Aclosed).

License
-------
Copyright (C) 2020 John Tsiombikas <nuclear@member.fsf.org>

Copyright (C) 2021-2024 [ruevs](https://github.com/ruevs) and [rpavlik](https://github.com/rpavlik)

This library is free software, feel free to use, modify and/or redistribute it under the terms of
the GNU General Public License v3, or at your option any later version published by the Free
Software Foundation. See [COPYING](COPYING) for details.
