---
date: '2025-05-10T10:50:32+08:00'
title: 'using Makefile On Windows with powershell'
---

Actually, makefile can be used on Windows. I have used them when intergrating SuRun with Go.

remove directory
```makefile
pwsh -nop -c "rm -r -Force ../build-ig || 1"
```
create directory

```makefile
pwsh -nop -c "mkdir ReleaseUx64" || true
```

launch makefile in MSYS2
```makefile
msys-surun:
	pwsh -nop msys.ps1 -ucrt64 -c "make _msys_surun"

_msys_surun:
	# commands in MSYS2
```


