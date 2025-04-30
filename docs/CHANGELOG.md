ToDo:
---------------------
* Handle \??\C:\Windows\System32\conhost.exe* make Tray Windows for automagically started apps configurable
* Hooks must directly write to Service pipe
* on Win x64 detect x32 Executables on x64 hook (ShellExecute32->WoW->CreateProcess64)
* On ShellExec/IAT-Hook: Create Process suspended! Hooks must resume process.
* Create Inline Hook instead of IAT-Hook
* GUI for SwitchToUser
* Create a "Default" SuRunner for users that are not in "SuRunners"
* MAP network drives
* Checksums for "Always Yes" programs
* use IContextMenu2/IContextMenu to implement an Icon/Popup menu
* Console SuRun support
* LOG-File for SuRun activity

To be done in future:
---------------------
* Use Radio-Buttons for (normal/elevated) Auto-Magic
* Hide/Show all context menu entries consistently
* make all context menu entries dynamically with ShellExt 
  (E.g.: msi with popup-menu)

Deferred Whishlist:
---------------------
* icons for SuRuns context menu entries
* Intercept CreateProcessAsUser in services and check for programs started 
  with limited rights that need to be started as admin

------------------------------------------------------------------------------
SuRun Changes:
------------------------------------------------------------------------------

SuRun 1.2.1.6b1 - 2022-10-03
----------------------------
* FIX: SuRun needs to borrow the Process Token from LSASS.exe to get the 
       SeCreateTokenPrivilege, this did not work with PPL enabled 
       (HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa,RunAsPPL")
       This has been fixed

SuRun 1.2.1.5 - 2021-09-16
----------------------------
* FIX: SuRun got a new option to delay the service start. This is a workaround
       to Stop Windows from deleting saved credentials.

SuRun 1.2.1.4 - 2020-09-24 (silent update)
----------------------------
* FIX: Control Panel As Administrator did not work anymore in Windows 10 2004

SuRun 1.2.1.4 - 2019-09-24
----------------------------
* Released 1.2.1.4

SuRun 1.2.1.4b1 - 2019-05-11
----------------------------
* FIX: SuRun's "H.I.P.S. Warning" Window did not show on user's desktop
* FIX: GetEffectiveRightsFromAcl BUGs in Windows 10 1809, replaced the function

SuRun 1.2.1.3 - 2019-01-28
----------------------------
* Released 1.2.1.3

SuRun 1.2.1.3rc1 - 2017-10-23
----------------------------
* CHG: Made Hooks Windows 10 1709 compatible.

SuRun 1.2.1.3b3 - 2017-08-20
----------------------------
* NEW: SuRun detects renamed computer and imports user's settings
* FIX: User Bitmaps for Domain Users were not shown

SuRun 1.2.1.3b2 - 2017-08-09
----------------------------
* NEW: Moderated Restricted Surunners. ("User can only run predefined programs 
			 with elevated rights")
	     If a restricted SuRunner tries to run a non validated Programm with 
	     elevated rights, SuRun will now ask for Administrator credentials to 
	     launch the program elevated in the user's context.
* FIX: Fixed Spanish Resources

SuRun 1.2.1.3b1 - 2017-03-11
----------------------------
* NEW: SuRun Executables are digitally signed
* CHG: Changed SuRun's Context Menu Strings from "<...>" to "SuRun: <...>"
* FIX: Members of "Power Users" and "Backup Opeators" could not use SuRun
* FIX: Bug in IAT-Hooks made Internet Explorer Crash on Windows 10

SuRun 1.2.1.2 - 2015-08-09
----------------------------
* CHG: Made Hooks Windows X compatible.

SuRun 1.2.1.1 - 2015-02-15
----------------------------
* NEW: Option to hide SuRun's "green smiley" tray icon
* NEW: Support for Windows 8 and Windows 8.1
* CHG: Option "remember password" in RunAs-Dialog is grayed out when SuRun 
       always asks for the user's password
* CHG: In Windows Vista and later SuRun does not modify registry settings for 
       UAC "Run as administrator". 
       This enables <ctrl>+<shift>+<lClick> for pinned programs in Explorer.
* CHG: The hooks ignore all command lines with "://" before the first " ".
       So any http://www.yyy.zzz commands are initially ignored.
* CHG: IAT-Hook intercepts ShellExecute and friends because Windows does 
       not support SEE_MASK_NOCLOSEPROCESS in IShellExecuteHook.
* CHG: A Bug in Online Armor makes any program crash, that is using the Exit 
       Code (NOT GetCurrentProcessId()), as SuRun did, before this change.
* FIX: User Bitmap Display on confirmation and runas dialogs did not show
* FIX: SuRun's menu item "SuRun: Empty Recycle Bin" was not uninstalled
* FIX: Long command lines were not displayed ok on confirmation dialogs
* FIX: /SWITCHTO did not work in Windows XP x32.
* FIX: Saving Passwords failed in Windows 2000
* FIX: When using SuRun /RunAs, if the user name was cleared, SuRun deleted 
       the SuRun settings for all users. This caused any "SuRunners" member 
       to get full access to the system.
       The Bug appeared with SuRun's /RunAs support in version 1.2.0.0.
* FIX: Changed permissions of started Programs for Jumplist compatibility.
* FIX: SuRun's hooks were not x86 and x64 "cross compatible".
* FIX: SuRun's 32Bit IAT-Hook caused a General Protection Fault in the 
       32Bit Office Help viewer on x64 Windows 7.
* FIX: On all Windows x64 Systems SuRun's hooks did not return a valid 
       process handle.
* FIX: If a user started "SuRun.exe <TypeYourStringHere>/<AnyCommand>", 
       <AnyCommand> did have no extension and one of 
       <AnyCommand>.[lnk|cmd|bat|com|pif] were present in  
       "[HKCU|HKLM]\Software\Microsoft\Windows\CurrentVersion\App Paths" 
       then SuRun asked for AnyCommand.<ext> to be run as Administrator.
       Example: If you installed Defraggler and then started 
       http://www.piriform.com/Defraggler from Explorer, then SuRun asked 
       if you want to launch C:\Program Files\Defraggler\Defraggler64.exe 
       with elevated rights.
* FIX: ShellExecuteHook asked to start file.exe when file.exe.ext was run

SuRun 1.2.1.1b8 - 2013-09-03
----------------------------
* NEW: Option to hide SuRun's "green smiley" tray icon
* CHG: command lines with "://" before the first " " are ignored by the hooks
* CHG: Application Manifest support for Windows 8 and Windows 8.1
* FIX: Implemented Q&D fix to hook Windows 8.1 "api-ms-win-*-l1-*" DLLs
* FIX: Saving Passwords failed in Windows 2000

SuRun 1.2.1.1b7 - 2013-02-24
----------------------------
* FIX: SuRun's 32Bit IAT-Hook GPF'd in 32Bit Office Help viewer on x64 Win7

SuRun 1.2.1.1b6 - 2013-01-05
----------------------------
* FIX: SuRun's menu item "SuRun: Empty Recycle Bin" was not uninstalled
* FIX: /SWITCHTO did not work in XPx32
* CHG: In Windows Vista and later SuRun leaves UAC "Run as administrator" 
       as it is. This enables <ctrl>+<shift>+<lClick> for pinned programs 
       in Explorer.

SuRun 1.2.1.1b5 - 2012-12-12
----------------------------
* FIX: If you started "SuRun.exe <TypeYourStringHere>/AnyCommand", 
       AnyCommand did have no extension and any of 
       AnyCommand.[lnk|cmd|bat|com|pif] were present in  
       "[HKCU|HKLM]\Software\Microsoft\Windows\CurrentVersion\App Paths" 
       then SuRun asked for AnyCommand.<ext> to be run as Administrator.
       Example: If you installed Defraggler and then started 
       http://www.piriform.com/Defraggler from Explorer, then SuRun asked 
       if you want to launch C:\Program Files\Defraggler\Defraggler64.exe 
       with elevated rights.
* FIX: In any Windows x64 the 32 Bit Hooks started "SuRun32.bin /TestAA" 
       instead of SuRun.exe. This caused SuRun32.bin to block the hooked 
       program for a while and to not elevate the to be started program.
* CHG: Fix for Bug in Online Armor. Online Armor makes any program crash, 
       that is using the Exit Code (~GetCurrentProcessId()).
* CHG: IAT-Hook intercepts ShellExecute and friends because Windows does 
       not support SEE_MASK_NOCLOSEPROCESS in IShellExecuteHook.
* CHG: Option "remember password" in RunAs-Dialog is grayed out when SuRun 
       always asks for the user's password

SuRun 1.2.1.1b4 - 2012-11-27
----------------------------
* FIX: When using SuRun /RunAs, if the user name was cleared, SuRun deleted 
       the HKLM\Security\SuRun Registry key. 
       All SuRun settings in that registry location were lost.
       This caused "SuRunners" members to get full access to the system.
       The Bug appeared with SuRun's /RunAs support in version 1.2.0.0.

SuRun 1.2.1.1b3 - 2012-11-19
----------------------------
* FIX: On all Windows x64 Systems SuRun's hooks did not return a valid 
       process handle until now.
* FIX: SuRun's hooks were not x86 and x64 "cross compatible".
       If an x64 hook catched an x86 call, the hooks could cause a GPF, 
       because SuRun wrote a PROCESS_INFOROMATION structure to the client 
       process and that structure has different sizes on x86 and x64.

SuRun 1.2.1.1b2 - 2012-11-12
----------------------------
* FIX: Implemented Q&D fix to hook Windows 8 "api-ms-win-*-l1-1-1" DLLs
* FIX: Changed permissions of started Apps (SetAdminDenyUserAccess) for 
       Windows 7 Jumplist compatibility
* FIX: ShellExecuteHook asked to start file.exe when file.exe.ext was run
* FIX: User Bitmap Display on confirmation and runas dialogs did not show
* FIX: Long command lines were not displayed ok on confirmation dialogs

SuRun 1.2.1.0 - 2011-12-30
--------------------------
* NEW: French resources by Laurent Hilsz. Thanks!
* NEW: Portuguese language resources by "the.magic.silver.bullet" Thanks!
* NEW: Command "SuRun: Empty recycle bin" in Recycle Bin context menu
* NEW: In Vista and Win7 SuRun's system menu and context menu entries show 
       a SuRun Shield icon
* NEW: /USER <name> command line parameter for specifying the RunAs user
* NEW: /LOW command line parameter to force launching processes non elevated
       /LOW you can even start programs that UAC would not allow to be 
       run low (E.g. Regedit.exe)
* NEW: User SYSTEM is supported for /RUNAS, but only when used by a non
       restricted SuRunner or by a real Administrator
* NEW: InstallSurun and SuRun set DEP permanently ON
* NEW: SuRun's binaries are flagged to use ASLR
* NEW: SuRun's /RunAs has a new "Run elevated" Checkbox for non restricted 
       SuRunners and Administrators
* NEW: The display time for the "program was started automagically message" 
       can be set
* CHG: Removed dependencies on wtsapi32.h and wtsapi32.lib
* CHG: added bin\Crypt32x64.Lib and bin\Crypt32x86.Lib because Crypt32.Lib is 
       missing in VS2005.
* FIX: Two or more spaces after a direct SuRun command line option caused 
  SuRun to just exit. (E.g.: "SuRun /wait  cmd")
* FIX: Fixed implementation of CreateProcessAsuserA IAT-Hook
* FIX: Updating SuRun with a different locale failed.
       An English SuRun could not be updated by a French SuRun because the 
       SuRun service name was localized.
* FIX: "(Re-)Start as Administrator" did not work with captionless Windows
* FIX: IAT-Hook prevented SwitchDesktop in AVAST and caused a system deadlock
* FIX: If SuRun starts %windir%\system32\cmd.exe it inserts a /D option into 
  the command line to avoid cmd from running AutoRuns
* FIX: IAT-Hooks are directly loaded without a separate thread.
  If that GPF's, a thread is created to load the hooks.
* FIX: Command lines with >4096 characters caused a GPF in the SuRun 
  client and the Hooks
* FIX: SuRunExt.Dll prevents unloading SuRunExt.Dll dynamically. This effectively 
  eliminated GPFs in SuRunExt.Dll_unloaded on my Win7pro system.

SuRun 1.2.1.0 rc6 - 2011-12-29
-------------------------------
* NEW: The display time for the "program was started automagically message" 
       can be set

SuRun 1.2.1.0 rc5 - 2011-12-22
-------------------------------
* FIX: Command lines with >4096 characters caused a GPF in the SuRun 
  client and the Hooks

SuRun 1.2.1.0 rc4 - 2011-12-21
-------------------------------
* CHG: "Replace RunAs with SuRuns RunAs" now also handles "runasuser" in 
  Vista++, preserves the Menu visibility ("Extended" Value) and replaces 
  Windows UAC entries by SuRun's "Start as Administrator"

SuRun 1.2.1.0 rc3 - 2011-12-18
-------------------------------
* CHG: SuRun uses a custom Menu icon.

SuRun 1.2.1.0 rc2 - 2011-12-16
-------------------------------
* FIX: Command "SuRun: Empty recycle bin" caused SuRun to sometimes not work
  (Automagic etc.)
* NEW: In Vista and Win7 SuRun's system menu and context entries show the 
  LUA Shield

SuRun 1.2.1.0 rc1 - 2011-12-14
-------------------------------
* NEW: Command "SuRun: Empty recycle bin" in Recycle Bin context menu
* FIX: Two or more spaces after an SuRun Option in the command line caused 
  SuRun to just exit. (E.g.: "SuRun /wait  cmd"

SuRun 1.2.1.0 Beta 10 - 2011-10-17
----------------------------------
* FIX: Fixed implementation of CreateProcessAsuserA IAT-Hook
* FIX: IAT-Hook sometimes failed with ASLR enabled
* NEW: PORTUGUESE language resources by "the.magic.silver.bullet"

SuRun 1.2.1.0 Beta 9 - 2011-07-06
---------------------------------
* NEW: User SYSTEM is supported for /RUNAS, but only when used by a non
       restricted SuRunner or by a real Administrator

SuRun 1.2.1.0 Beta 8 - 2011-06-24
---------------------------------
* NEW: /USER <name> command line parameter for specifying the RunAs user
* NEW: /LOW command line parameter for launching non elevated processes
* FIX: "runas" was not handled by ShellExecuteHook of last beta

SuRun 1.2.1.0 Beta 7 - 2011-06-02
---------------------------------
* NEW: French resources by Laurent Hilsz. Thanks!
* FIX: Updating SuRun with a different locale failed.
       An English SuRun could not be updated by a French SuRun because the 
       SuRun service name was localized.
* NEW: Removed dependencies on wtsapi32.h and wtsapi32.lib
* NEW: added bin\Crypt32x64.Lib and bin\Crypt32x86.Lib because Crypt32.Lib is 
       missing in VS2005.

SuRun 1.2.1.0 Beta 6 - 2011-05-03
---------------------------------
* NEW: partly French resources by Laurent Hilsz. Thanks!
* FIX: "(Re-)Start as Administrator" did not work with captionless Windows
* FIX: IAT-Hook prevented SwitchDesktop in AVAST and caused a system deadlock

SuRun 1.2.1.0 Beta 5 - 2011-03-18
---------------------------------
* FIX: Install Hardware automatically as administrator did not work in all 
    1.2.1.0 Betas.

SuRun 1.2.1.0 Beta 4 - 2011-02-24
---------------------------------
* NEW: InstallSurun and SuRun set DEP permanently ON
* NEW: All files are compiled DEP compatible and with ASLR ON

SuRun 1.2.1.0 Beta 3 - 2011-02-15
---------------------------------
* NEW: SuRun's RunAs has a new "Run elevated" Option for non-restricted 
    SuRunners and Administrators

SuRun 1.2.1.0 Beta 2 - 2011-02-15
---------------------------------
* FIX: If SuRun starts %windir%\system32\cmd.exe it inserts a /D option into 
  the command line to avoid cmd from running AutoRuns

SuRun 1.2.1.0 Beta 1 - 2011-02-01
---------------------------------
* IAT-Hooks are directly loaded without a separate thread.
  When that GPF's, a thread is created to load the hooks.
* SuRunExt.Dll uses GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN,...) to 
  prevent unloading the Dll dynamically. This effectively eliminated GPFs in 
  SuRunExt.Dll_unloaded on my Win7pro system.

SuRun 1.2.0.9 - 2010-12-23
--------------------------
* NEW: In Windows 7 Explorer can be launched with elevated rights.
* NEW: The local System is allowed as user for Backup/Restore.
* NEW: If "Start the program automagically with elevated rights." and "NEVER 
  start this program with elevated rights." are both checked, SuRun directly 
  starts the program with the rights of the current user.
  (Automagic starts Application "non elevated" on "AutoExec" and "AutoCancel")
* NEW: SuRun's ShellExecute Hook let's non-"SuRunners" use SuRun's "Run as..."
* NEW: SuRun can be forced to store and use a user's real password. 
  This hopefully solves problems in domain networks.
* NEW: Polish resources by Junne. Thanks!
* NEW: SuRun handles ShellExecute's "runas" verb "Replace RunAs with SuRuns 
  RunAs" is activated
* NEW: SuRun remembers the last /RunAs user
* CHG: In Vista and Windows 7 "Control Panel as Administrator" starts the 
  "Control Panel Main page"
* CHG: SuRun uses CryptProtectData() for encrypting user passwords. 
  This encryption uses a master key derived from the user's Windows password.
* CHG: If the SuRun service fails to start a Program, it tries to start the 
  Program with impersonation. First as local Administrator and then, as the 
  current user.
* FIX: When using the black high contrast theme, in some of SuRun's windows no 
  text was visible.
* FIX: SuRun's Context Menu Extension adds it's items only once. 
  When right clicking the folder tree in Windows 7 Explorer calls the extension 
  twice, one time for the folder and one time for the folder background.
* FIX: SuRun's Tray symbol is better BlackBox (bBBlean) taskbar compatible
* FIX: SuRun keeps safe Desktop WatchDog happy when scanning for domain users
* FIX: With "Require the user's password" active and timed out in domains for 
  approved programs SuRun GPF'ed silently
* FIX: Hooks are set in a separate thread to avoid GPFs in 
  "SuRunExt.dll_unloaded".
* FIX: Hook initialisation uses an exception filter that restarts Hook
  initialisation on access violation
* FIX: InstallSuRun terminated silently on Windows 7 with Aero.

SuRun 1.2.0.9 rc - 2010-12-22
---------------------------------
* CHG: Updated language resources

SuRun 1.2.0.9 rc2 - 2010-12-13
---------------------------------
* NEW: B&W Icon

SuRun 1.2.0.9 rc1 - 2010-12-03
---------------------------------
* CHG: In Vista/Win7 Control Panel as Administrator" starts "Control Panel Main page"

SuRun 1.2.0.9 Beta15 - 2010-11-08
---------------------------------
* CHG: SuRun uses CryptProtectData() for storing user passwords. 
  This encryption uses a master key derived from the user's Windows password.
* NEW: SuRun can be forced to store and use a user's password. 
  This hopefully solves problems with domain networks.
* NEW: SuRun's ShellExecute Hook let's non-SuRunners use SuRun's "Run as..."
* FIX: When using the black high contrast theme, in some of SuRun's windows no 
  text was visible.
* CHG: ScreenSnap uses CreateDIBSection instead of Get-/SetDIBits

SuRun 1.2.0.9 Beta14 - 2010-08-04
---------------------------------
* If CreateProcessAsUser fails, SuRun's service impersonates first as Admin 
  and if that fails too, it impersonates as logged on user

SuRun 1.2.0.9 Beta13 - 2010-07-21
---------------------------------
* SuRun's service tries to impersonate the user on ERROR_INVALID_PASSWORD

SuRun 1.2.0.9 Beta12 - 2010-06-30
---------------------------------
* SuRuns Context Menu Extension add it's items only once. When righ clicking 
  the folder tree in Win7 Explorer calls the extension twice, one time for the 
  folder and one time for the folder background.
* IAT-Hook explicitly loads SuRunExt.dll to avoid unloading the Dll too early.
  In Win7 x64 a call to LoadLibW() in Unloaded_SuRunExt.dll GPFed.

SuRun 1.2.0.9 Beta11 - 2010-06-28
---------------------------------
* Windows 7 Explorer can be launched with admin rights by cheating the registry
  (AppID\\{CDCBCFCA-3CDC-436f-A4E2-0E02075250C2}","RunAs") THANKS to Junne!!!
* SuRunExtDll export functions wait for Initproc to be done

SuRun 1.2.0.9 Beta10 - 2010-01-12
---------------------------------
* FIX: SuRuns second service instance did not realize if the client process 
    has admin rights and thus it asked the user to become a SuRunners member.

SuRun 1.2.0.9 Beta 9 - 2010-01-11
---------------------------------
* NEW: Local System is accepted as user for Backup/Restore
* NEW: If "Start the program automagically with elevated rights." and "NEVER 
  start this program with elevated rights." are both checked, SuRuns Hooks 
  directly start the program with the rights of the hooked user process.
  (Automagic starts App "non elevated" on "AutoExec" and "AutoCancel")
* CHG: Updated polish language (special characters)
* FIX: Made SuRuns Tray symbol better BlackBox (bBBlean) taskbar compatible
* FIX: SuRun keeps WatchDog happy when scanning for domain users
* FIX: Fixed white backgroud color in SuRun's dialogs

SuRun 1.2.0.9 Beta 8 - 2009-11-23
---------------------------------
*NEW: POLISH resources by junne
*CHG: Updated Dutch resources by Stephan Paternotte
*FIX: Require the user's password after timeout and in domains for approved 
  programs GFP'ed silently

SuRun 1.2.0.9 Beta 7 - 2009-10-18
---------------------------------
* FIX: IAT-Hook initialisation uses an exception filter that restarts 
    initialisation on access violation

SuRun 1.2.0.9 Beta 6 - 2009-10-12
---------------------------------
* FIX: IAT-Hooks are always set in a separate thread (again) to avoid GPFs in 
    "SuRunExt.dll_unloaded".
* FIX: "'SuRun Explorer' here" is disabled and grayed in Windows 7.

SuRun 1.2.0.9 Beta 5 - 2009-10-08
---------------------------------
* FIX: "SuRun /SYSMENUHOOK" caused 100% CPU load on one CPU core
* FIX: Pressing <ENTER> after Install or Update caused to run install or 
    update for another time
* FIX: Tray icon disappeared when "SuRun /SYSMENUHOOK" was started "too early"

SuRun 1.2.0.9 Beta 4 - 2009-10-08
---------------------------------
* FIX: "SuRun /RunAs" now runs processes as limited user for UAC-Admins
* FIX: Tray icon sometimes did not close on SuRun Uninstall/Update
* FIX: Fast clicking or pressing <ENTER> on Install or Update caused to run 
    install or update twice

SuRun 1.2.0.9 Beta 3 - 2009-10-06
---------------------------------
* CHG: SuRun handles RunAs verb for ShellExecute/~Ex when "Replace RunAs with 
    SuRuns RunAs" is activated (Windows 5 only!)

SuRun 1.2.0.9 Beta 2 - 2009-10-09
---------------------------------
* NEW: SuRun remembers the last /RunAs user
* CHG: SuRuns Installer sets the focus to "Logoff"/"Restart" after Installation

SuRun 1.2.0.9 Beta 1 - 2009-10-09
---------------------------------
* CHG: On Windows 7 SuRun does not support and Display "Start as Administrator"
    on the control panel and the recycle bin. Also SuRun does not display 
    "(Re)start as Administrator" in Explorer's system menu
* FIX: HideAppStartCursor() caused InstallSuRun on Win7 with Aero to terminate

SuRun 1.2.0.8 - 2009-09-23
--------------------------
* renamed SuRun 1.2.0.8 Beta 1 to SuRun 1.2.0.8

SuRun 1.2.0.8 Beta 1 - 2009-09-22
---------------------------------
* CHG: The tray symbol shows Processes with unknown Windows Text with their 
    file name
* FIX: Starting with SuRun 1.2.0.7, after a SuRunner started one application as 
    Administrator and the system was not shut down properly (E.g. in cause of a
    power loss), the user's password was messed up. Now SuRun flushes the 
    registry to disk after restoring the user password hash and restores the 
    password hash on power loss.

SuRun 1.2.0.7 - 2009-09-10 (changes to SuRun 1.2.0.6):
--------------------------------
* NEW: Added preliminary Spanish resources
* NEW: Added check for "*UPGRADE*" in file names that need admin rights
* NEW: SuRun supports user bitmaps in Vista and Windows 7 (HKLM\SAM\SAM\
    Domains\Account\Users\<ID>,UserTile)
* NEW: Implemented Fast User Swithing via command line parameters 
    "SuRun /SwitchTo <[domain\]user|Session>".
* CHG: "SuRun /SYSMENUHOOK" will always keep running to be able to close all 
    processes created by SuRun when receiving WM_ENDSESSION
* NEW: Added app.manifest to InstallSuRun for better Vista/Win7 compatibility
* NEW: Added Vista/Win7 compatibility sections to app.manifest
* NEW: Added command line compiling support for VC8 in BuildSuRun.cmd
* NEW: IATHook now also hooks SwitchDesktop
* CHG: Less annoying Balloon Tips
* CHG: SuRun needs to store the password of domain users
* CHG: If blank password use is disabled, SuRun shows a less agressive 
    "The following Administrator accounts have no password" warning
* CHG: In Windows 2000, XP, 2003, SuRun intercepts process creation from 
    services.exe, to enable installing hardware without entering a password
* CHG: Command line parameters for programs to be started are not modified
* CHG: Hovering the mouse over SuRuns Tray Message Windows show a Hand cursor
* CHG: SuRun makes Windows turn off the "AppStarting" mouse cursor
* CHG: Got rid of the "virtualized" status in Windows 7 with new app.manifest
* CHG: IATHook no longer needs ShlWapi.dll
* FIX: IATHook GPF'd with unnamed modules
* FIX: Made hooks NTVDM and Windows 7 compatible
* FIX: Fixed compiling issues with VC8 (VS2005)
* FIX: Reduced registry access from "SuRun /Sysmenuhook"
* FIX: Changed System Menu Hook to modify main system menu only, not sub menus
* FIX: Gregory Maynard-Hoare reported a vunerability, present in SuRun 1.2.0.0 
    to SuRun 1.2.0.7b12. Any dll using hooks was (by the Windows OS!) injected 
    into elevated processes started by SuRun. 
    Thus any program could run elevated code.
* FIX: Desktop background did not respect negative monitor area start values
* FIX: Screen-Fade did not work in Windows 7
* FIX: SuRuns WatchDog event is set in a timer proc instead of a separate thread.
    This insures that if SuRuns GUI is blocked, the WathDog shows up.
* FIX: SuRuns WatchDog did not terminate if SuRuns GUI was "killed"

SuRun 1.2.0.7 rc 2 - 2009-09-08 (changes to SuRun 1.2.0.7 rc 1):
--------------------------------
* FIX: "Install new Hardware automagically as administrator" is disabled and 
    unchecked in Vista++
* FIX: "SuRun /Sysmenuhook" did close on my laptop in cause of a WM_QUIT.
* CHG: IATHook no longer needs ShlWapi.dll
* CHG: IATHook only hooks CreateProcessAsUserW in umpnpmgr.dll in Services.exe
* CHG: IATHook only initializes in a separate thread, if psapi.dll has not been 
    loaded yet

SuRun 1.2.0.7 rc 1 - 2009-09-05 (changes to SuRun 1.2.0.6):
--------------------------------
* NEW: Added preliminary Spanish resources
* NEW: Added check for "*UPGRADE*" in file names that need admin rights
* NEW: IATHook now also hooks SwitchDesktop
* NEW: SuRun supports user bitmaps in Vista and Windows 7 (HKLM\SAM\SAM\
    Domains\Account\Users\<ID>,UserTile)
* NEW: Implemented Fast User Swithing via command line parameters 
    "SuRun /SwitchTo <[domain\]user|Session>".
* CHG: "SuRun /SYSMENUHOOK" will always keep running to be able to close all 
    processes created by SuRun when receiving WM_ENDSESSION
* CHG: Less annoying Balloon Tips
* CHG: SuRun needs to store the password of domain users
* CHG: If blank password use is disabled, SuRun shows a less agressive 
    "The following Administrator accounts have no password" warning
* CHG: In Windows 2000, XP, 2003, SuRun intercepts process creation from 
    services.exe, to enable installing hardware without entering a password
* CHG: Command line parameters for programs to be started are not modified
* CHG: Hovering the mouse over SuRuns Tray Message Windows show a Hand cursor
* CHG: SuRun makes Windows turn off the "AppStarting" mouse cursor
* FIX: Reduced registry access from "SuRun /Sysmenuhook"
* FIX: Changed SysmenuHook to modify main sysmenu only, not sub menus
* FIX: Gregory Maynard-Hoare reported a vunerability, present in SuRun 1.2.0.0 
    to SuRun 1.2.0.7b12. Any dll using hooks was (by the Windows OS!) injected 
    into elevated processes started by SuRun. 
    Thus any program could run elevated code.
* FIX: IATHook GPF'd with unnamed modules
* FIX: Made hooks NTVDM and Windows 7 compatible
* FIX: Desktop background did not respect negative monitor area start values
* CHG: Got rid of the "virtualized" status in Windows 7 with new app.manifest
* FIX: Screen-Fade did not work in Windows 7
* FIX: SuRuns WatchDog event is set in a timer proc instead of a separate thread.
    This insures that if SuRuns GUI is blocked, the WathDog shows up.
* FIX: SuRuns WatchDog did not terminate if SuRuns GUI was "killed"

SuRun 1.2.0.7 Beta 15 - 2009-09-01:
-----------------------------------
* FIX: Fixed some issues with SuRuns Tray symbol
* CHG: In Windows before Vista, when the user logs off, SuRun closes all 
  "Start as administrator"/"Run as..." programs. Thus the user management does 
  not complain about the user registry being accessed while it is unloaded.
* CHG: "SuRun /SYSMENUHOOK" will always keep running to be able to close all 
  processes created by SuRun when receiving WM_ENDSESSION.
* CHG: SuRun settings uncheck "Install new Hardware automagically as 
  administrator" if IAT Hook is disabled

SuRun 1.2.0.7 Beta 14 - 2009-08-31:
-----------------------------------
* CHG: Less annoying Balloon Tips
* FIX: Tray icon status did not update while limited user program were active
* FIX: Removed temp user account. This lead to wrong user names in WhoAmI etc.
* CHG: SuRun needs to logon every user as administrator who uses "Start as 
  Administrator". The only way to get that done right is to put that user 
  temporarily into "Administrators". This leads to the problem, that SuRun 
  normally would need to know (and save!) the password of that user. To avoid 
  that for users who are not joined to a domain, SuRun does the following:
  * create backup of the user's password hash
  * create a new random password for the user
  * put the user to the Administrators group
  * logon the user (this get's the token for "Start as administrator")
  * remove the user from the administrators group
  * restore the saved password hash
  This is safe because:
  * the user cannot logon himself as Admin, since he does not know the 
    random password
  * no password is stored in the registry
  * SuRun never uses/knows the real user password

  For domain users password hashes are not stored in the local registry. 
  So SuRun cannot create a backup of the hash. Thus to use SuRun as a Domain
  user you need to let SuRun store your password in the local registry.
  I currently try to change the encryption password for those users.
  It should dependend on something that is stored on the PDC or in AD.

SuRun 1.2.0.7 Beta 13 - 2009-08-26:
-----------------------------------
* NEW: SuRun uses a local helper account to create the elevated token.
    Now the policy editor and and the user account management of the control 
    panel work for elevated SuRunners.
* CHG: If blank password use is disabled, SuRun shows a less agressive 
    "The following Administrator accounts have no password" warning
* CHG: SuRun no more uses AppInit_DLLs registry key
* CHG: Removed option "set a hook into services and administrative processes."
* CHG: Installer does not require reboot in Vista++
* CHG: The SuRun service injects SuRunExt.dll into services.exe to enable
    installing hardware without entering an admin password on the user desktop
* FIX: Reduced registry access from "SuRun /Sysmenuhook".
* FIX: Changed SysmenuHook to modify main sysmenu only, not sub menus
* FIX: Gregory Maynard-Hoare reported a vunerability, present in SuRun 1.2.0.0 
    to SuRun 1.2.0.7b12. This was fixed with this Beta.

SuRun 1.2.0.7 Beta 12 - 2009-08-17:
-----------------------------------
* Added preliminary Spanish/Argentina resources
* Added check for "*UPGRADE*" in file names that need admin rights
* NEW: Option to set hooks into Admin/Service processes
* NEW: IAT-Hook intercepts CreateProcessAsUser
* NEW: Option to autodetect "rundll32,newdev.dll" for installing new hardware
* Installer requires reboot in Vista/Win7 because SuRun hooks service processes
* FIX: Tray icon and Ballon tips were always shown (since 1.2.0.7 Beta 11)

SuRun 1.2.0.7 Beta 11 - 2009-07-07:
-----------------------------------
* FIX: SuRuns IAT-Hook does not suspend Threads anmore in a process before 
  modifying the IAT. Suspending threads caused NTVDM to not work properly.
* FIX: To make SuRunExt.dll "AppInit_DLLs" compatible, it now initializes in a 
  separate thread. (Win x64 GPF'd)
* FIX: IATHook GPF'd with unnamed modules

SuRun 1.2.0.7 Beta 10 - 2009-06-28:
-----------------------------------
* NEW: IATHook now also hooks SwitchDesktop
* NEW: Vista user bitmap support (HKLM\SAM\SAM\Domains\Account\Users\<ID>,UserTile)
* NEW: SuRun /SwitchTo complains if user is not logged on
* CHG: SuRun uses AppInit_DLLs registry key again
* CHG: Reduced registry access from "SuRun /SYSMENUHOOK"
* FIX: Desktop background did not respect negative monitor area start values
* Win7rc1: Got rid of the "virtualized" status with new app.manifest.
* Win7rc1: IAT-Hook hooks new System dlls
* Win7rc1: Screen-Fade did not work

SuRun 1.2.0.7 Beta 9 - 2009-05-19:
-----------------------------------
* NEW: Implemented Fast User Swithing via command line parameters 
  "SuRun /SwitchTo <[domain\]user|Session>". If that works on most systems, 
  it will be wrapped with a Task-Switcher like GUI.
* FIX: Tray symbol could show information for processes in other sessions

SuRun 1.2.0.7 Beta 8 - 2009-05-08:
-----------------------------------
* FIX: SuRun SuRun 1.2.0.7 Beta 6,7 did close when an OpenFile-Dialog was open

SuRun 1.2.0.7 Beta 7 - 2009-04-26:
-----------------------------------
* Command line Parameters for programs to be started are no longer modified.

SuRun 1.2.0.7 Beta 6 - 2009-04-05:
-----------------------------------
* SuRuns WatchDog event is set in a timer proc instead of a separate thread.
  This insures that if SuRuns GUI is blocked, the WathDog shows up.
* FIX: SuRuns WatchDog did not terminate if SuRuns GUI was killed.

SuRun 1.2.0.7 Beta 5 - 2009-03-10:
-----------------------------------
* Moving the mouse over SuRuns Tray Message Windows show a Hand cursor
* SuRun makes Windows turn off the "AppStarting" mouse cursor

SuRun 1.2.0.6 - 2009-03-01:
-----------------------------------
renamed "SuRun 1.2.0.6 rc 1"  to "SuRun 1.2.0.6"

SuRun 1.2.0.6 rc 1 - 2009-02-24:
-----------------------------------
* SuRuns context menu extension now checks if the user "right clicks" on his
  own or on the common desktop for displaying "control panel as administrator"
* If IShellexecuteHook did not start a program elevated, the IATHook will not 
  try to ask the user. The last command is stored in HKCU\Software\SuRun
* When resolving a command line, SuRun also checks "HKCU/HKLM\Software\
  Microsoft\Windows\CurrentVersion\App Paths". So "SuRun AcroRd32" now works.
* Improved SuRun Icons by Hans Marino. Thanks!

SuRun 1.2.0.6 beta 13 - 2009-01-22:
-----------------------------------
* new Application flag to never ask for a password, even if asking for 
  passwords is enabled. This is for SuRun-AutoRun-Programs.
* Changed program list flag descriptions for (hopefully) better understanding
* SuRuns inter process communication automatically times out after 15 seconds 
  if a client app opens the service pipe without sending data (DOS attack)
* FIX: If a program in the users automagic list was about to be started and 
  the password timed out, SuRun asked the wrong question.
* FIX: If ask for users password was enabled, SuRun Settings did not ask for 
  the password after the timeout.

SuRun 1.2.0.6 beta 12 - 2009-01-15:
-----------------------------------
* new Command line Option /WAIT. SuRun waits until the elevated process exits.
* new Command line Option /RETPID. SuRun returns the Process ID of the 
  elevated process.
* Tray symbol now reappears if Explorer is restarted
* If SuRun uses Vistas Split-Admin token, it is now protected against access 
  from the non-elevated user

SuRun 1.2.0.6 beta 11 - 2008-12-25:
-----------------------------------
* FIX: When importing users SuRun removed all imported users from the local 
    administrators group even when UAC is enabled. Now SuRun leaves UAC admins
    in the local administrators group.
* FIX: The SuRun function "HasRegistryKeyAccess" freed (LocalFree) a DACL 
    pointer that was not allocated. That caused "SuRun /SETUP" to creash in 
    Vista x64. SuRuns unhandled exception filter was not called by the System!

SuRun 1.2.0.6 beta 10 - 2008-12-17:
-----------------------------------
* SuRuns command line resolver did not handle control panel applets correctly
* SuRuns Beta has a new option "/CRASH" to test DrWtsn32

SuRun 1.2.0.6 beta 9 - 2008-12-16:
----------------------------------
* MyCPAU uses impersonation if CreateProvessasUser returns ERROR_DIRECTORY

SuRun 1.2.0.6 beta 8 - 2008-12-16:
----------------------------------
* WatchDog process terminates itself if SuRuns Desktop is no longer running
* Enhanced Debug-Out for "MyCPAU"

SuRun 1.2.0.6 beta 7 - 2008-12-11:
----------------------------------
* FIX: If a safe Windows Desktop is active ("Winlogon", "Disconnect", 
  "Screen-saver"), SuRun did automatically cancel all ShellExecute requests, 
  even those that where marked as "always run elevated" and "no ask".
  Now SuRun does correctly handle apps that are marked as "always run elevated"
  and "no ask" and auto-cancels all requests that require SuRuns safe desktop.
* NEW: Option to adjust, hide and disable the "Cancel" Timeout
* NEW: Service uses the the user's keyboard layout
* NEW: LogonUser uses impersonation
* CHG: RunAs-Logon-Dialog does not always check if Password is valid
* CHG: removed "SuRun /RunAs" support to start programs with elevated rights 
* CHG: removed support of SuRun Confirmation Dialog to switch to "RunAs"

SuRun 1.2.0.6 beta 6 - 2008-11-25:
----------------------------------
* SuRun Message Boxes are system modal
* SuRun does no Desktop fade in/out, when the user is in a Terminal session
* BlackList Match filter does Long/Short file name conversion
* Blacklist "Add" Button does not set quotes
* SuRun /RunAs supports starting programs with elevated rights. To use this
  the invoker of SuRun /RunAs must be Administrator or unrestricted SuRunner
* SuRun Confirmation Dialog supports switching to another User
* On Vista SuRun does to not detect installers per default
* FIX: SuRuns IATHook does not intercept calls to GetProcAddress() any more. 
  This caused Outlook 2007 with Exchange Server and Windows Destkop Search 
  to crash.

SuRun 1.2.0.6 beta 5 - 2008-11-13:
----------------------------------
* SuRuns IAT-Hook suspends all Threads in a process before modifying the IAT.
  Hopefully this finally solves the Outlook 2007 and Eclipse problems.
* SuRun sends Messages to Widows Debug output if something fails while trying 
  to start an elevated process. Hopefully this helps to fix the Problems with 
  SuRun in Domains.
* FIX: "SuRun /BACKUP" did not work at all!
* FIX: "SuRun /BACKUP" and "SuRun /RESTORE" did not support "quoted" file names
* "SuRun /RESTORE" now displays a "File not found" message

SuRun 1.2.0.6 beta 4 - 2008-11-12:
----------------------------------
* "'Administrators' instead of 'Object creator' as default owner for objects 
  created by administrators." policy is left as it is on SuRun Update
* new command line option "/BACKUP <file>"
* VISTA: SuRun works for UAC-Administrators. If a UAC-Administrator is in 
  SuRunners he/she can use SuRun as normal SuRunner.
* VISTA: When UAC is enabled and Admins are put into SuRunners, they are NOT 
    removed from "Administrators"
* VISTA: Split Admins get the elevated token

SuRun 1.2.0.6 beta 3 - 2008-11-01:
----------------------------------
* SuRun can be compiled using Visual C++ 8 (2005)
* SuRuns GUI Windows are moved to the primary Monitor
	
SuRun 1.2.0.6 beta 2 - 2008-10-23:
----------------------------------
* SuRuns GUI is closed when Winlogon shows up (e.G. if a user presses <WIN>+"L" 
  while the SuRun GUI is active).

SuRun 1.2.0.6 beta 1 - 2008-10-19:
----------------------------------
* SuRun uses Winlogons Desktop instead of creating an own one to avoid "Could 
  not create safe desktop" errors.
* SuRuns shows it's beta version number

SuRun 1.2.0.5 - 2008-09-16:
---------------------------
* A click on a Tray-Message-Window now closes it
* SuRun did not honor Windows' Object creator owner policy
* SuRuns blurred Background now shows transluent windows

SuRun 1.2.0.4 - 2008-09-14:
---------------------------
* YetAnotherStupidBug, InstallSuRun installed the 32 Bit SuRun on Windows x64.

SuRun 1.2.0.3 - 2008-09-11:
---------------------------
* FIX: Starting Explorer (Control Panel etc) as Administrator did only work, if 
  Folder Options->"Launch folder windows in a separate process" was activated.

SuRun 1.2.0.2 - 2008-09-10:
---------------------------
* SuRuns blurred screen background looks nicer
* SuRun also shows "Start as Administrator" on the recycle bin folder
* FIX: SuRun could not launch EFS encrypted applications "as Administrator"

SuRun 1.2.0.1 - 2008-08-27:
---------------------------
* InstallSuRun.exe passes command line to SuRun.exe, so "InstallSuRun /INSTALL" 
  will silently install SuRun on a system
* NEW command line switch /RESTORE <SuRunSettings>. 
  One needs to be a real Administrator to restore SuRun settings.
* Command line switch "/INSTALL" accepts <SuRunSettings> parameter.
  E.g. to silently deploy SuRun run "InstallSuRun /INSTALL MySuRunSetup.INI".
  One needs to be a real Administrator to deploy SuRun.
* "RunOnce" apps (that are executed by Explorer while the WinLogon Desktop 
  is still active) that seemed to require administrative privileges (e.g. 
  C:\Program files\Outlook Express\Setup50.exe) did block SuRuns Desktop.
* FIX: Clicking "Set recommended options for home users" with no users in the 
  SuRunners group enabled the check boxes of SuRun Settings Page 2. When then 
  clicking one of these boxes, SuRun Settings were terminated abnormally.
* FIX: InstallSuRun did not delete Temp files when it was started with limited 
  rights because MoveFileEx had no write access to HKLM.
* FIX: InstallSuRun sometimes did not work when run from the Desktop of an 
  limited user because access permissions denied access for Administrators.
* FIX: "Try to detect if unknown applications need to start with 
  elevated rights." could not be turned off.
* FIX: SuRun /RUNAS did not load the correct HKEY_CURRENT_USER
* Calls to LSA (lsass.exe) were extensively reduced.
* All User/Group functions now use impersonation. So they should not fail 
  anymore on domain computers.
* FIX: The Tray Icon caused ~400 context switches per second and thus ~2% CPU 
    Load on "slow" machines.

SuRun 1.2.0.0 - 2008-08-03:
---------------------------
* SuRun does not need nor store user passwords any more.
  SuRun does not need to put SuRunners into the Admin group anymore for 
  starting elevated Processes. SuRun does not need to use a helper process to 
  start the elevated process anymore.
  SuRun does not depend any more on the Secondary Logon Service.
  The new method of starting elevated processes also has one welcome side 
  effect. All Network drives of the logged on user remain connected.
* SuRun works in Windows Vista side by side with UAC
* SuRun can Backup and Restore SuRun settings
* SuRun can Export and Import a users program list
* SuRun uses tricks so that you can start Explorer after an elevated 
  Explorer runs. In Windows Vista SuRun uses Explorers command line switch 
  "/SEPARATE" to start elevated processes.
* To avoid AutoRun startup delays SuRuns service is now loaded in the 
  "PlugPlay" service group, before the network is started.
* SuRun has a Blacklist for Programs not to hook
* New Settings Option to not detect Programs that require elevation
* SuRun Settings have a "Set recommended options for home users" Button. 
* Wildcards "*" and "?" are supported for commands in the users program list
* When adding files to the users program list, SuRun adds automatic quotes.
* Setup does a test run of added/edited commands
* SuRuns "Replace RunAs" now preserves the original "RunAs" display name
* New Setup Option "Hide SuRun from user"
* New Setup Option "Require user password for SuRun Settings"
* New Setup Option "Hide SuRun from users that are not member of the 
  'SuRunners' Group"
* When asking to elevate a process, a "Cancel" count down progress bar is shown
* SuRun does not show "Restart as Admin" in system menu of the shell process
* SuRun does not try to start automagically elevated "AsInvoker" Apps
* "Restart as Admin" Processes are killed only AFTER the user chooses "OK"
* "Restart as Admin" Processes are killed the way TaskManager closes 
  Applications by sending WM_CLOSE to the top level widows of the process, 
  giving the process 5 seconds time to close and then terminating the process.
* FIX: In Windows x64 SuRun32 could not read SuRuns Settings
* FIX: SuRun created an endless loop of "SuRun /USERINST" processes when it 
  was run in the Ceedo sandbox.
* FIX: SuRuns System menu entries did not always show on the first click.
* FIX: When SuRun "Failed to create the safe Desktop" the WatchDog was shown on 
  the users desktop
* Safe Desktop is now created before the screenshot is made to save resources
* The WatchDog process now handles SwitchDesktop issues
* SuRun uses a separate thread to create and fade in the blurred screen
* FIX: When installing applications that require to reboot and to run a "Setup"
  on logon (XP SP3 is an example), SURUN asked to start that setup as Admin but 
  then it DID NOT SWITCH BACK TO THE USER'S DESKTOP 
* SuRun's IAT-Hook only hooks modules that actually use "CreateProcess"
* SuRun's IAT-Hook uses InterlockedExchangePointer for better stability
* Eclipse (JAVA) and SuRuns IAT Hook do not conflict anymore
* The SuRun service uses uses a separate Thread to serve the Tray Icons

SuRun 1.2.0.0 - 2008-07-30: (rc5)
---------------------------
* Added Ctrl+A and Del hotkeys to user's program list and Blacklist
* Added HKLM\Software\SuRun "Language" DWORD= 1:German, 2:English, 3:Dutch
* Minor fixes

SuRun 1.1.9.12 - 2008-07-23: (rc2)
---------------------------
* Updated dutch resources
* When deactivating the Tray Icon SuRun also deactivates balloon tips
* When Hiding SuRun from all users, "Start as Administrator" did do nothing
  on administrator accounts

SuRun 1.1.9.11 - 2008-07-22: (rc1)
---------------------------
* SuRun does not import, export nor set defaults for the Windows Settings
* When uninstalling SuRun removes the Windows settings for "ElevateNonAdmins",
  "NoAutoRebootWithLoggedOnUsers" and "'Administrators' instead of 'Object 
  creator' as default owner for objects created by administrators." ONLY if
  they have been set by SuRun.

SuRun 1.1.9.10 - 2008-07-18: (beta)
---------------------------
* When Uninstalling SuRun, the Windows settings for "ElevateNonAdmins",
  "NoAutoRebootWithLoggedOnUsers" and "'Administrators' instead of 'Object 
  creator' as default owner for objects created by administrators." are not 
  disabled anymore.
* Import/Export on settings page 2 only imports/exports the users program list.
* Settings Backup does not ask for options. Just everything is backed up.
* Cosmetical changes in Settings Dialog Page 1
* Users program list can now have mutliple items selected and deleted at once
* Turning off "Show Expert settings" does not ask nor set defaults anymore

SuRun 1.1.9.9 - 2008-07-17: (beta)
---------------------------
* Cosmetical changes in Settings Dialog Pages 1 and 2
* Removed Help-Button from SuRun Settings Page 2
* FIX: When "Show SuRun settings for experienced users" was disabled, SuRun 
  did always Set recommended defaults on Setup pages 3 and 4
* FIX: SuRun's Tray menu now closes when clicking outside

SuRun 1.1.9.8 - 2008-07-17: (beta)
---------------------------
* Cosmetical changes of Settings page 3
* Removed "Desktop directory not present" message when opening/saving files
* Removed option "Do not use 'SuRunners' group"
* Export/Import is split: 
  -On page 1 you can Import/Export all SuRun settings and SuRunners users
  -On page 2 you can Import/Export the current users WhiteList and Settings

SuRun 1.1.9.7b1 - 2008-07-16: (beta)
---------------------------
* Added option "Do not use 'SuRunners' group"
* After a SuRunner was put to the SuRunners group, SuRun always asked for the 
  password 
* On Import, if no SuRunner is present, "Import Settings for <user>" is 
  grayed out
* If pages 3 or 4 were activated and data with pages 3 and 4 turned off was
  imported, the UI was screwed up
* Before Exporting data, current settings are applied


SuRun 1.1.9.7 - 2008-07-15: (beta)
---------------------------
* Cosmetical stuff in SuRun settings
* Setup Dialog restores last selected tab and user after import
* FIX: Import of "Replace RunAs" failed
* FIX: SuRun did not delete HKLM\Security\SuRun

SuRun 1.1.9.6 - 2008-07-15: (beta)
---------------------------
* Import/Export settings
* Cosmetical improvements for SuRun's Settings Dialog

SuRun 1.1.9.5 - 2008-07-15: (beta)
---------------------------
* SuRun Settings Dialog has now four tabs. The last two contain settings for 
  experienced users and can be hidden.
* Implemented Explorer tricks so that you can start Explorer after an elevated 
  Explorer runs. In Windows Vista SuRun uses Explorers command line switch 
  "/SEPARATE" to start elevated processes.
* To avoid AutoRun startup delays SuRun's service is now loaded in the 
  "PlugPlay" service group, before the network is started.
* Tray Message Windows placement has been improved.

SuRun 1.1.9.4 - 2008-07-12: (beta)
---------------------------
* SuRun Settings dialog is now resizeable
* Tray Message Windows move when Display resolution changes
* Removed indeterminate state for "Show Icon in Taskbar"
* Removed indeterminate state for "Replace RunAs"
* Removed "Restricted SuRunner may start this app" Option. 
  Restricted SuRunners may run programs with elevated rights as specified in 
  their program list.
* Restricted SuRunners are not asked any more if they want to start Setup(etc.) 
  programs with elevated rights
* Button "Set recommended options for home users" sets Settings on all pages
  but only for selected user.
* Options, that new SuRunners may not run Setup and that they and may only run 
  predefined Apps with SuRun were removed
* FIX: Tray Message Windows did not close after 20s
* FIX: All elevated processes were started from %windir%\system32

SuRun 1.1.9.3 - 2008-07-09: (beta)
---------------------------
* Blacklist for Programs not to hook
* Setup-Page 3 revised
* FIX: "Store Password" in RunAs-Dialog was not moved while Dialog reszing 
* FIX: "SuRun /RunAs" did load the wrong user environment
* FIX: Another try to display SuRun in the System Menu on the first click
* FIX: The "Warning for Admins with empty passwords" did not alway show up

SuRun 1.1.9.2 - 2008-07-08: (beta)
---------------------------
* Warning for SuRunners that want to hide SuRun from themselves
* FIX: "Hide SuRun from all non SuRunners" did also hide SuRun Settings from 
  Administrators
* FIX: SuRun did not work with console processes (e.g. cmd.exe)
* FIX: If "hide SuRun from all non SuRunners" was active, SuRun did by default 
  hide from all Users

SuRun 1.1.9.1 - 2008-07-07: (beta)
---------------------------
* SuRun does not store passwords any more. To start a user process, SuRun uses
  zwCreateToken and to RunAs something LSALogonUser is used.
* Setup Option to "store passwords" was changed to "Require user password" 
  with an optional grace period
* New Settings Option to hide SuRun from all non SuRunners
* New Settings Option to not detect Programs that require elevation
* FIX: Empty Admin Password Warning did not show on NonAdmin/NonSuRunners 
  without Tray Icon
* FIX: Settings Page 2 "Show Ballon Tips" was grayed out after "Set recommended 
  options for home users"
* FIX: In Windows x64 SuRun32 could not read SuRun's Settings (since SuRun 1.0!)
* FIX: SuRun created an endless loop of "SuRun /USERINST" processes when it 
  was run in the Ceedo sandbox.
* FIX: SuRun's System menu entries did not always show on the first click.

SuRun 1.1.9.0 - 2008-07-05: (beta)
---------------------------
* SuRun uses LSALogonUser instead of CreateProcessWithLogonW. Processes started
  by SuRun will be terminated by a WinLogon Notification handler in 
  SuRunExt.dll when the user logs off.
  SuRun does not need to put SuRunners into the Admin group anymore for 
  starting elevated Processes. SuRun does not need to use a helper process to 
  start the elevated process anymore.
  SuRun does not depend any more on the Secondary Logon Service.
* SuRun seems to work in Windows Vista side by side with UAC enabled
* SuRun Settigs have a "Set recommended options for home users" Button. 
  Clicking this button causes SuRun to set all Options in the currently visible 
  Settings page to be set to "usual home user" settings
* Wildcards "*" and "?" are supported for commands in the users program list
* When adding files to the users program list, SuRun adds automatic quotes.
* Setup does a test run of added/edited commands
* SuRun's "Replace RunAs" now preserves the original "RunAs" display name
* New Setup Option "Hide SuRun from user"
* New Setup Option "Require user password for SuRun Settings"
* When asking to elevate a process, a "Cancel" count down progress bar is shown
* SuRun does not show "Restart as Admin" in system menu of the shell process
* SuRun does not try to start automagically elevated "AsInvoker" Apps
* "Restart as Admin" Processes are killed only AFTER the user chooses "OK"
* "Restart as Admin" Processes are killed the way TaskManager closes 
  Applications by sending WM_CLOSE to the top level widows of the process, 
  giving the process 5 seconds time to close and then terminating the process.
* When SuRun "Failed to create the safe Desktop" the WatchDog was shown on the 
  users desktop
* Safe Desktop is now created before the screenshot is made to save resources
* The WatchDog process now handles SwitchDesktop issues
* SuRun uses a separate thread to create and fade in the blurred screen
* When installing applications that require to reboot and to run a "Setup" on
  logon (XP SP3 is an example), SURUN asked to start that setup as Admin but 
  then it DID NOT SWITCH BACK TO THE USER'S DESKTOP 
* IATHook only hooks modules that actually use "CreateProcess"
* IATHook uses InterlockedExchangePointer for better stability
* Eclipse (JAVA) and SuRun's IAT Hook seem not to conflict anymore
* The SuRun service uses uses a separate Thread to serve the Tray Icons

SuRun 1.1.0.6 refresh - 2008-05-26:
-----------------------------------
* Win64 "SuRun /SYSMENUHOOK" did not start SuRun32.bin (Mutex)
* Dutch language resources, thanks to Stephan Paternotte

SuRun 1.1.0.6 - 2008-04-29:
----------------------------
* SuRun asks, if a user tries to restart the Windows Shell.
* Removed "Administrators see SuRun's Setup Dialog on their desktop" feature for
  two reasons: 
  1. The Settings dialog for normal users sometimes came up behind the blurred 
     background window. 
  2. The settings dialog was not closed when an Administrator did log off.
* IAT-Hook is again off by default because of incompatibilities with self 
  checking software. (E.G. Access 2003 and Outlook 2003)
* Removed Blurred Desktop flicker

SuRun 1.1.0.5 - 2008-04-23:
----------------------------
* Administrators see SuRun's Setup Dialog on their desktop

SuRun 1.1.0.4 - 2008-04-11:
----------------------------
* FIX: (!!!) The "[Meaning: Explorer My Computer\Control Panel]" display
       screwed up the command line causing SuRun to not work in many cases!

SuRun 1.1.0.3 - 2008-04-11:
----------------------------
* NEW: "SuRun's Settings" appears in the control panels category view in
       "Performance and Maintenance"
* NEW: SuRun displays "[Meaning: Explorer My Computer\Control Panel]" for shell 
       names like "Explorer ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\
       ::{21EC2020-3AEA-1069-A2DD-08002B30309D}"
* NEW: SuRun can replace Windows "Run as..." in context menu
* NEW: Option to show a user status icon and balloon tips in the notification 
       area when the current application does not run as the logged on user
* NEW: SuRun warns non restricted SuRunners and Administrators after login if 
       any Administrators have empty passwords
* NEW: Blurred background screen fades in and out
* NEW: User file list rules: "never start automatically"/"never start elevated"
* NEW: User file list "Add" and "Edit" shows the rule flags in a dialog
* NEW: Initialise desktop background before and close it after SwitchDesktop
* NEW: "Apply" Button in SuRun Settings
* CHG: SuRun's IAT-Hook is turned ON by default
* CHG: If Admin credentials are required, SuRun does not include Domain Admin 
       accounts in the drop down list
* CHG: Double click in user list does "Edit" the command line
* FIX: Scenario: Restircted SuRunner, changed password, SuRun asks to autorun 
       a predefined app with elevated rights, User presses 'Cancel'. Then the
       app was flagged "never start automagically".
* FIX: SuRun's "Run as..." and Administrator authentication respect the 
       "Accounts: Limit local account use of blank passwords to console logon 
       only" group policy.
* FIX: When using Sandboxie and the IAT-Hook, starting a sandboxed program 
       took about four minutes. SuRun now sees a blocked communication with 
       the server and exits immediately.
* FIX: "'Explorer here' as admin" did not open the folder, when it had spaces 
       in the name
* FIX: A user who was not allowed to change SuRun settings, could start the
       control panel as admin and then change SuRun's settings.

SuRun 1.1.0.2 - 2008-03-19:
----------------------------
* In a domain SuRun could not be used without being logged in to the domain. 

SuRun 1.1.0.1 - 2008-03-11:
----------------------------
* The IShellExecuteHook did not work properly because SuRun did not initialise 
  to zero the static variables it uses.

SuRun 1.1.0.0 - 2008-03-10: (changes to SuRun 1.0.2.9)
----------------------------
* SuRun's start menu entries were removed. 'SuRun Settings' and 'Uninstall 
  SuRun' can be done from the control panel.
* SuRun Installation can be done from "InstallSuRun.exe". InstallSuRun contains
  both, the 32Bit and 64Bit version and automatically installs the correct 
  version for your OS.
  Installation/Update is Dialog based with options:
    * "Do not change SuRun's file associations" on Update
    * Run Setup after Install on first Install
    * Show "Set 'Administrators' instead of 'Object creator' as default owner 
      for objects created by administrators." when this was not set before.
  Uninstallation is Dialog based with options
    * Keep all SuRun Settings
    * Delete "SuRunners"
    * Make SuRunners Admins
* SuRun runs in a domain. It enumerates domain accounts for administrative 
  authorization and uses the local group "SuRunners" for local authorization.
* SuRun can be restricted on a per User basis:
  - Users can be denied to use "SuRun setup".
  - Users can be restricted to specific applications that are allowed to run 
    with elevated rights
  This enables to use SuRun in Parent/Children scenarios or in Companies where 
  real Administrators want to work with lowered rights.
* SuRun can intercept the execution of processes by hooking the Import Address 
  Table of Modules (experimental) and by implementing the IShellExecuteHook
  Interface (recommended).
  Each SuRunner can specify a list of programs that will automagically started
  with elevated rights.
  Additionally SuRun parses processes for Vista Manifests and file names.
  -All files with extension msi and msc and all files with extension exe, cmd, 
   lnk, com, pif, bat and a file name that contains install, setup or update 
   are suspected that they must be run with elevated rights.
  -All files with a Vista RT_MANIFEST resource containing <*trustInfo>->
   <*security>-><*requestedPrivileges>-><*requestedExecutionLevel 
   level="requireAdministrator"> are suspected that they must be run with 
   elevated rights.
  The SuRunner can specify to start a program from the List with elevated 
  rights without being asked. If that happens, SuRun can show a Message window 
  on screen that a program was launched with elevated rights.
* FIX: SuRun could be hooked by IAT-Hookers. CreateProcessWithLogonW could 
  be intercepted by an IAT-Hooker and the Credentials could be used to run an 
  administrative process. Now a clean SuRun is started by the Service with 
  "AppInit_Dlls" disabled to do a clean CreateProcessWithLogonW.
* When User is in SuRunners and Explorer runs as Admin, SuRun urges the user 
  to logoff before SuRun can be used.
* Choosing "Don't ask this question again for this program" and pressing 
  cancel causes SuRun to auto-cancel all future requests to run this program
  with elevated rights.
* Non 'SuRunners' will not see any of SuRun's Execution Hooks, System menu 
  ((Re)Start as Administrator) or shell context menu (Control panel/cmd/
  Explorer as Administrator) entries.
* New self made Control Panel Icon
* SuRun tries to locate the Application to be started. So "surun cmd" will
  make ask SuRun whether "C:\Windows\System32\cmd.exe" is allowed to run.
* SuRun's Shell integration can be customized.
* SuRun waits for max 3 minutes after the Windows start for the Service.
* New command line Option: /QUIET
* New Commands. If the User right-clicks on a folder background, two new Items,
  "'SuRun cmd' here" and "'SuRun Explorer' here" are shown.
* Added Context menu for Folders in Explorer (cmd/Explorer <here> as admin)
* "SuRun *.reg" now starts "%Windir%\Regedit.exe *.reg" as Admin
* Added "Start as Admin" for *.reg files
* SuRun is now hidden from the frequently used program list of the Start menu

SuRun 1.0.2.110 - 2008-03-10: (Beta)
----------------------------
* InstallSuRun was not compatible with older SuRun (<=1.0.3.0) versions
* LogonCurrentUser/CurrUserAck did not display User Picture
* Hooks use >84k less stack space
* Safe Desktop uses less stack and seems to be YZshadow tolerant now
* FireFox Install/Update does work now
* Second try for empty passwords. Logon sometimes fails on first try.

SuRun 1.0.2.109 - 2008-03-02: (Beta)
----------------------------
* New self made Control Panel Icon
* The MIDL compiler sometimes did not work when SuRun's IAT-Hook was enabled.
  Now the Exec-Hooks do not allocate memory (calloc) anymore and MIDL works.
* Installation/Update is Dialog based with options:
  * "Do not change SuRun's file associations" on Update
  * Run Setup after Install on first Install
  * Show "Set 'Administrators' instead of 'Object creator' as default owner 
    for objects created by administrators." when this was not set before.
* Uninstallation is Dialog based with options
  * Keep all SuRun Settings
  * Delete "SuRunners"
  * Make SuRunners Admins
* One Installation Container for all SuRun versions "InstallSuRun.exe"

SuRun 1.0.2.108 - 2008-02-24: (Beta)
----------------------------
* FIX: TrayWindow was not size limited
* FIX: FireFox Update did not work with ShellExec-Hook
* FIX: ResolveCommandLine failed with leading Spaces
* SuRun /SysmenuHook exits immediately if no IAT-Hook and no "(Re)Start as 
    Admin" System Menu hooks are used
* SuRun has no Start Menu entry anymore. 
  You can configure and uninstall SuRun from the Control Panel
* BeOrBecomeSuRunner uses different Strings (User/YOU) for Setup/Run

SuRun 1.0.2.107 - 2008-02-22: (Beta)
----------------------------
* SuRun optionally shows a tray window after elevated AutoRunning a process
* FIX: When uninstalling "Start as admin" for *.msc files was not removed
* Setup Dialog hides autorun icons if hooks are disabled
* Setup Dialog hides restriction icons if user is not restricted
* LoadLibrary("Shell32.dll") in WinMain() caused YZShadow to crash SuRun Setup
  When using YZShadow, you must manually add the following to YzShadow.ini:
    EXCLUSION_LIST_CLASSNAME=ScreenWndClass[TAB]#32770
    EXCLUSION_LIST_EXENAME=C:\WINDOWS\surun.exe[TAB]C:\WINDOWS\surun.exe
    ([TAB] means a TAB character (code 09))
* When restarting the SuRun Service all these Settings where set to defaults:
  * "Control Panel As Admin" on Desktop Menu
  * "Cmd here As Admin" on Folder Menu
  * "Explorer here As Admin" on Folder Menu
  * "Restart As Admin" in System-Menu
  * "Start As Admin" in System-Menu
  * Use IAT-Hook
  * Use Shellexec Hook

SuRun 1.0.2.106 - 2008-02-19: (Beta)
----------------------------
* When User is in SuRunners and Explorer runs as Admin, SuRun urges the user 
  to logoff before SuRun can be used.
* When finally uninstalling SuRun, SuRun asks if the SuRunners group should be 
  removed and if all "SuRunners" should become Administrators
* New Setup Options: 
  -what Hooks SuRun should use (ShellExec, IAT-Hook, none)
  -Admins will/not be asked to become SuRunner
  -No one will/not be asked to become SuRunner
  -New SuRunners can/cannot modify SuRun settings
  -New SuRunners are/not restricted to run predefined programs
* "Run as Admin" context menu for the "Control Panel" on "My Computer"
* When User selects to "Cancel", SuRun will not complain that the program 
  could not be run
* IShellExecute Hook is again implemented in SuRun
* "SuRun Settings" Control panel applet

SuRun 1.0.3.0 - 2008-02-15: BackPort Release (changes to SuRun 1.0.2.9)
---------------------------
Because of the vulnerability I ported some features of the current Beta back 
to the release version.
* FIX: SuRun could be hooked by IAT-Hookers. CreateProcessWithLogonW could 
  be intercepted by an IAT-Hooker and the Credentials could be used to run an 
  administrative process. Now a clean SuRun is started by the Service with 
  "AppInit_Dlls" disabled to do a clean CreateProcessWithLogonW.
* SuRun is now hidden from the frequently used program list of the Start menu
* SuRun tries to locate the Application to be started. So "surun cmd" will
  make ask SuRun whether "C:\Windows\System32\cmd.exe" is allowed to run.
* "SuRun *.reg" now starts "%Windir%\Regedit.exe *.reg" as Admin
* Added "Start as Admin" for *.reg files
* Fixed "SuRun %SystemRoot%\System32\control.exe" and
  "SuRun %SystemRoot%\System32\ncpa.cpl"
* fixed command line processing for "SuRun *.msi"

SuRun 1.0.2.105 - 2008-02-15: (Beta)
----------------------------
*FIX: The IAT-Hook could cause circular calls that lead to Stack overflow

SuRun 1.0.2.104 - 2008-02-15: (Beta)
----------------------------
* FIX: Vista does not set a Threads Desktop until it creates a Window.
  This caused a Deadlock because the SuRun client did show a Message Box on 
  the secure Desktop that it does not have access to.
* ShellExecuteHook was replaced by Import Address Table (IAT) Hooking
  WARNING: This is pretty experimental:
  SuRunExt.dll get loaded into all Processes that have a Window or are linked 
  to user32.dll. SuRun intercepts all calls to LoadLibrary, CreateProcess 
  and GetProcAddress. This ables SuRun to run predefined apps with elevated 
  rights and to check for manifests/setup programs more efficiently than with 
  a IShellExecute hook.
* FIX: SuRun could be hooked by IAT-Hookers. CreateProcessWithLogonW could 
  be intercepted by an IAT-Hooker and the Credentials could be used to run an 
  administrative process. Now a clean SuRun is started by the Service with 
  "AppInit_Dlls" disabled to do a clean CreateProcessWithLogonW.
* ShellExtension will be installed/removed with Service Start/Stop

SuRun 1.0.2.103 - 2008-01-21: (Beta)
----------------------------
* ShellExecute Hook and Shell Context menu are now disabled for non "SuRunners"
* The Acronis TrueImage (SwitchDesktop) fix was disabled in SuRun 1.0.2.102

SuRun 1.0.2.102 - 2008-01-20: (Beta)
----------------------------
* ShellExecute Hook and Shell Context menu are now disabled for Administrators
* SuRun now runs in Windows Vista :)) with UAC disabled
* SuRun now Enables/Disables IShellExecHook in Windows Vista
* "SuRun *.reg" now starts "%Windir%\Regedit.exe *.reg" as Admin
* Added "Start as Admin" for *.reg files

SuRun 1.0.2.101 - 2008-01-07: (Beta)
----------------------------
* Fixed "AutoRun" ShellExecuteHook. In an AutoRun.inf in K:\ win an [AutoRun] 
  entry of 'open=setup.exe /autorun' caused the wrong command line 
  'SuRun.exe "setup.exe /autorun" /K:\'
* The english String for <IDS_DELUSER> was missing.

SuRun 1.0.2.100 - 2008-01-07: (Beta)
----------------------------
* Fixed "SuRun %SystemRoot%\System32\control.exe" and
  "SuRun %SystemRoot%\System32\ncpa.cpl"
* Added ShellExecuteHook support for verbs "AutoRun" and "cplopen".
  So SuRun can now automatically start "*.cpl" files and AutoRun.INF entries 
  on removable media with elevated rights
* Choosing "Don't ask this question again for this program" and pressing 
  cancel causes SuRun to auto-cancel all future requests to run this program
  with elevated rights.

SuRun 1.0.2.99 - 2008-01-06: (Beta)
---------------------------
* Removed parsing for Vista Manifests with <*requestedExecutionLevel 
   level="highestAvailable">.

SuRun 1.0.2.98 - 2008-01-05: (Beta)
---------------------------
* SuRun is now hidden from the frequently used program list of the Start menu
* SuRun's file name pattern matching for files with SPACEs did not work.

SuRun 1.0.2.97 - 2008-01-04: (Beta)
---------------------------
* fixed command line processing for "SuRun *.msi"
* changed "'cmd <folder>' as administrator" and "'Explorer <folder>' as 
  administrator" to "'SuRun cmd' here" and "'SuRun Explorer' here"
* Administrators will not see any of SuRun's System menu or shell menu entries
* Added parsing for Vista Manifests and Executable file name pattern matching.
  -All files with extension msi and msc and all files with extension exe, cmd, 
   lnk, com, pif, bat and a file name that contains install, setup or update 
   are suspected that they must be run with elevated rights.
  -All files with a Vista RT_MANIFEST resource containing <*trustInfo>->
   <*security>-><*requestedPrivileges>-><*requestedExecutionLevel 
   level="requireAdministrator|highestAvailable"> are suspected that they must 
   be run with elevated rights.

SuRun 1.0.2.96 - 2007-12-23: (Beta)
---------------------------
* Setup User Interface improvements
* Speedups for Domain computers
* Simplification in the group membership check for SuRunners
* The "Is Client an Admin?" check is now done with the user token of the client 
  process and not with the group membership of the client user name
***Hopefully all speedups and simplifications did not cause security wholes***

SuRun 1.0.2.95 - 2007-12-09: (Beta)
---------------------------
* Acronis TrueImage (SwitchDesktop) caused the users files list control not to 
  be drawn.

SuRun 1.0.2.94 - 2007-12-09: (Beta)
---------------------------
* Context menu for Folders in Explorer (cmd/Explorer <here> as admin)
* When clicking the "Add..." user program button, *.lnk files are resolved to 
  their targets
* File Open/Save Dialogs now show all extensions in SuRun Setup.


SuRun 1.0.2.93 - 2007-12-07: (Beta)
---------------------------
* New Commands. If the User right-clicks on a folder background, two new Items,
  "'cmd <folder>' as administrator" and "'Explorer <folder>' as administrator"
  are shown.
* New command line Option: /QUIET
* New: SuRun runs in a domain. It enumerates domain accounts for administrative 
  authorization and uses the local group "SuRunners" for local authorization.
* SuRun waits for max 3 minutes after the Windows start for the Service.
* SuRun tries to locate the Application to be started. So "surun cmd" will
  make ask SuRun whether "C:\Windows\System32\cmd.exe" is allowed to run.
* Users can make Windows Explorer run specific Applications always with 
  elevated rights. (No "SuRun" command required.)
* SuRun can be restricted on a per User basis:
  - Users can be denied to use "SuRun setup".
  - Users can be restricted to specific applications that are allowed to run 
    with elevated rights
  This enables to use SuRun in Parent/Children scenarios or in Companies where 
  real Administrators want to work with lowered rights.
* SuRun's Shell integration can be customized.

SuRun 1.0.2.9 - 2007-11-17:
---------------------------
* SuRun now sets an ExitCode
* FIX: In Windows XP a domain name can have more than DNLEN(=15) characters.
    This caused GetProcessUsername() to fail and NetLocalGroupAddMembers() 
    to return "1: Invalid Function".
* Fixed a Bug in the LogonDialog that could cause an exception.

SuRun 1.0.2.8 - 2007-10-11:
---------------------------
* Added code to avoid Deadlock with AntiVir's RootKit detector "avipbb.sys" 
  that breaks OpenProcess()
* Added code to recover SuRun's Desktop when user processes call SwitchDesktop
  "shedhlp.exe", part of Acronis True Image Home 11 calls SwitchDesktop() 
  periodically and so switches from SuRun's Desktop back to the users Desktop.

SuRun 1.0.2.7 - 2007-09-21:
---------------------------
* Fixed a Bug in the Sysmenuhook that caused "start as administrator" to
  fire multiple times.

SuRun 1.0.2.6 - 2007-09-20:
---------------------------
* SuRun - x64 Version added, the version number is the same because SuRun 
  has no new functions or bugfixes

SuRun 1.0.2.6 - 2007-09-14:
---------------------------
* With the Option "Store &Passwords (protected, encrypted)" disabled SuRun 
  did not start any Process...Thanks to A.H.Klein for reporting.

SuRun 1.0.2.5 - 2007-09-05:
---------------------------
* Empty user passwords did not work in an out of the box Windows. Users 
  were forced to use the policy editor to set "Accounts: Limit local 
  account use of blank passwords to console logon only" to "Disabled".
  Now SuRun temporarily sets this policy automatically to "Disabled" and 
  after starting the administrative process SuRun restores the policy.

SuRun 1.0.2.4 - 2007-08-31:
---------------------------
* SuRun has been translated to polish by sarmat, Thanks! :-)
* Microsoft Installer Patch Files (*.msp) can be started with elevated rights

SuRun 1.0.2.3 - 2007-08-18:
---------------------------
* SuRun now works with users that have an empty password

SuRun 1.0.2.2 - 2007-07-30:
---------------------------
* Added SuRun Version display to setup dialog caption

SuRun 1.0.2.1 - 2007-07-24:
---------------------------
* The way that SuRun checks a users group membership was changed
* "surun ncpa.cpl" did not work
* SuRun now reports detailed, why a user could not be added or removed to/from 
  a user group
* SuRun now assures that a "SuRunner" is NOT member of "Administrators"
* SuRun now checks that a user is member of "Administrators" or "SuRunners" 
  before launching setup
* SuRun now starts/resumes the "Secondary Logon" service automatically
* SuRun now complains if the windows shell is running as Administrator

SuRun 1.0.2.0 - 2007-05-29:
---------------------------
* SuRun Setup now contains new options:
   -Allow 'SuRunners' to set (and show) the system time
   -Allow 'SuRunners' to change 'Power Options' and select power schemes
   -Show Windows update notifications to all users
   -No auto-restart for scheduled Automatic Windows Update installations
   -Set 'Administrators' instead of 'Object creator' as default owner for 
    objects created by administrators
  The last option is pretty important!

SuRun 1.0.1.2 - 2007-05-16:
---------------------------
* Sven (http://speedproject.de) found a bug in the context menu extension for 
  the Desktop. The Entries for the sub menu "new" were not displayed when 
  SuRun was active.
* All calls GetWindowsDirectory were replaced with GetSystemWindowsDirectory 
  to make SuRun work with Windows 2003 Terminal Server Edition
* Control Panel and Control Panel Applets were not shown in Win2k, Win2k3.
  SuRun now sets the DWORD value "SeparateProcess" in the registry path
  "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" to 1 to
  let Explorer start a separate Process for the control panel. The main control 
  panel is now started with Explorer using the "Workspace\Control Panel" GUIDs.
  Control Panel Applets are now started with RunDLL32 instead of control.exe.

SuRun 1.0.1.1 - 2007-05-13:
---------------------------
* Sven (http://speedproject.de) fixed some typos and beautified the logon 
  dialogs, Thanks!

SuRun 1.0.1.0 - 2007-05-11:
---------------------------
* Added Whitelist for programs that are always run without asking
* When finally uninstalling SuRun the registry is cleaned
* Logon dialogs are resized only if the command line is too long
* Dialogs have a 40s Timeout to automatic "cancel"
* SuRun retries to open the service pipe for 3 minutes. This is useful, when a 
  user starts multiple apps in short intervals. (e.g. from the StartUp menu)

SuRun 1.0.0.1 - 2007-05-09:
---------------------------
* Fixed possible CreateRemoteThread() attack. Access for current user to 
  processes started by SuRun is now denied.

SuRun 1.0.0.0 - 2007-05-08:
---------------------------
* first public release

==============================================================================
                                 by Kay Bruns (c) 2007-15, http://kay-bruns.de
==============================================================================
