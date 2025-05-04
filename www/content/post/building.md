---
date: '2025-05-04T18:51:07+08:00'
title: 'Building SuRun'
---

As with any modern C++ applications, the support for GCC and MINGW is deprecated.

you must install Visual Studio first.

This is the same with npm and flutter packages.

## Building

SuRun contains both 64bit and 32bit parts. to build you can use config "x64 Unicode Debug|x64" and "SuRun32 Unicode Debug|win32".

As with [decisions][dec], I have dropped Release builds.

You can use [a conveience script][script] for building.

[script]: https://github.com/soda92/surun/blob/main/scripts/build.ps1
[dec]: https://github.com/soda92/surun/blob/main/docs/decisions.md
