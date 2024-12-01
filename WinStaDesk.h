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

//////////////////////////////////////////////////////////////////////////////
// Home of the CRunOnNewDeskTop class
//
// this class creates a new desktop in a given window station, captures the 
// screen of the current desktop, switches to the new desktop and sets the 
// captured screen as blurred backgroud.
//
// in the class destructor it swiches back to the users desktop
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ScreenSnap.h"

//This class creates a new desktop with a darkened blurred image of the 
//current dektop as background.
class CRunOnNewDeskTop
{
public:
  CRunOnNewDeskTop(LPCTSTR WinStaName,LPCTSTR DeskName,LPCTSTR UserDesk,bool bCreateBkWnd,bool bFadeIn);
  ~CRunOnNewDeskTop();
  bool IsValid();
  void CleanUp();
  void FadeOut();
  BOOL SwitchToOwnDesk();
  void SwitchToUserDesk();
  HWND GetDeskWnd(){return m_Screen.hWnd();};
private:
  bool    m_bOk;
  HWINSTA m_hwinstaSave;
  HWINSTA m_hwinstaUser;
  HDESK   m_hdeskSave;
  HDESK   m_hdeskUser;
  HDESK   m_hDeskSwitch;
public:
  CBlurredScreen m_Screen;
};

extern CRunOnNewDeskTop* g_RunOnNewDesk;

// WindowStation Desktop Names:

//Call SetEvent(g_WatchDogEvent) in a WM_TIMER for a period of less than two 
//seconds on the Safe desktop to prevent the watchdog process from displaying 
//it's dialog on the input desktop
//extern HANDLE g_WatchDogEvent;

BOOL GetWinStaName(LPTSTR WinSta,DWORD ccWinSta);
BOOL GetDesktopName(LPTSTR DeskName,DWORD ccDeskName);

void SetProcWinStaDesk(LPCTSTR WinSta,LPCTSTR Desk);
void SetAccessToWinDesk(HANDLE htok,LPCTSTR WinSta,LPCTSTR Desk,BOOL bGrant);

bool CreateSafeDesktop(LPTSTR WinSta,LPCTSTR UserDesk,bool BlurDesk,bool bFade);
void DeleteSafeDesktop(bool bFade);
