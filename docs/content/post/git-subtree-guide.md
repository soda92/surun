---
date: '2025-05-04T18:49:18+08:00'
title: 'Git Subtree Guide'
---

see <https://www.atlassian.com/git/tutorials/git-subtree>

## init
`git subtree add --prefix docs/themes/PaperMod https://github.com/soda92/PaperMod2 my --squash`

## Pull
```
git subtree pull --prefix docs/themes/PaperMod https://github.com/soda92/PaperMod2 my --squash
```