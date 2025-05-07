---
date: '2025-05-07T17:48:16+08:00'
title: 'Remove Bscmake'
---


```python
from surun_tools.glob1 import glob_files
```


```python
glob_files("*.vcxproj")
```




    [WindowsPath('C:/src/surun/src/PC/InstallSuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRun.vcxproj'),
     WindowsPath('C:/src/surun/src/PC/SuRunExt.vcxproj'),
     WindowsPath('C:/src/surun/tests/TestScreenshot/TestScreenshot.vcxproj'),
     WindowsPath('C:/src/surun/tests/TestScreenshotSuRun/TestScreenshotSuRun.vcxproj')]




```python
lines = []
for i in glob_files("*.vcxproj"):
    content = i.read_text(encoding="utf8")
    for line in content.split('\n'):
        if "bscmake" in line.lower():
            lines.append(line)
lines[:3]
```




    []




```python
from lxml import etree

ns = {"default": "http://schemas.microsoft.com/developer/msbuild/2003"}


def remove_bscmake_nodes(xml_file):
    """
    Removes all nodes with the name 'bscmake' and their contents from an XML file.

    Args:
        xml_file (str): The path to the XML file.
    """
    try:
        parser = etree.XMLParser(remove_comments=False)
        tree = etree.parse(xml_file, parser)
        root = tree.getroot()

        for i in root.findall('default:ItemDefinitionGroup', ns):
            for j in i.findall("default:Bscmake", ns):
                i.remove(j)

        tree.write(xml_file, encoding="utf-8", xml_declaration=True)
        from pathlib import Path

        content = Path(xml_file).read_text(encoding="utf8")
        content = content.replace("ns0:", "").replace(":ns0", "").replace('/>', ' />')
        Path(xml_file).write_text(content, encoding="utf8")

    except FileNotFoundError:
        print(f"Error: File not found at '{xml_file}'.")
    except etree.XMLSyntaxError:
        print(f"Error: Could not parse the XML file '{xml_file}'.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


for file_path in glob_files("*.vcxproj"):
    remove_bscmake_nodes(file_path)
```


```python

```
