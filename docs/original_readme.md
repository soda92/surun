
==============================================================================
SuRun...  Super User Run
                                                      by Kay Bruns (c) 2007-19
==============================================================================

------------------------------------------------------------------------------
SuRun?
------------------------------------------------------------------------------

SuRun eases using Windows 2000 to Windows Vista with limited user rights.

With SuRun you can start applications with elevated rights without needing
administrator credentials.

The idea is simple and was taken from SuDown (http://SuDown.sourceforge.net).
The user usually works with the pc as standard user with limited rights. 
If a program needs administrative rights, the user starts "SuRun <app>". 
SuRun then asks the user in a secure desktop if <app> should really be 
run with administrative rights. If the user acknowledges, SuRun will start 
<app> AS THE CURRENT USER but WITH ADMINISTRATIVE RIGHTS.

SuRun uses an own Windows service to start administrative processes. 
It does not require nor store any user passwords (in non domain scenarios).

To use SuRun a user must be member of the local user group "SuRunners".
If the user is no member of "SuRunners" and tries to use SuRun, (s)he will be 
asked to join "SuRunners". The user must either be an administrator or enter 
administrator credentials to join the SuRunners group.
(SuRun does not store any administrator credentials!)

Members of "SuRunners" can be restricted so that they must not change SuRun's 
settings and to be able to only start predefined applications with elevated 
rights. So parents can allow their children to play games that require 
administrative rights without loosing control of the system. Also employees 
can be allowed to run several applications with elevated rights without being 
able to change the system.

SuRun installs a system wide hook that appends "Start as Administrator..." and 
"Restart as Administrator..." to the system menu of every application that does 
not run as administrator. That enables doing things that you otherwise could 
not do, e.g. setting the Windows clock by double clicking it in the task bar 
notification area would normally display a "Access denied" Message and exit. 
With SuRun you are able to right click the Caption, choose "Restart as 
Administrator..." and to finally set the clock.

SuRun integrates with the windows shell and adds "Start as Administrator..." to 
the Shell context menu of bat, cmd, cpl, exe, lnk, msi, msc and reg files. It 
also adds "'SuRun cmd' here" and "'SuRun Explorer' here" to the context menu of 
folders and "Control panel as administrator" to the Desktop context menu.

SuRun tries to intercept the start of applications. If an application requires 
administrative rights it automatically can be started so. 
SuRun presumes that an application requires administrative rights when:
-it is in the Users List of applications to run with elevated rights
-it has the extension msi or msc
-it has the extension exe, cmd, lnk, com, pif or bat and the file name 
 contains install, setup or update
-it has a MANIFEST resource containing <*trustInfo>-><*security>->
 <*requestedPrivileges>-><*requestedExecutionLevel 
 level="requireAdministrator">
-a file <appname>.manifest in the same folder as the application contains
 <*trustInfo>-><*security>-><*requestedPrivileges>-><*requestedExecutionLevel 
 level="requireAdministrator">

SuRun can be configured with "SuRun Settings" from the Windows control panel.

------------------------------------------------------------------------------
Why not use the built in "Run As..." Windows command?
------------------------------------------------------------------------------

*RunAs can (without any administrative rights) be abused by keyloggers and 
 Import Address Table Hookers to get the credentials of an Administrator.

*Windows loads the registry and environment for the user that you run as.
 If a software is about to be installed, the installation program will see
 the admins HKEY_CURENT_USER and may create registry entries there.
 Also the software sees "C:\Documents and Settings\Administrator" as the users
 profile path.

 SuRun uses the current user account, so all registry entries and file system 
 paths are the same as the user would expect.

------------------------------------------------------------------------------
Why not use SuDown?
------------------------------------------------------------------------------

*SuDown can very easily be used to spy your account password.
 SuDowns password dialog runs in the users desktop and the password can be 
 caught by any application that uses Windows hooks, even by autohotkey.
*SuDown puts every SuDoer, after he logged on, into the Administrators group.
 Spying the password and using it in a call to CreateProcessWithLogonW
 would make the spy running as administrator.
*SuDown starts any process as administrator without asking for permission for 
 a couple of minutes after the user entered the correct password.
*SuDown does not work in a plain Windows 2000 because the windows function 
 "LogOnuser" in Windows 2000 requires a privilege that only system processes 
 have.

------------------------------------------------------------------------------
Why use SuRun?
------------------------------------------------------------------------------

*SuRun uses a secure desktop for sensitive user interaction:
 SuRun uses a service to create a secure desktop in the window station of the 
 users logon session. On that desktop it will ask the user for permission or 
 the password. The desktop is not accessible by user applications. Keyboard 
 and mouse hooks will also not work on that desktop.
*SuRun does not require a password.
*SuRun does not put nor leave the user in the administrators group.

------------------------------------------------------------------------------
Command line switches
------------------------------------------------------------------------------
SuRun supports the following command line:

"SuRun [options] [<program> [parameters]]" to start <program> as Administrator.

options:
  /BACKUP <file>      backup "SuRun Settings" to <file>
  /INSTALL            Install SuRun unattended
  /INSTALL <file>     Install SuRun unattended and do "SuRun /RESTORE <file>"
  /QUIET              do not display an messages
  /RESTORE <file>     restore "SuRun Settings" from <file>
  /RETPID             return the process ID of the elevated process
  /RUNAS [/USER <user>] 
                      start <program> as different user ("SYSTEM" is supported)
  /LOW                /LOW forces the program to be started with low privileges
                      (even if UAC would only allow it to be run elevated)
  /SETUP              start "SuRun Settings"
  /SWITCHTO <User>    Switch Desktop to the session of "User"
  /SWITCHTO <Session> Switch Desktop to logon session [0,1,2...]
  /UNINSTALL          remove SuRun from the system
  /WAIT               wait until the elevated process exits

Below are some SuRun command lines for common Windows tasks:
  Automatic Updates:    "surun wuaucpl.cpl"
  Computer Management:  "surun compmgmt.msc"
  Group Policy:         "surun gpedit.msc"
  Date and Time:        "surun timedate.cpl"
  Network Connections:	"surun ncpa.cpl"
  Security Center:	    "surun wscui.cpl"
  Software:	            "surun appwiz.cpl"
  Local Policy:         "surun secpol.msc"
  System Properties:    "surun sysdm.cpl"

Some applications do not work with SuRun when started with the limited current
user account credentials. These must be started with the credentials of a real 
Administrator account using "SuRun /RUNAS":
  User accounts:        "surun /runas nusrmgr.cpl"
  Windows Update:	      "surun /runas wupdmgr.exe"

------------------------------------------------------------------------------
How to build the sources?
------------------------------------------------------------------------------

To compile SuRun you need Visual C++ 6.0 and Microsoft's Platform SDK for 
Windows Server 2003 R3 or Visual Studio 2005 or 2008 with x86 and x64 tools.

* To compile SuRun with Visual Studio 6, 2005 or 2008 just run BuildSuRun.cmd.

* To manually compile SuRun with VS2005 or 2008, open SuRun(VC9).sln and build
  "SuRun32 Unicode Release|Win32", then "x64 Unicode Release|x64" and at last 
  "Unicode Release|Win32"

* To manually compile SuRun with VC6 you need the last official Platform SDK 
  for VC6, Version 04/2005 ("Windows Server 2003 SP1").
  
  To build the 32Bit version of SuRun, use SuRun.dsw to build "Win32 Unicode 
  Release". 
  SuRun.exe and SuRunExt.dll will be compiled to the directory ReleaseU.

  Compiling the 64 Bit version is a bit more tricky.
  
  In x64, Windows runs two subsystems in parallel, Win64 and Win32. To get the 
  System Menu hook for 32Bit and 64Bit Applications, you must install one Hook 
  for each subsystem. So SuRun64 consists of four files. "SuRun.exe", the Win64
  executable; "SuRunExt.dll" for the Win64 Hooks; "SuRun32.bin", the 32Bit
  executable and "SuRunExt32.dll" for the 32Bit system menu hook.
  -"Open Build Environment Window"->"Windows XP 64-bit Build Environment"
   ->"Set Windows XP x64 Build Environment (Retail)"
  -In the command prompt type "MSDEV.EXE /useenv"
  I use the following batch file for that:
    --------------------------------------------------------------------------
    set VC6Dir=E:\VStudio
    Set MSSDK=E:\MSTOOLS
    call %VC6Dir%\VC98\Bin\VCVARS32.BAT
    call %MSSDK%\SetEnv.Cmd /X64
    start %VC6Dir%\Common\MSDev98\Bin\MSDEV.EXE /useenv
    --------------------------------------------------------------------------
  
  In MSDEV with the AMD64 build environment compile the "Win32 x64 Unicode 
  Release" to get SuRun.exe and SuRunExt.dll.
  Then start MSDEV just as usual with the 32Bit build environment and make 
  "Win32 SuRun32 Unicode Release" to get SuRun32.bin and SuRunExt32.dll.

  After compiling "SuRun - Win32 Unicode Release", "SuRun - Win32 SuRun32 
  Unicode Release" and "SuRun - Win32 x64 Unicode Release" you can compile
  "InstallSuRun - Win32 Release" to build the Install container.
  (You'll need UPX 3.02 or newer in the %path%)
  
------------------------------------------------------------------------------
Changes:
------------------------------------------------------------------------------

SuRun 1.2.1.4 - 2020-09-24 (silent update)
----------------------------
* FIX: Control Panel As Administrator did not work anymore in Windows 10 2004

SuRun 1.2.1.4 - 2019-09-24
----------------------------
* FIX: SuRun's "H.I.P.S. Warning" Window did not show on user's desktop
* FIX: GetEffectiveRightsFromAcl BUGs in Windows 10 1809, replaced the function

SuRun 1.2.1.3 - 2019-01-28
----------------------------
* NEW: SuRun Executables are digitally signed
* NEW: SuRun detects renamed computer and imports user's settings
* NEW: Moderated Restricted Surunners. ("User can only run predefined programs 
			 with elevated rights")
	     If a restricted SuRunner tries to run a non validated Programm with 
	     elevated rights, SuRun will now ask for Administrator credentials to 
	     launch the program elevated in the user's context.
* CHG: Made Hooks Windows 10 1709 compatible.
* CHG: Changed SuRun's Context Menu Strings from "<...>" to "SuRun: <...>"
* FIX: Members of "Power Users" and "Backup Opeators" could not use SuRun
* FIX: Bug in IAT-Hooks made Internet Explorer Crash on Windows 10
* FIX: User Bitmaps for Domain Users were not shown
* FIX: Fixed Spanish Resources

SuRun 1.2.1.2 - 2015-08-09
----------------------------
* CHG: Made Hooks Windows 10 compatible.

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

SuRun 1.2.0.8 - 2009-09-23
--------------------------
* CHG: The tray symbol shows Processes with unknown Windows Text with their 
    file name
* FIX: Starting with SuRun 1.2.0.7, after a SuRunner started one application as 
    Administrator and the system was not shut down properly (E.g. in cause of a
    power loss), the user's password was messed up. Now SuRun flushes the 
    registry to disk after restoring the user password hash and restores the 
    password hash on power loss.

SuRun 1.2.0.7 - 2009-09-10:
---------------------------
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

SuRun 1.2.0.6 - 2009-03-01:
---------------------------
New stuff:
* SuRun works for UAC-Administrators in Windows Vista. If a UAC-Administrator 
  is in SuRunners he/she can use SuRun as normal SuRunner.
* In Windows Vista, Split Admins get a copy of the elevated user token. The 
  token copy is protected against access from the non-elevated token.
* VISTA: On Vista SuRun does to not detect installers per default.
* SuRun uses Winlogons Desktop instead of creating an own one to avoid "Could 
  not create safe desktop" errors.
* SuRuns GUI Windows are moved to the primary Monitor
* Command line option "/BACKUP <file>"
* new Command line Option /WAIT. SuRun waits until the elevated process exits.
* new Command line Option /RETPID. SuRun returns the Process ID of the 
  elevated process. This is for using SuRun from your own program. You now can 
  open a (SYNCHRONIZE) process handle wait for the started process to end.
* new Application flag to never ask for a password, even if asking for 
  passwords is enabled. This is for SuRun-AutoRun-Programs.
* Option to adjust, hide and disable the "Cancel" Timeout.
* Improved SuRun Icons by Hans Marino. Thanks!
* SuRun does no Desktop fade in/out, when the user is in a Terminal session
* BlackList Match filter does Long/Short file name conversion
* When resolving a command line, SuRun also checks "HKCU/HKLM\Software\
  Microsoft\Windows\CurrentVersion\App Paths". So "SuRun AcroRd32" now works.
* Tray symbol now reappears if Explorer is restarted
* Service uses the the user's keyboard layout
* LogonUser uses impersonation.
* SuRun can be compiled using Visual C++ 8 (2005)
Changes:
* Changed program list flag descriptions for (hopefully) better understanding
* SuRuns inter-process-communication now times out after 15 seconds if a client 
  app opens the service pipe without sending data (possible DOS attack).
* WatchDog process terminates itself if SuRuns Desktop is no longer running
* "SuRun /RESTORE" now displays a "File not found" message
* "'Administrators' instead of 'Object creator' as default owner for objects 
  created by administrators." policy is left as it is on SuRun Update
Fixes:
* SuRuns IATHook does not intercept calls to GetProcAddress() any more. This 
  caused Outlook 2007 with Exchange Server and Windows Destkop Search to crash.
* SuRuns context menu extension now checks if the user "right clicks" on the 
  common desktop for displaying "control panel as administrator"
* If IShellexecuteHook did not start a program elevated, the IATHook will not 
  try to ask the user. The last command is stored in HKCU\Software\SuRun
* If a program in the users automagic list was about to be started and the 
  password timed out, SuRun asked the wrong question.
* If ask for users password was enabled, SuRun Settings did not ask for the 
  password after the timeout.
* The SuRun function "HasRegistryKeyAccess" freed (LocalFree) a DACL pointer 
  that was not allocated. That caused "SuRun /SETUP" to crash in Vista x64. 
  SuRuns unhandled exception filter was not called by the System!
* SuRuns command line parser did not handle control panel applets correctly
* If a safe Windows Desktop is active ("Winlogon", "Disconnect", 
  "Screen-saver"), SuRun did automatically cancel all Automagic requests, 
  even those that where marked as "always run elevated" and "no ask".
  Now SuRun does correctly handle apps that are marked as "always run elevated"
  and "no ask" and auto-cancels all requests that require SuRuns safe desktop.
* Blacklist "Add" Button does not set quotes anymore.

SuRun 1.2.0.5 - 2008-09-16:
---------------------------
* A click on a Tray-Message-Window now closes it

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

SuRun 1.2.0.1 - 2008-09-09:
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

SuRun 1.1.0.6 - 2008-05-26:
---------------------------
* Win64 "SuRun /SYSMENUHOOK" did not start SuRun32.bin (Mutex)
* Dutch language resources, thanks to Stephan Paternotte
* Added WatchDog: If SuRun stops responding while the safe desktop is active, a
    Window is shown that allows to switch back to the users desktop. Then on
    the users desktop a window is shown that allows switching back to the safe 
    desktop.
    This is needed, if a Host Intrusion Detection System (HIPS) blocks SuRun 
    and urges the user to allow/deny the action, SuRun is trying. The HIPS 
    usually asks the user on his desktop and this caused a deadlock.
* SuRun asks, if a user tries to restart the Windows Shell.
* IAT-Hook is again off by default because of incompatibilities with self 
  checking software. (E.G. Access 2003 and Outlook 2003)
* Removed Blurred Desktop flicker

SuRun 1.1.0.5 - 2008-04-23:
----------------------------
* was a mistake!

SuRun 1.1.0.4 - 2008-04-11:
----------------------------
* FIX: (!!!) The "[Meaning: Explorer My Computer\Control Panel]" display
       screwed up the command line causing SuRun to not work in many cases!

SuRun 1.1.0.3 - 2008-04-11:
----------------------------
* NEW: "SuRun Settings" appears in the control panels category view in
       "Performance and Maintenance"
* NEW: SuRun displays "[Meaning: Explorer My Computer\Control Panel]" for shell 
       names like "Explorer ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\
       ::{21EC2020-3AEA-1069-A2DD-08002B30309D}"
* NEW: SuRun can replace Windows "Run as..." in context menu
* NEW: User file list rules: "never start automatically"/"never start elevated"
* NEW: Option to show a user status icon and balloon tips in the notification 
       area when the current application does not run as the logged on user
* NEW: SuRun warns non restricted SuRunners and Administrators after login if 
       any Administrators have empty passwords
* NEW: Blurred background screen fades in and out
* NEW: User file list "Add" and "Edit" shows the rule flags in a dialog
* NEW: Initialise desktop background before and close it after SwitchDesktop
* NEW: "Apply" Button in SuRun Settings
* CHG: SuRuns IAT-Hook is turned ON by default
* CHG: If Admin credentials are required, SuRun does not include Domain Admin 
       accounts in the drop down list
* CHG: Double click in user list does "Edit" the command line
* FIX: Scenario: Restircted SuRunner, changed password, SuRun asks to autorun 
       a predefined app with elevated rights, User presses 'Cancel'. Then the
       app was flagged "never start automagically".
* FIX: SuRuns "Run as..." and Administrator authentication respect the 
       "Accounts: Limit local account use of blank passwords to console logon 
       only" group policy.
* FIX: When using Sandboxie and the IAT-Hook, starting a sandboxed program 
       took about four minutes. SuRun now sees a blocked communication with 
       the server and exits immediately.
* FIX: "'Explorer here' as admin" did not open the folder, when it had spaces 
       in the name
* FIX: A user who was not allowed to change SuRun settings, could start the
       control panel as admin and then change SuRuns settings.

SuRun 1.1.0.3 - 2008-03-30:
----------------------------
* QnD Fix: When using Sandboxie and the IAT-Hook, starting a sandboxed 
  program took about four minutes. 
* FIX: "'Explorer here' as admin" did not open the folder, when it had spaces 
  in the name
* FIX: A user who was not allowed to change SuRun settings, could start the
  control panel as admin and then change SuRuns settings.

SuRun 1.1.0.2 - 2008-03-19:
----------------------------
* In a domain SuRun could not be used without being logged in to the domain. 

SuRun 1.1.0.1 - 2008-03-11:
----------------------------
* The IShellExecuteHook did not work properly because SuRun did not initialize 
  to zero the static variables it uses.

SuRun 1.1.0.0 - 2008-03-10: (changes to SuRun 1.0.2.9)
----------------------------
* SuRuns start menu entries were removed. 'SuRun Settings' and 'Uninstall 
  SuRun' can be done from the control panel.
* SuRun Installation can be done from "InstallSuRun.exe". InstallSuRun contains
  both, the 32Bit and 64Bit version and automatically installs the correct 
  version for your OS.
  Installation/Update is Dialog based with options:
    * "Do not change SuRuns file associations" on Update
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
* Non 'SuRunners' will not see any of SuRuns Execution Hooks, System menu 
  ((Re)Start as Administrator) or shell context menu (Control panel/cmd/
  Explorer as Administrator) entries.
* New self made Control Panel Icon
* SuRun tries to locate the Application to be started. So "surun cmd" will
  make ask SuRun whether "C:\Windows\System32\cmd.exe" is allowed to run.
* SuRuns Shell integration can be customized.
* SuRun waits for max 3 minutes after the Windows start for the Service.
* New command line Option: /QUIET
* New Commands. If the User right-clicks on a folder background, two new Items,
  "'SuRun cmd' here" and "'SuRun Explorer' here" are shown.
* Added Context menu for Folders in Explorer (cmd/Explorer <here> as admin)
* "SuRun *.reg" now starts "%Windir%\Regedit.exe *.reg" as Admin
* Added "Start as Admin" for *.reg files
* SuRun is now hidden from the frequently used program list of the Start menu

SuRun 1.0.3.0 - 2008-02-15: BackPort Release
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
* Added code to recover SuRuns Desktop when user processes call SwitchDesktop()
  "shedhlp.exe", part of Acronis True Image Home 11 calls SwitchDesktop() 
  periodically and so switches from SuRuns Desktop back to the users Desktop.

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
    ojects created by administrators
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