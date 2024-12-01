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
// 
//Quick and dirty resource string class
// 
//////////////////////////////////////////////////////////////////////////////
#pragma once
#pragma warning(disable : 4996)

template<const int _S> class CResourceString
{
public:
  CResourceString()
  {
    m_hInst=GetModuleHandle(0);
    memset(&m_str,0,sizeof(m_str));
  }
  CResourceString(HINSTANCE hInst)
  {
    m_hInst=hInst;
    memset(&m_str,0,sizeof(m_str));
  }
  CResourceString(int nID,...)
  {
    m_hInst=GetModuleHandle(0);
    memset(&m_str,0,sizeof(m_str));
    TCHAR S[_S];
    LoadString(m_hInst,nID,S,_S-1);
    va_list va;
    va_start(va,nID);
    _vsntprintf(m_str,_S-1,S,va);
    va_end(va);
  }
  CResourceString(HINSTANCE hInst,int nID,...)
  {
    m_hInst=hInst;
    memset(&m_str,0,sizeof(m_str));
    TCHAR S[_S];
    LoadString(m_hInst,nID,S,_S-1);
    va_list va;
    va_start(va,nID);
    _vsntprintf(m_str,_S-1,S,va);
    va_end(va);
  }
  CResourceString(LPCTSTR s,...)
  {
    m_hInst=GetModuleHandle(0);
    memset(&m_str,0,sizeof(m_str));
    va_list va;
    va_start(va,s);
    _vsntprintf(m_str,_S-1,s,va);
    va_end(va);
  }
  CResourceString(int nID,va_list va)
  {
    m_hInst=GetModuleHandle(0);
    memset(&m_str,0,sizeof(m_str));
    TCHAR S[_S];
    LoadString(m_hInst,nID,S,_S-1);
    _vsntprintf(m_str,_S-1,S,va);
  }
  CResourceString(HINSTANCE hInst,va_list va)
  {
    m_hInst=hInst;
    memset(&m_str,0,sizeof(m_str));
    TCHAR S[_S];
    LoadString(hInst,nID,S,_S-1);
    _vsntprintf(m_str,_S-1,S,va);
  }
  const LPCTSTR operator =(int nID)
  {
    memset(&m_str,0,sizeof(m_str));
    LoadString(m_hInst,nID,m_str,_S-1);
    return m_str;
  }
  const LPCTSTR operator =(LPCTSTR s)
  {
    memset(&m_str,0,sizeof(m_str));
    _tcsncpy(m_str,s,_S);
    return m_str;
  }
  const TCHAR operator [](int Index)
  {
    return (Index<_S)?((Index>0)?m_str[Index]:'\0'):'\0';
  }
  operator LPTSTR() { return m_str; };
  operator LPCTSTR(){ return m_str; };
protected:
  TCHAR m_str[_S];
  HINSTANCE m_hInst;
};

typedef CResourceString<256> CResStr;
typedef CResourceString<4096> CBigResStr;