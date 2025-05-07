---
date: 2025-05-01T00:58:19+08:00
title: Manifest_Files
---

# About manifest files

[reference docs][1]

[1]: https://learn.microsoft.com/en-us/windows/win32/sbscs/application-manifests


```python
from surun_tools.glob1 import glob_files
for i in glob_files("*.manifest"):
    print(i)
```

    C:\src\surun\install_surun\app.manifest
    C:\src\surun\resources\app.manifest
    C:\src\surun\surun_com\resources\app.manifest
    C:\src\surun\tests\TestScreenshot\app.manifest
    

An application manifest (also known as a side-by-side application manifest, or a fusion manifest) is an XML file that describes and identifies the shared and private side-by-side assemblies that an application should bind to at run time. These should be the same assembly versions that were used to test the application. Application manifests might also describe metadata for files that are private to the application.

also see: [dotnet docs][2]

Every assembly, whether static or dynamic, contains a collection of data that describes how the elements in the assembly relate to each other. The assembly manifest contains this assembly metadata. An assembly manifest contains all the metadata needed to specify the assembly's version requirements and security identity, and all metadata needed to define the scope of the assembly and resolve references to resources and classes. The assembly manifest can be stored in either a PE file (an .exe or .dll) with common intermediate language (CIL) code or in a standalone PE file that contains only assembly manifest information.

[2]: https://learn.microsoft.com/en-us/dotnet/standard/assembly/manifest

## To learn

Seems that this is a technology to address the DLL hell problem.

https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-redirection


