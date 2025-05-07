---
date: '2025-05-07T18:39:45+08:00'
title: 'Remove Unused Confs'
---


```python
from surun_tools.glob1 import glob_files
glob_files("*.vcxproj")
```




    [WindowsPath('C:/src/surun/src/PC/InstallSuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRunExt.vcxproj'),
     WindowsPath('C:/src/surun/tests/TestScreenshot/TestScreenshot.vcxproj'),
     WindowsPath('C:/src/surun/tests/TestScreenshotSuRun/TestScreenshotSuRun.vcxproj')]




```python
proj_files = list(filter(lambda x: "test" not in str(x), glob_files("*.vcxproj")))
proj_files
```




    [WindowsPath('C:/src/surun/src/PC/InstallSuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRunExt.vcxproj')]




```python
ext_proj = glob_files("*.vcxproj")[2]
```


```python
print(ext_proj)
```

    C:\src\surun\src\PC\SuRunExt.vcxproj
    


```python
from lxml import etree
parser = etree.XMLParser(remove_comments=False)
tree = etree.parse(ext_proj, parser)
```


```python
def is_correct_conf(s: str) -> bool:
    if "x64 Unicode Debug|x64" in s:
        return True
    elif "SuRun32 Unicode Debug|Win32" in s:
        return True
    return False
```


```python
def save():
    global tree
    tree.write(ext_proj, encoding="utf-8", xml_declaration=True)
    from pathlib import Path

    content = Path(ext_proj).read_text(encoding="utf8")
    content = content.replace("ns0:", "").replace(":ns0", "").replace('/>', ' />')
    Path(ext_proj).write_text(content, encoding="utf8")
```


```python
ns = {"default": "http://schemas.microsoft.com/developer/msbuild/2003"}


def remove(tag, tag2="Condition"):
    tag = "/".join(map(lambda x: "default:" + x, tag.split("/")))
    root = tree.getroot()

    for i in root.findall(f"{tag}", ns):
        if tag2 not in i.attrib:
            continue
        if not is_correct_conf(i.attrib[tag2]):
            i.getparent().remove(i)

    save()
```


```python
remove("ItemGroup/ProjectConfiguration")

```


```python
remove("PropertyGroup")
```


```python
remove("ImportGroup")
```


```python
remove("ItemDefinitionGroup")
```


```python
remove("ItemGroup/CustomBuild/ExcludedFromBuild")
```


```python
from pathlib import Path
for i in proj_files:
    tree2 = etree.parse(i, parser)
    ns = {"default": "http://schemas.microsoft.com/developer/msbuild/2003"}

    def remove(tag, tag2="Condition"):
        tag = "/".join(map(lambda x: "default:" + x, tag.split("/")))
        root = tree2.getroot()

        for j in root.findall(f"{tag}", ns):
            if tag2 not in j.attrib:
                continue
            if not is_correct_conf(j.attrib[tag2]):
                j.getparent().remove(j)

        tree2.write(i, encoding="utf-8", xml_declaration=True)

        content = Path(i).read_text(encoding="utf8")
        content = content.replace("ns0:", "").replace(":ns0", "").replace('/>', ' />')
        Path(i).write_text(content, encoding="utf8")
    remove("ItemGroup/ProjectConfiguration")
    remove("PropertyGroup")
    remove("ImportGroup")
    remove("ItemDefinitionGroup")
    remove("ItemGroup/CustomBuild/ExcludedFromBuild")
```

## for sln files

remove these:
```
GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32 <-
		Debug|x64 = Debug|x64     <-
...
EndGlobalSection
```

for this file we can see that only conf contains | symbol. we can use that.


```python
sln:Path = glob_files("*.sln")[0]
print(sln)
```

    C:\src\surun\src\PC\SuRun.sln
    


```python
content = sln.read_text(encoding="utf8")
text = []
for i in content.split("\n"):
    if "|" in i:
        if not is_correct_conf(i):
            continue
        else:
            text.append(i)
    else:
        text.append(i)
sln.write_text("\n".join(text), encoding="utf8")
```




    3861




```python

```
