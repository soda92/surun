# SuRun

<a href="https://pypi.org/project/soda-surun/">
    <img alt="PyPI - Version" src="https://img.shields.io/pypi/v/soda-surun">
</a>

A program for grant app admin rights without UAC prompt.

Forked from [Kay Bruns' surun][1].

## Install
```
pip install -U soda-surun
surun-install
```

To install from source, see [SuRun installation][install].

## Development

see [SuRun Development enviroment setup][dev].

## modifications

I have changed code to make surun:
- Windows 10 only (winver to 0x0A00)
- compatible with 4K screen (see [this commit][4k-commit])

[or]: https://surun-docs.web.app/post/original_readme/
[4k-commit]: https://github.com/soda92/surun/commit/bad6e31f13f115a65a314c6615c8d585eb1bb325
[1]: https://kay-bruns.de/wp/software/surun/

[install]: https://surun-docs.web.app/post/surun-installation/
[dev]: https://surun-docs.web.app/post/environment/
