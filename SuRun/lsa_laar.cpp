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
// This code is based on 'MVPs.org's LsaAddAccountRights sample
//    http://win32.mvps.org/lsa/lsa_leawur.cpp
//////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x500
#define WINVER       0x500
#include <windows.h>
#include <ntsecapi.h>
#include <TCHAR.h>
#include "lsa_laar.h"
#pragma warning(disable : 4996)

class LsaUnicodeString: public _LSA_UNICODE_STRING
{
public:
	LsaUnicodeString( int maxSize = 512 )
  {
	  Buffer = NULL;
	  init( maxSize );
  }
	LsaUnicodeString( char *s )
  {
    MaximumLength=0;
	  Buffer = NULL;
    Set(s);
  }
	LsaUnicodeString( wchar_t *s )
  {
    MaximumLength=0;
	  Buffer = NULL;
    Set(s);
  }
	~LsaUnicodeString()
  {
	  if ( Buffer )
		  free( Buffer );
  }
	void init( int maxSize = 512 )
  {
	  if ( Buffer )
		  free( Buffer );
	  Length = 0;
	  MaximumLength = (USHORT)maxSize;
	  Buffer = ( maxSize == 0 )? NULL: (wchar_t *) calloc( maxSize, 1 );
  }
  void Set( char *s )
  {
    int l=(int)strlen( s ) * sizeof(wchar_t);
    if ( l >= MaximumLength )
      init( l );
    MultiByteToWideChar( CP_ACP, 0, s, l/sizeof(wchar_t), Buffer, l );
    Length = (USHORT)l;
  }
  void Set( wchar_t *s )
  {
    int l=(int)wcslen( s ) * sizeof(wchar_t);
	  if ( l >= MaximumLength )
		  init( l );
	  memmove(Buffer,s,l);
	  Length = (USHORT)l;
  }
	operator LSA_UNICODE_STRING *()
  {
	  return static_cast<LSA_UNICODE_STRING *>( this );
  }
	operator char *()
  {
	  char *ansiBuf= (char *) calloc( Length / sizeof(wchar_t) + 1, 1 );
	  WideCharToMultiByte( CP_ACP, 0, Buffer, Length / sizeof(wchar_t), 
      ansiBuf, Length + sizeof(wchar_t) / 1, NULL, NULL );
	  return ansiBuf;
  }
	operator wchar_t *()
  {
	  wchar_t *Buf = (wchar_t *) calloc( Length + 1, 1 );
	  memmove(Buf,Buffer,Length);
	  return Buf;
  }
	const LsaUnicodeString& operator=( char *s )
  {
    Set(s);
	  return *this;
  }
	const LsaUnicodeString& operator=( wchar_t *s )
  {
    Set(s);
	  return *this;
  }
	const BOOL operator==( LSA_UNICODE_STRING& s )
  {
    if (Length!=s.Length)
      return FALSE;
	  return wcsnicmp(Buffer,s.Buffer,Length/sizeof(wchar_t))==0;
  }
};

#define RET_ERR(e) if (e) { RetLastErr=LsaNtStatusToWinError(e); goto CleanUp; }

#ifndef STATUS_SOME_NOT_MAPPED
#define STATUS_SOME_NOT_MAPPED ((NTSTATUS)0x00000107L)
#endif 

BOOL AccountPrivilege(LPTSTR Account,LPTSTR Privilege,PrivOp op)
{
  BOOL RetVal=TRUE;
  DWORD RetLastErr=NOERROR;
	LsaUnicodeString acct=Account;
	LsaUnicodeString priv=Privilege;
	NTSTATUS nts;
	LSA_HANDLE hPol;
  PSID sid=0;
	PLSA_REFERENCED_DOMAIN_LIST refDomList = NULL;
	PLSA_TRANSLATED_SID sidList = NULL;
  PLSA_UNICODE_STRING Rights=0;
	// open the policy object on the target computer
	static SECURITY_QUALITY_OF_SERVICE sqos =
		{ sizeof SECURITY_QUALITY_OF_SERVICE, SecurityImpersonation, SECURITY_DYNAMIC_TRACKING, FALSE };
	static LSA_OBJECT_ATTRIBUTES lsaOA =
		{ sizeof LSA_OBJECT_ATTRIBUTES, NULL, NULL, 0, NULL, &sqos };
	RET_ERR(LsaOpenPolicy(0,&lsaOA,GENERIC_READ|GENERIC_EXECUTE|
    POLICY_LOOKUP_NAMES|POLICY_CREATE_ACCOUNT,&hPol));
	// translate the account name to a RID plus associated domain SID
	nts = LsaLookupNames(hPol,1,acct,&refDomList,&sidList );
	if ( nts == STATUS_SOME_NOT_MAPPED )
		nts = 0;
	RET_ERR(nts );
	// build list of complete SIDs here
	switch ( sidList->Use )
	{
		case SidTypeAlias:
		case SidTypeUser:
		case SidTypeGroup:
		case SidTypeWellKnownGroup:
      {
        PSID s= refDomList->Domains[sidList->DomainIndex].Sid;
        UCHAR ssac=*GetSidSubAuthorityCount( s );
        DWORD sidlen = GetSidLengthRequired((BYTE) 1 + ssac);
        sid = malloc( sidlen ); // max SID length (we hope)
        InitializeSid( sid, GetSidIdentifierAuthority( s ),(BYTE) 1 + ssac );
        DWORD d;
        for (d = 0; d < (DWORD)ssac; ++ d )
          *GetSidSubAuthority( sid, d ) = *GetSidSubAuthority( s, d );
        *GetSidSubAuthority( sid, d ) = sidList->RelativeId;
        switch (op)
        {
        case AddPrivilege:
		      RET_ERR(LsaAddAccountRights(hPol,sid,(LSA_UNICODE_STRING*)&priv,1));
          break;
        case DelPrivilege:
		      RET_ERR(LsaRemoveAccountRights(hPol,sid,0,(LSA_UNICODE_STRING*)&priv,1));
          break;
        case HasPrivilege:
          {
            DWORD nRights=0;
            RET_ERR(LsaEnumerateAccountRights(hPol,sid,&Rights,&nRights));
            RetVal=FALSE;
            for (DWORD r=0;r<nRights;r++)
              if (priv==Rights[r])
              {
                RetVal=TRUE;
                break;
              }
          }
          break;
        }
      }
			break;
	}
CleanUp:
	LsaClose( hPol );
	if ( sid != NULL )
		free( sid );
  if(refDomList)
    LsaFreeMemory(refDomList);
  if (sidList)
    LsaFreeMemory(sidList);
  if(Rights)
    LsaFreeMemory(Rights);
  SetLastError(RetLastErr);
  return (RetLastErr!=NOERROR)?FALSE:RetVal;
}

LPWSTR GetAccountPrivileges(LPWSTR Account)
{
  DWORD RetLastErr=NOERROR;
	LsaUnicodeString acct=Account;
	NTSTATUS nts;
	LSA_HANDLE hPol;
  PSID sid=0;
	PLSA_REFERENCED_DOMAIN_LIST refDomList = NULL;
	PLSA_TRANSLATED_SID sidList = NULL;
  PLSA_UNICODE_STRING Rights=0;
  LPWSTR sRet=0;
	// open the policy object on the target computer
	static SECURITY_QUALITY_OF_SERVICE sqos =
		{ sizeof SECURITY_QUALITY_OF_SERVICE, SecurityImpersonation, SECURITY_DYNAMIC_TRACKING, FALSE };
	static LSA_OBJECT_ATTRIBUTES lsaOA =
		{ sizeof LSA_OBJECT_ATTRIBUTES, NULL, NULL, 0, NULL, &sqos };
	RET_ERR(LsaOpenPolicy(0,&lsaOA,GENERIC_READ|GENERIC_EXECUTE|POLICY_LOOKUP_NAMES,&hPol));
	// translate the account name to a RID plus associated domain SID
	nts = LsaLookupNames(hPol,1,acct,&refDomList,&sidList );
	if ( nts == STATUS_SOME_NOT_MAPPED )
		nts = 0;
	RET_ERR(nts );
	// build list of complete SIDs here
	switch ( sidList->Use )
	{
		case SidTypeAlias:
		case SidTypeUser:
		case SidTypeGroup:
		case SidTypeWellKnownGroup:
      {
        PSID s= refDomList->Domains[sidList->DomainIndex].Sid;
        UCHAR ssac=*GetSidSubAuthorityCount( s );
        DWORD sidlen = GetSidLengthRequired((BYTE) 1 + ssac);
        sid = malloc( sidlen ); // max SID length (we hope)
        InitializeSid( sid, GetSidIdentifierAuthority( s ),(BYTE) 1 + ssac );
        DWORD d;
        for (d = 0; d < (DWORD)ssac; ++ d )
          *GetSidSubAuthority( sid, d ) = *GetSidSubAuthority( s, d );
        *GetSidSubAuthority( sid, d ) = sidList->RelativeId;
        DWORD nRights=0;
        RET_ERR(LsaEnumerateAccountRights(hPol,sid,&Rights,&nRights));
        DWORD sLen=0;
        DWORD r;
        for (r=0;r<nRights;r++)
          sLen+=Rights[r].Length+sizeof(WCHAR);
        sRet=(LPWSTR)calloc(sLen+sizeof(WCHAR),1);
        if (!sRet)
          goto CleanUp;
        BYTE* S=(BYTE*)sRet;
        for (r=0;r<nRights;r++)
        {
          memmove(S,Rights[r].Buffer,Rights[r].Length);
          S+=Rights[r].Length+sizeof(WCHAR);
        }
      }
	}
CleanUp:
	LsaClose( hPol );
	if ( sid != NULL )
		free( sid );
  if(refDomList)
    LsaFreeMemory(refDomList);
  if (sidList)
    LsaFreeMemory(sidList);
  if(Rights)
    LsaFreeMemory(Rights);
  SetLastError(RetLastErr);
  return sRet;
}

PSID GetAccountSID(LPWSTR Account)
{
  DWORD RetLastErr=NOERROR;
	LsaUnicodeString acct=Account;
	NTSTATUS nts;
	LSA_HANDLE hPol;
  PSID sid=0;
	PLSA_REFERENCED_DOMAIN_LIST refDomList = NULL;
	PLSA_TRANSLATED_SID sidList = NULL;
  PLSA_UNICODE_STRING Rights=0;
	// open the policy object on the target computer
	static SECURITY_QUALITY_OF_SERVICE sqos =
		{ sizeof SECURITY_QUALITY_OF_SERVICE, SecurityImpersonation, SECURITY_DYNAMIC_TRACKING, FALSE };
	static LSA_OBJECT_ATTRIBUTES lsaOA =
		{ sizeof LSA_OBJECT_ATTRIBUTES, NULL, NULL, 0, NULL, &sqos };
	RET_ERR(LsaOpenPolicy(0,&lsaOA,POLICY_LOOKUP_NAMES,&hPol));
	// translate the account name to a RID plus associated domain SID
	nts = LsaLookupNames(hPol,1,acct,&refDomList,&sidList );
	if ( nts == STATUS_SOME_NOT_MAPPED )
		nts = 0;
	RET_ERR(nts );
	// build list of complete SIDs here
	switch ( sidList->Use )
	{
		case SidTypeAlias:
		case SidTypeUser:
		case SidTypeGroup:
		case SidTypeWellKnownGroup:
      {
        PSID s= refDomList->Domains[sidList->DomainIndex].Sid;
        UCHAR ssac=*GetSidSubAuthorityCount( s );
        DWORD sidlen = GetSidLengthRequired((BYTE) 1 + ssac);
        sid = malloc( sidlen ); // max SID length (we hope)
        InitializeSid( sid, GetSidIdentifierAuthority( s ),(BYTE) 1 + ssac );
        DWORD d;
        for (d = 0; d < (DWORD)ssac; ++ d )
          *GetSidSubAuthority( sid, d ) = *GetSidSubAuthority( s, d );
        *GetSidSubAuthority( sid, d ) = sidList->RelativeId;
      }
	}
CleanUp:
	LsaClose( hPol );
  if(refDomList)
    LsaFreeMemory(refDomList);
  if (sidList)
    LsaFreeMemory(sidList);
  if(Rights)
    LsaFreeMemory(Rights);
  SetLastError(RetLastErr);
  return sid;
}
