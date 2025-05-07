---
date: '2025-05-07T17:39:07+08:00'
title: 'Rename Vc9'
---


```python
# vc9 files
from surun_tools.glob1 import glob_files

glob_files("*VC9.sln")
```




    [WindowsPath('C:/src/surun/src/PC/SuRunVC9.sln')]




```python
glob_files("*VC9.vcxproj*")
```




    [WindowsPath('C:/src/surun/src/install_surun/InstallSuRunVC9.vcxproj.user'),
     WindowsPath('C:/src/surun/src/PC/InstallSuRunVC9.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/InstallSuRunVC9.vcxproj.filters'),
     WindowsPath('C:/src/surun/src/PC/InstallSuRunVC9.vcxproj.user'),
     WindowsPath('C:/src/surun/src/PC/SuRunExtVC9.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRunExtVC9.vcxproj.filters'),
     WindowsPath('C:/src/surun/src/PC/SuRunExtVC9.vcxproj.user'),
     WindowsPath('C:/src/surun/src/PC/SuRunVC9.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRunVC9.vcxproj.filters'),
     WindowsPath('C:/src/surun/src/PC/SuRunVC9.vcxproj.user'),
     WindowsPath('C:/src/surun/src/PC/DebugUx64/SuRunExtVC9.vcxproj.FileListAbsolute.txt'),
     WindowsPath('C:/src/surun/src/PC/DebugUx64/SuRunVC9.vcxproj.FileListAbsolute.txt')]




```python
from pathlib import Path
for i in glob_files("*VC9.vcxproj*"):
    new_name = str(i).replace("VC9", "")
    import shutil
    shutil.move(i, Path(new_name))
```


```python
from pathlib import Path
for i in glob_files("*VC9.*"):
    new_name = str(i).replace("VC9", "")
    import shutil
    shutil.move(i, Path(new_name))
```


```python
sln = glob_files("*.sln")[0]
sln
```




    WindowsPath('C:/src/surun/src/PC/SuRun.sln')




```python
c: str = sln.read_text(encoding="utf8")
c = c.replace("VC9", "")
sln.write_text(c, encoding="utf8")
```




    15758




```python

```
