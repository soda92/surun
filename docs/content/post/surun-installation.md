---
date: '2025-05-04T18:35:58+08:00'
title: 'Surun Installation'
---

<a href="https://pypi.org/project/soda-surun/">
    <img alt="PyPI - Version" src="https://img.shields.io/pypi/v/soda-surun">
</a>

## install

```bash
pip install -U soda-surun
surun-install
```
The installation requires admin priviledges.

You need to logout and login after installation.
You may need to re-login on browsers for all websites because of credential change.

## install from source

The source code was also released on PYPI. You can also choose to install from source.

You need to install VS2022 community/buildtools with C++ workload and ATL&MFC. see [dev][dev]
for more details.

To install from source:

- download source tar.gz from PYPI, or clone [the repository][repo].
- install:
    - for install from file, run `pip install soda-surun-a.b.c.d.tar.gz`.
    - for install from github source, change to the repo dir, then run `pip install .`.

[repo]: https://github.com/soda92/surun
[dev]: https://surun-docs.web.app/post/environment/
