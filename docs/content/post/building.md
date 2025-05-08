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

You can just run `surun_build` in the python venv, it calls the [build script][script].

[script]: https://github.com/soda92/surun/blob/main/scripts/build.ps1
[dec]: https://surun-docs.web.app/post/decisions/
