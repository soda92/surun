///////////////////////////////////////////////////////////////////////////////
// This code was taken from the: SSPI Authentication Sample
// 
//  This program demonstrates how to use SSPI to authenticate user credentials.
// 
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
// 
//  Copyright (C) 2001.  Microsoft Corporation.  All rights reserved.
///////////////////////////////////////////////////////////////////////////////
#define SECURITY_WIN32
#include <windows.h>
#include <tchar.h>
#include <sspi.h>
#include "sspi_auth.h"
#include "DBGTrace.h"

typedef struct _AUTH_SEQ 
{
  BOOL fInitialized;
  BOOL fHaveCredHandle;
  BOOL fHaveCtxtHandle;
  CredHandle hcred;
  struct _SecHandle hctxt;
} AUTH_SEQ,*PAUTH_SEQ;

/////////////////////////////////////////////////////////////////////////////// 
// 
// LoadSecurityDll
// 
/////////////////////////////////////////////////////////////////////////////// 
ACCEPT_SECURITY_CONTEXT_FN       _AcceptSecurityContext     = NULL;
ACQUIRE_CREDENTIALS_HANDLE_FN    _AcquireCredentialsHandle  = NULL;
COMPLETE_AUTH_TOKEN_FN           _CompleteAuthToken         = NULL;
DELETE_SECURITY_CONTEXT_FN       _DeleteSecurityContext     = NULL;
FREE_CONTEXT_BUFFER_FN           _FreeContextBuffer         = NULL;
FREE_CREDENTIALS_HANDLE_FN       _FreeCredentialsHandle     = NULL;
INITIALIZE_SECURITY_CONTEXT_FN   _InitializeSecurityContext = NULL;
QUERY_SECURITY_PACKAGE_INFO_FN   _QuerySecurityPackageInfo  = NULL;
QUERY_SECURITY_CONTEXT_TOKEN_FN  _QuerySecurityContextToken = NULL;

BOOL LoadSecurityDll() 
{
  HMODULE hSecDLL = LoadLibrary(_T("security.dll"));
  if (!hSecDLL)
    return FALSE;
  _AcceptSecurityContext=(ACCEPT_SECURITY_CONTEXT_FN) GetProcAddress(hSecDLL,"AcceptSecurityContext");
  _CompleteAuthToken    =(COMPLETE_AUTH_TOKEN_FN)     GetProcAddress(hSecDLL,"CompleteAuthToken");
  _DeleteSecurityContext=(DELETE_SECURITY_CONTEXT_FN) GetProcAddress(hSecDLL,"DeleteSecurityContext");
  _FreeContextBuffer    =(FREE_CONTEXT_BUFFER_FN)     GetProcAddress(hSecDLL,"FreeContextBuffer");
  _FreeCredentialsHandle=(FREE_CREDENTIALS_HANDLE_FN) GetProcAddress(hSecDLL,"FreeCredentialsHandle");
  _QuerySecurityContextToken=(QUERY_SECURITY_CONTEXT_TOKEN_FN) GetProcAddress(hSecDLL,"QuerySecurityContextToken");
#ifdef UNICODE
  _AcquireCredentialsHandle=(ACQUIRE_CREDENTIALS_HANDLE_FN)GetProcAddress(hSecDLL,"AcquireCredentialsHandleW");
  _InitializeSecurityContext=(INITIALIZE_SECURITY_CONTEXT_FN)GetProcAddress(hSecDLL,"InitializeSecurityContextW");
  _QuerySecurityPackageInfo=(QUERY_SECURITY_PACKAGE_INFO_FN)GetProcAddress(hSecDLL,"QuerySecurityPackageInfoW");
#else
  _AcquireCredentialsHandle = (ACQUIRE_CREDENTIALS_HANDLE_FN)GetProcAddress(hSecDLL,"AcquireCredentialsHandleA");
  _InitializeSecurityContext = (INITIALIZE_SECURITY_CONTEXT_FN)GetProcAddress(hSecDLL,"InitializeSecurityContextA");
  _QuerySecurityPackageInfo = (QUERY_SECURITY_PACKAGE_INFO_FN)GetProcAddress(hSecDLL,"QuerySecurityPackageInfoA");
#endif
  return _AcceptSecurityContext && _DeleteSecurityContext && _FreeContextBuffer 
    && _FreeCredentialsHandle && _AcquireCredentialsHandle && _InitializeSecurityContext 
    && _QuerySecurityPackageInfo && _CompleteAuthToken && _QuerySecurityContextToken;
}


/////////////////////////////////////////////////////////////////////////////// 
//
// GenClientContext
// 
// Optionally takes an input buffer coming from the server and returns
// a buffer of information to send back to the server.  Also returns
// an indication of whether or not the context is complete.
/////////////////////////////////////////////////////////////////////////////// 
BOOL GenClientContext(PAUTH_SEQ pAS,PSEC_WINNT_AUTH_IDENTITY pAuthIdentity, 
                      PVOID pIn,DWORD cbIn,PVOID pOut,PDWORD pcbOut,PBOOL pfDone) 
{
  SECURITY_STATUS ss;
  TimeStamp       tsExpiry;
  SecBufferDesc   sbdOut;
  SecBuffer       sbOut;
  SecBufferDesc   sbdIn;
  SecBuffer       sbIn;
  ULONG           fContextAttr;
  if (!pAS->fInitialized) 
  {
    ss = _AcquireCredentialsHandle(NULL,_T("NTLM"),SECPKG_CRED_OUTBOUND,NULL,
      pAuthIdentity,NULL,NULL,&pAS->hcred,&tsExpiry);
    if (ss < 0) 
    {
      DBGTrace1("AcquireCredentialsHandle failed: %s",GetErrorNameStatic(ss));
      return FALSE;
    }
    pAS->fHaveCredHandle = TRUE;
  }
  // Prepare output buffer
  sbdOut.ulVersion  = 0;
  sbdOut.cBuffers   = 1;
  sbdOut.pBuffers   = &sbOut;
  sbOut.cbBuffer    = *pcbOut;
  sbOut.BufferType  = SECBUFFER_TOKEN;
  sbOut.pvBuffer    = pOut;
  // Prepare input buffer
  if (pAS->fInitialized)  
  {
    sbdIn.ulVersion = 0;
    sbdIn.cBuffers = 1;
    sbdIn.pBuffers = &sbIn;
    sbIn.cbBuffer = cbIn;
    sbIn.BufferType = SECBUFFER_TOKEN;
    sbIn.pvBuffer = pIn;
  }
  ss = _InitializeSecurityContext(&pAS->hcred,pAS->fInitialized?&pAS->hctxt:NULL,
    NULL,0,0,SECURITY_NATIVE_DREP,pAS->fInitialized?&sbdIn:NULL,0,&pAS->hctxt,
    &sbdOut,&fContextAttr,&tsExpiry);
  if (ss < 0)  
  {
    DBGTrace1("InitializeSecurityContext failed: %s",GetErrorNameStatic(ss));
    return FALSE;
  }
  pAS->fHaveCtxtHandle = TRUE;
  // If necessary, complete token
  if (ss == SEC_I_COMPLETE_NEEDED || ss == SEC_I_COMPLETE_AND_CONTINUE) 
  {
    if (!_CompleteAuthToken) 
    {
      DBGTrace("CompleteAuthToken not linked!");
      return FALSE;
    }
    ss = _CompleteAuthToken(&pAS->hctxt,&sbdOut);
    if (ss < 0)  
    {
      DBGTrace1("CompleteAuthToken failed: %s",GetErrorNameStatic(ss));
      return FALSE;
    }
  }
  *pcbOut = sbOut.cbBuffer;
  if (!pAS->fInitialized)
    pAS->fInitialized = TRUE;
  *pfDone = !(ss == SEC_I_CONTINUE_NEEDED || ss == SEC_I_COMPLETE_AND_CONTINUE );
  return TRUE;
}


/////////////////////////////////////////////////////////////////////////////// 
//
// GenServerContext
//
// Takes an input buffer coming from the client and returns a buffer
// to be sent to the client.  Also returns an indication of whether or
// not the context is complete.
/////////////////////////////////////////////////////////////////////////////// 
BOOL GenServerContext(PAUTH_SEQ pAS,PVOID pIn,DWORD cbIn,PVOID pOut,PDWORD pcbOut,PBOOL pfDone) 
{
  SECURITY_STATUS ss;
  TimeStamp       tsExpiry;
  SecBufferDesc   sbdOut;
  SecBuffer       sbOut;
  SecBufferDesc   sbdIn;
  SecBuffer       sbIn;
  ULONG           fContextAttr;
  if (!pAS->fInitialized)  
  {
    ss = _AcquireCredentialsHandle(NULL,_T("NTLM"),SECPKG_CRED_INBOUND,NULL,
      NULL,NULL,NULL,&pAS->hcred,&tsExpiry);
    if (ss < 0)  
    {
      DBGTrace1("AcquireCredentialsHandle failed: %s",GetErrorNameStatic(ss));
      return FALSE;
    }
    pAS->fHaveCredHandle = TRUE;
  }
  // Prepare output buffer
  sbdOut.ulVersion  = 0;
  sbdOut.cBuffers   = 1;
  sbdOut.pBuffers   = &sbOut;
  sbOut.cbBuffer    = *pcbOut;
  sbOut.BufferType  = SECBUFFER_TOKEN;
  sbOut.pvBuffer    = pOut;
  // Prepare input buffer
  sbdIn.ulVersion   = 0;
  sbdIn.cBuffers    = 1;
  sbdIn.pBuffers    = &sbIn;
  sbIn.cbBuffer     = cbIn;
  sbIn.BufferType   = SECBUFFER_TOKEN;
  sbIn.pvBuffer     = pIn;
  ss = _AcceptSecurityContext(&pAS->hcred,pAS->fInitialized?&pAS->hctxt:NULL,
    &sbdIn,0,SECURITY_NATIVE_DREP,&pAS->hctxt,&sbdOut,&fContextAttr,&tsExpiry);
  if (ss < 0)  
  {
    DBGTrace1("AcceptSecurityContext failed: %s",GetErrorNameStatic(ss));
    return FALSE;
  }
  pAS->fHaveCtxtHandle = TRUE;
  // If necessary, complete token
  if (ss == SEC_I_COMPLETE_NEEDED || ss == SEC_I_COMPLETE_AND_CONTINUE) 
  {
    if (!_CompleteAuthToken) 
    {
      DBGTrace("CompleteAuthToken not linked!");
      return FALSE;
    }
    ss = _CompleteAuthToken(&pAS->hctxt,&sbdOut);
    if (ss < 0)  
    {
      DBGTrace1("CompleteAuthToken failed: %s",GetErrorNameStatic(ss));
      return FALSE;
    }
  }
  *pcbOut = sbOut.cbBuffer;
  if (!pAS->fInitialized)
    pAS->fInitialized = TRUE;
  *pfDone = !((ss == SEC_I_CONTINUE_NEEDED ) || (ss == SEC_I_COMPLETE_AND_CONTINUE));
  return TRUE;
}


/////////////////////////////////////////////////////////////////////////////// 
//
//  SSPLogonUser
//
/////////////////////////////////////////////////////////////////////////////// 
HANDLE SSPLogonUser(LPCTSTR szDomain,LPCTSTR szUser,LPCTSTR szPassword) 
{
  AUTH_SEQ    asServer   = {0};
  AUTH_SEQ    asClient   = {0};
  BOOL        fDone      = FALSE;
  HANDLE      hToken     = NULL;
  DWORD       cbOut      = 0;
  DWORD       cbIn       = 0;
  DWORD       cbMaxToken = 0;
  PVOID       pClientBuf = NULL;
  PVOID       pServerBuf = NULL;
  PSecPkgInfo pSPI       = NULL;
  SEC_WINNT_AUTH_IDENTITY ai;
  __try 
  {
    
    if (!LoadSecurityDll())
    {
      DBGTrace("SSPLogonUser: Exit because LoadSecurityDll failed!");
      return FALSE;
    }
    // Get max token size
    _QuerySecurityPackageInfo(_T("NTLM"),&pSPI);
    cbMaxToken = pSPI->cbMaxToken;
    _FreeContextBuffer(pSPI);
    // Allocate buffers for client and server messages
    pClientBuf = malloc(cbMaxToken);
    memset(pClientBuf,0,cbMaxToken);
    pServerBuf = malloc(cbMaxToken);
    memset(pServerBuf,0,cbMaxToken);
    // Initialize auth identity structure
    ZeroMemory(&ai,sizeof(ai));
#if defined(UNICODE) || defined(_UNICODE)
    ai.Domain = (USHORT*)szDomain;
    ai.DomainLength = lstrlen(szDomain);
    ai.User = (USHORT*)szUser;
    ai.UserLength = lstrlen(szUser);
    ai.Password = (USHORT*)szPassword;
    ai.PasswordLength = lstrlen(szPassword);
    ai.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else      
    ai.Domain = (unsigned char *)szDomain;
    ai.DomainLength = lstrlen(szDomain);
    ai.User = (unsigned char *)szUser;
    ai.UserLength = lstrlen(szUser);
    ai.Password = (unsigned char *)szPassword;
    ai.PasswordLength = lstrlen(szPassword);
    ai.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif
    // Prepare client message (negotiate) .
    cbOut = cbMaxToken;
    if (!GenClientContext(&asClient,&ai,NULL,0,pClientBuf,&cbOut,&fDone))
      __leave;
    // Prepare server message (challenge) .
    cbIn  = cbOut;
    cbOut = cbMaxToken;
    if (!GenServerContext(&asServer,pClientBuf,cbIn,pServerBuf,&cbOut,&fDone))
      __leave;
    // Most likely failure: AcceptServerContext fails with SEC_E_LOGON_DENIED
    // in the case of bad szUser or szPassword.
    // Unexpected Result: Logon will succeed if you pass in a bad szUser and 
    // the guest account is enabled in the specified domain.
    // Prepare client message (authenticate) .
    cbIn  = cbOut;
    cbOut = cbMaxToken;
    if (!GenClientContext(&asClient,&ai,pServerBuf,cbIn,pClientBuf,&cbOut,&fDone))
      __leave;
    // Prepare server message (authentication) .
    cbIn  = cbOut;
    cbOut = cbMaxToken;
    if (!GenServerContext(&asServer,pClientBuf,cbIn,pServerBuf,&cbOut,&fDone))
      __leave;
    _QuerySecurityContextToken(&asServer.hctxt,&hToken);
  } __finally 
  {
    // Clean up resources
    if (asClient.fHaveCtxtHandle)
      _DeleteSecurityContext(&asClient.hctxt);
    if (asClient.fHaveCredHandle)
      _FreeCredentialsHandle(&asClient.hcred);
    if (asServer.fHaveCtxtHandle)
      _DeleteSecurityContext(&asServer.hctxt);
    if (asServer.fHaveCredHandle)
      _FreeCredentialsHandle(&asServer.hcred);
    free(pClientBuf);
    free(pServerBuf);
  }
  return hToken;
}
