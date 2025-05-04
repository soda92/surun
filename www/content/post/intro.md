---
date: '2025-05-04T19:01:44+08:00'
title: 'Intro to SuRun'
---

[SuRun Repository][repo]

[repo]: https://github.com/soda92/surun.git

SuRun eases using Windows 11 with limited user rights.

With SuRun you can start applications with elevated rights without needing
administrator credentials.

The idea is simple and was taken from [SuDown][sudown].
The user usually works with the pc as standard user with limited rights. 
If a program needs administrative rights, the user starts "SuRun `app`". 
SuRun then asks the user in a secure desktop if `app` should really be 
run with administrative rights. If the user acknowledges, SuRun will start 
`app` AS THE CURRENT USER but WITH ADMINISTRATIVE RIGHTS.

SuRun uses an own Windows service to start administrative processes. 
It does not require nor store any user passwords (in non domain scenarios).

To use SuRun a user must be member of the local user group "SuRunners".
If the user is no member of "SuRunners" and tries to use SuRun, (s)he will be 
asked to join "SuRunners". The user must either be an administrator or enter 
administrator credentials to join the SuRunners group.
(SuRun does not store any administrator credentials!)

[sudown]: http://SuDown.sourceforge.net

[How to install SuRun][install]

[install]: https://surun-docs.web.app/post/surun-installation/
