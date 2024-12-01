//////////////////////////////////////////////////////////////////////////////
//
// This source code is part of SuRun
//
// Some sources in this project evolved from Microsoft sample code, some from 
// other free sources. The Shield Icons are taken from Windows XP Service Pack 
// 2 (xpsp2res.dll) 
// 
// Feel free to use the SuRun sources for your liking.
// 
//                                (c) Kay Bruns (http://kay-bruns.de), 2007-15
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tchar.h>
#include "helpers.h"
#include "IsAdmin.h"
#include "UserGroups.h"
#include "resstr.h"
#include "SuRunExt/SuRunExt.h"

#define WATCHDOG_EVENT_NAME   _T("SURUN_WATCHDOG_EVENT")

//////////////////////////////////////////////////////////////////////////////
// 
//  HKLM\Security\SuRun keys
// 
//////////////////////////////////////////////////////////////////////////////

#define SVCKEY    _T("SECURITY\\SuRun")

#define PASSWKEY      SVCKEY _T("\\Cache")
#define TIMESKEY      SVCKEY _T("\\Times")
#define WHTLSTKEY(u)  CBigResStr(SVCKEY _T("\\%s"),u)
#define USERKEY(u)    CBigResStr(SVCKEY _T("\\%s\\Settings"),u)

#define RAPASSKEY(u)  CBigResStr(SVCKEY _T("\\RunAs\\%s\\Cache"),u)
#define SHEXOPTKEY     _T("CLSID\\") sGUID _T("\\Options")
#define USROPTKEY(u)  CBigResStr(SHEXOPTKEY _T("\\%s"),u)

#define WINLOGONKEY _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\SuRun")
#define SURUNKEY    _T("SOFTWARE\\SuRun")
//////////////////////////////////////////////////////////////////////////////
// 
//  Macros for all Settings
// 
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//Settings for all users; saved to "HKLM\SECURITY\SuRun":
//////////////////////////////////////////////////////////////////////////////
#define GetOption(s,d)    GetRegInt(HKLM,SVCKEY,s,d)
#define SetOption(s,v,d)  if (GetOption(s,d)!=v)\
                            SetRegInt(HKLM,SVCKEY,s,v)

//blurred user desktop background on secure Desktop
#define GetBlurDesk       (GetOption(_T("BlurDesktop"),1)!=0)
#define SetBlurDesk(b)    SetOption(_T("BlurDesktop"),b,1)

#define GetFadeDesk       (GetOption(_T("FadeDesktop"),1)!=0)
#define SetFadeDesk(b)    SetOption(_T("FadeDesktop"),b,1)

#define GetUseWinLogonDesk  (GetOption(_T("UseWinLogonDesk"),1)!=0)
#define SetUseWinLogonDesk(b) SetOption(_T("UseWinLogonDesk"),b,1)

//Save or not Passwords in the registry
#define GetAskPW         ((GetOption(_T("SavePasswords"),1)==0) || (GetPwTimeOut!=0))

#define GetSavePW         (GetOption(_T("SavePasswords"),1)!=0)
#define SetSavePW(b)      SetOption(_T("SavePasswords"),b,1); \
                          if (!b) \
                          { \
                            DelRegKey(HKLM,PASSWKEY); \
                            DelRegKey(HKLM,TIMESKEY); \
                          }

//Minutes to wait until "Is that OK?" is asked again
#define GetPwTimeOut      min(60,max(0,(int)GetOption(_T("AskTimeOut"),0)))
#define SetPwTimeOut(t)   SetOption(_T("AskTimeOut"),(DWORD)min(60,max(0,(int)t)),0)

//Do not ask an Admin to become a SuRunner
#define GetNoConvAdmin    (GetOption(_T("NoAutoAdminToSuRunner"),0)!=0)
#define SetNoConvAdmin(b) SetOption(_T("NoAutoAdminToSuRunner"),b,0)

//Do not ask anyone to become a SuRunner
#define GetNoConvUser     (GetOption(_T("NoAutoUserToSuRunner"),0)!=0)
#define SetNoConvUser(b)  SetOption(_T("NoAutoUserToSuRunner"),b,0)

//Test Manifest and file name of started files
#define GetTestReqAdmin    (GetOption(_T("TestReqAdmin"),(_winmajor>=6)?0:1)!=0)
#define SetTestReqAdmin(b) SetOption(_T("TestReqAdmin"),b,(_winmajor>=6)?0:1)

//Hide Expert settings
#define GetHideExpertSettings    (GetOption(_T("HideExpertSettings"),1)!=0)
#define SetHideExpertSettings(b) SetOption(_T("HideExpertSettings"),b,1)

//Show warning if Admin has no password to:
#define APW_ALL         0
#define APW_SURUN_ADMIN 1
#define APW_NR_SR_ADMIN 2
#define APW_ADMIN       3
#define APW_NONE        4

#define GetAdminNoPassWarn    GetOption(L"AdminNoPassWarn",APW_NR_SR_ADMIN)
#define SetAdminNoPassWarn(v) SetOption(L"AdminNoPassWarn",v,APW_NR_SR_ADMIN)

//////////////////////////////////////////////////////////////////////////////
//Settings for every user; saved to "HKLM\SECURITY\SuRun\<ComputerName>\<UserName>":
//////////////////////////////////////////////////////////////////////////////
#define GetUsrSetting(u,s,d)    GetRegInt(HKLM,USERKEY(u),s,d)
#define SetUsrSetting(u,s,v,d)  if(GetUsrSetting(u,s,d)!=v)\
                                  SetRegInt(HKLM,USERKEY(u),s,v)

#define DelUsrSettings(u)       if(u && *u) \
                                { \
                                  DelRegKey(HKLM,WHTLSTKEY(u)); \
                                  DelRegKey(HKCR,USROPTKEY(u)); \
                                }
                                
#define DelAllUsrSettings       DelRegKeyChildren(HKLM,SVCKEY);\
                                DelRegKey(HKCR,SHEXOPTKEY)
                                  

//SuRunner is not allowed to run Setup
#define GetNoRunSetup(u)      (GetUsrSetting(u,_T("AdminOnlySetup"),0)!=0)
#define SetNoRunSetup(u,b)    SetUsrSetting(u,_T("AdminOnlySetup"),b,0)
//SuRunner may only run predefined Applications
#define GetRestrictApps(u)    (GetUsrSetting(u,_T("RestrictApps"),0)!=0)
#define SetRestrictApps(u,b)  SetUsrSetting(u,_T("RestrictApps"),b,0)
//SuRunner may install Devices
#define GetInstallDevs(u)     (GetUsrSetting(u,_T("InstallDevs"),GetRestrictApps(u)==0)!=0)
#define SetInstallDevs(u,b)   SetUsrSetting(u,_T("InstallDevs"),b,0)

#define GetStoreUsrPW(u)      GetUsrSetting(u,_T("StoreUsrPW"),0)
#define SetStoreUsrPW(u,b)    if(GetUsrSetting(u,_T("StoreUsrPW"),0)!=b) \
                              { \
                                SetRegInt(HKLM,USERKEY(u),_T("StoreUsrPW"),b); \
                                DeleteTempAdminToken(u); \
                                if (!b) \
                                  DeletePassword(u);  \
                              }

//////////////////////////////////////////////////////////////////////////////
//Shell Extension Settings; stored in: HKCR\\CLSID\\sGUID
//////////////////////////////////////////////////////////////////////////////
#define GetShExtSetting(s,d)    GetRegInt(HKCR,L"CLSID\\" sGUID,s,d)
#define SetShExtSetting(s,v,d)  if (GetShExtSetting(s,d)!=v)\
                                  SetRegInt(HKCR,L"CLSID\\" sGUID,s,v)

//Hide SuRun from non SuRunners
#define GetDefHideSuRun    (GetShExtSetting(_T("HideSuRunFromNonSuRunners"),0)!=0)
#define SetDefHideSuRun(b) SetShExtSetting(_T("HideSuRunFromNonSuRunners"),b,0)

//the following settings are stored in: HKCR\\CLSID\\sGUID
#define ControlAsAdmin  L"ControlAsAdmin"  //"Control Panel As Admin" on Desktop Menu
#define CmdHereAsAdmin  L"CmdHereAsAdmin"  //"Cmd here As Admin" on Folder Menu
#define ExpHereAsAdmin  L"ExpHereAsAdmin"  //"Explorer here As Admin" on Folder Menu
#define RestartAsAdmin  L"RestartAsAdmin"  //"Restart As Admin" in System-Menu
#define StartAsAdmin    L"StartAsAdmin"    //"Start As Admin" in System-Menu

#define UseIShExHook    L"UseIShExHook"    //install IShellExecuteHook
#define UseIATHook      L"UseIATHook"      //install IAT Hook
//#define UseSVCHook      L"UseSVCHook"      //install IAT Hook into services/admin processes
#define UseRemoteThread L"UseRemoteThread" //use CreateRemoteThread

#define ShowAutoRuns    L"ShowAutoRuns"    //use Show Message in Tray

#define UseSuRunGrp     L"UseSuRunners"    //use SuRunners Group

//"Control Panel As Admin" on Desktop Menu
#define GetCtrlAsAdmin        (GetShExtSetting(ControlAsAdmin,1)!=0)
#define SetCtrlAsAdmin(b)     SetShExtSetting(ControlAsAdmin,b,1)
//"Cmd here As Admin" on Folder Menu
#define GetCmdAsAdmin         (GetShExtSetting(CmdHereAsAdmin,0)!=0)
#define SetCmdAsAdmin(b)      SetShExtSetting(CmdHereAsAdmin,b,0)
//"Explorer here As Admin" on Folder Menu
#define GetExpAsAdmin         (GetShExtSetting(ExpHereAsAdmin,1)!=0)
#define SetExpAsAdmin(b)      SetShExtSetting(ExpHereAsAdmin,b,1)
//"Restart As Admin" in System-Menu
#define GetRestartAsAdmin     (GetShExtSetting(RestartAsAdmin,1)!=0)
#define SetRestartAsAdmin(b)  SetShExtSetting(RestartAsAdmin,b,1)
//"Start As Admin" in System-Menu
#define GetStartAsAdmin       (GetShExtSetting(StartAsAdmin,0)!=0)
#define SetStartAsAdmin(b)    SetShExtSetting(StartAsAdmin,b,0)

#define GetUseSuRunGrp        (1/*GetShExtSetting(UseSuRunGrp,1)!=0*/)
//#define SetUseSuRunGrp(b)     SetShExtSetting(UseSuRunGrp,b,1)

//Hook stuff
#define GetUseIShExHook       (GetShExtSetting(UseIShExHook,1)!=0)
#define SetUseIShExHook(b)     SetShExtSetting(UseIShExHook,b,1)
#define GetUseIATHook         (GetShExtSetting(UseIATHook,1)!=0)
#define SetUseIATHook(b)       SetShExtSetting(UseIATHook,b,1)
#define GetUseSVCHook         ((_winmajor<6) && GetUseIATHook /*GetShExtSetting(UseSVCHook,0)!=0)*/)
//#define SetUseSVCHook(b)       SetShExtSetting(UseSVCHook,b,0)

//TrayMsg stuff
#define GetShowAutoRuns       (GetShExtSetting(ShowAutoRuns,1)!=0)
#define SetShowAutoRuns(b)    SetShExtSetting(ShowAutoRuns,b,1)

//Show App admin status in system tray
#define TSA_NONE          0 //No TSA
#define TSA_ALL           1 //TSA for all
#define TSA_ADMIN         2 //TSA for admins
#define TSA_TIPS          8 //show balloon tips
#define TSA_HIDE_NORMAL  16 //hide "green" icon for standard user

#define GetShowTrayAdmin      GetShExtSetting(_T("ShowTrayAdmin"),TSA_ADMIN|TSA_TIPS)
#define SetShowTrayAdmin(b)   SetShExtSetting(_T("ShowTrayAdmin"),b,TSA_ADMIN|TSA_TIPS)

#define GetHideNormalIcon    ((GetShowTrayAdmin&TSA_HIDE_NORMAL)!=0)
#define SetHideNormalIcon(b) SetShowTrayAdmin(((GetShowTrayAdmin&(~TSA_HIDE_NORMAL))|(b?TSA_HIDE_NORMAL:0)))

#define GetHandleRunAs    (GetShExtSetting(_T("HandleRunAs"),1)!=0)
#define SetHandleRunAs(b) SetShExtSetting(_T("HandleRunAs"),b,1)

//Show App admin status in system tray per user setting
#define GetUsrOption(u,s,d)   GetRegInt(HKCR,USROPTKEY(u),s,d)
#define SetUsrOption(u,s,v,d) if(GetUsrOption(u,s,d)!=v)\
                                SetRegInt(HKCR,USROPTKEY(u),s,v)

#define DelUsrOption(u,s)     if (u && *u) RegDelVal(HKCR,USROPTKEY(u),s)

#define GetUserTSA(u)         GetUsrOption(u,_T("ShowTrayAdmin"),2)
#define SetUserTSA(u,v)       SetUsrOption(u,_T("ShowTrayAdmin"),v,2)

#define GetHideFromUser(u)    GetUsrOption(u,_T("HideFromUser"),0)
#define SetHideFromUser(u,h)  SetUsrOption(u,_T("HideFromUser"),h,0)

#define HideSuRun(u,Grps)   ((Grps&IS_IN_ADMINS)?0:GetUsrOption(u,_T("HideFromUser"),\
                              (Grps&IS_IN_SURUNNERS)?0:GetDefHideSuRun))

#define GetReqPw4Setup(u)    GetUsrOption(u,_T("ReqPw4Setup"),0)
#define SetReqPw4Setup(u,b)  SetUsrOption(u,_T("ReqPw4Setup"),b,0)

inline BOOL ShowTray(LPCTSTR u,BOOL bAdmin,BOOL bSuRunner)
{
  if (bSuRunner)
    return (!GetHideFromUser(u)) && (GetUserTSA(u)>0);
  if ((!bAdmin) && GetDefHideSuRun)
    return false;
  switch (GetShowTrayAdmin & (~(TSA_TIPS|TSA_HIDE_NORMAL)))
  {
  case TSA_NONE:
    return false;
  case TSA_ALL:
    return true;
  case TSA_ADMIN:
    return bAdmin;
  }
  return false;
}

inline BOOL ShowBalloon(LPCTSTR u,BOOL bAdmin,BOOL bSuRunner)
{
  if (bSuRunner)
    return (!GetHideFromUser(u)) && (GetUserTSA(u)==2);
  if ((!bAdmin) && GetDefHideSuRun)
    return false;
  DWORD tsa=GetShowTrayAdmin;
  if((tsa & TSA_TIPS)==0)
    return false;
  switch (tsa & (~(TSA_TIPS|TSA_HIDE_NORMAL)))
  {
  case TSA_NONE:
    return false;
  case TSA_ALL:
    return true;
  case TSA_ADMIN:
    return bAdmin;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
//  Windows Policy Stuff
//////////////////////////////////////////////////////////////////////////////

#define SRGetOwnerAdminGrp   (GetShExtSetting(L"DefaultOwnerAdmins",0)!=0)

#define GetOwnerAdminGrp      (GetRegInt(HKLM,\
                              _T("SYSTEM\\CurrentControlSet\\Control\\Lsa"),\
                              _T("nodefaultadminowner"),1)==0)

#define SetOwnerAdminGrp(b)   {SetRegInt(HKLM,\
                              _T("SYSTEM\\CurrentControlSet\\Control\\Lsa"),\
                              _T("nodefaultadminowner"),(b)==0);\
                              SetShExtSetting(L"DefaultOwnerAdmins",b,0);}

#define SRGetWinUpd4All     (GetShExtSetting(L"ShowWindowsUpdateForAll",0)!=0)

#define UACEnabled        (GetRegInt(HKLM,\
                            _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),\
                            _T("EnableLUA"),0)!=0)

#define GetWinUpd4All     (GetRegInt(HKLM,\
                            _T("SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate"),\
                            _T("ElevateNonAdmins"),0)!=0)

#define SetWinUpd4All(b)  {SetRegInt(HKLM,\
                            _T("SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate"),\
                            _T("ElevateNonAdmins"),b);\
                          SetShExtSetting(L"ShowWindowsUpdateForAll",b,0);}

#define SRGetWinUpdBoot     (GetShExtSetting(L"WindowsUpdateNoReboot",0)!=0)

#define GetWinUpdBoot     (GetRegInt(HKLM,\
                            _T("SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU"),\
                            _T("NoAutoRebootWithLoggedOnUsers"),0)!=0)

#define SetWinUpdBoot(b)  {SetRegInt(HKLM,\
                            _T("SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU"),\
                            _T("NoAutoRebootWithLoggedOnUsers"),b);\
                            SetShExtSetting(L"WindowsUpdateNoReboot",b,0);}

#define GetSetEnergy    HasRegistryKeyAccess(_T("MACHINE\\Software\\Microsoft\\")\
                    _T("Windows\\CurrentVersion\\Controls Folder\\PowerCfg"),SURUNNERSGROUP)

#define SetSetEnergy(b) SetRegistryTreeAccess(_T("MACHINE\\Software\\Microsoft\\")\
                    _T("Windows\\CurrentVersion\\Controls Folder\\PowerCfg"),SURUNNERSGROUP,b!=0)

//////////////////////////////////////////////////////////////////////////////
// 
//  Password cache
// 
//////////////////////////////////////////////////////////////////////////////

void LoadPassword(LPTSTR UserName,LPTSTR Password,DWORD nBytes);
void DeletePassword(LPTSTR UserName);
void SavePassword(LPTSTR UserName,LPTSTR Password);

//////////////////////////////////////////////////////////////////////////////
// 
//  Password TimeOut
// 
//////////////////////////////////////////////////////////////////////////////

BOOL PasswordExpired(LPTSTR UserName);
void UpdLastRunTime(LPTSTR UserName);

//////////////////////////////////////////////////////////////////////////////
// 
// WhiteList handling
// 
//////////////////////////////////////////////////////////////////////////////

#define FLAG_DONTASK    0x01 //SuRun will not ask if App can be executed
#define FLAG_SHELLEXEC  0x02 //ShellExecute hook will execute App elevated
//deleted flag: #define FLAG_NORESTRICT 0x04 //Restricted SuRunner may execute App elevated
#define FLAG_AUTOCANCEL 0x08 //SuRun will always answer "cancel"
#define FLAG_CANCEL_SX  0x10 //SuRun will answer "cancel" on ShellExec
#define FLAG_NEVERASK   0x20 //Never ask for password, even after password timeout

BOOL IsInWhiteList(LPCTSTR User,LPCTSTR CmdLine,DWORD Flag);
DWORD GetCommonWhiteListFlags(LPCTSTR User,LPCTSTR CmdLine,DWORD Default);
DWORD GetWhiteListFlags(LPCTSTR User,LPCTSTR CmdLine,DWORD Default);
BOOL AddToWhiteList(LPCTSTR User,LPCTSTR CmdLine,DWORD Flags=0);
BOOL SetWhiteListFlag(LPCTSTR User,LPCTSTR CmdLine,DWORD Flag,bool Enable);
BOOL ToggleWhiteListFlag(LPCTSTR User,LPCTSTR CmdLine,DWORD Flag);
BOOL RemoveFromWhiteList(LPCTSTR User,LPCTSTR CmdLine);

//////////////////////////////////////////////////////////////////////////////
// 
//  BlackList for IATHook
// 
//////////////////////////////////////////////////////////////////////////////
#define HKLSTKEY    SURUNKEY _T("\\DoNotHook")

#define GetSROption(s,d)   GetRegInt(HKCR,SURUNKEY,s,d)
#define SetSROption(s,v,d) if(GetSROption(s,d)!=v)\
                             SetRegInt(HKCR,SURUNKEY,s,v)
#define DelSROption(s)     RegDelVal(HKCR,SURUNKEY,s)

#define GetCancelTimeOut        GetSROption(_T("CancelTimeOut"),40)
#define SetCancelTimeOut(v)     SetSROption(_T("CancelTimeOut"),v,40)

#define GetShowCancelTimeOut    (GetSROption(_T("ShowCancelTimeOut"),0)!=0)
#define SetShowCancelTimeOut(v) SetSROption(_T("ShowCancelTimeOut"),v,0)

#define GetUseCancelTimeOut     (GetSROption(_T("UseCancelTimeOut"),1)!=0)
#define SetUseCancelTimeOut(v)  SetSROption(_T("UseCancelTimeOut"),v,1)

#define GetTrayTimeOut          GetSROption(_T("TrayTimeOut"),20)
#define SetTrayTimeOut(v)       SetSROption(_T("TrayTimeOut"),v,20)

BOOL IsInBlackList(LPCTSTR CmdLine);

//////////////////////////////////////////////////////////////////////////////
// 
//  Registry replace stuff
// 
//////////////////////////////////////////////////////////////////////////////
void ReplaceRunAsWithSuRun(HKEY hKey=HKCR);
void ReplaceSuRunWithRunAs(HKEY hKey=HKCR);

//////////////////////////////////////////////////////////////////////////////
// 
//  Import/Export
// 
//////////////////////////////////////////////////////////////////////////////

//void ImportSettings(LPCTSTR ini,bool bSuRunSettings,bool bBlackList,LPCTSTR User);
//void ExportSettings(LPCTSTR ini,bool bSuRunSettings,bool bBlackList,LPCTSTR User);

//////////////////////////////////////////////////////////////////////////////
// 
//  Setup
// 
//////////////////////////////////////////////////////////////////////////////

BOOL RunSetup(DWORD SessionID,LPCTSTR User);

bool ImportSettings(LPTSTR ini);
void ExportSettings(LPTSTR ini,DWORD SessionID,LPCTSTR User);
