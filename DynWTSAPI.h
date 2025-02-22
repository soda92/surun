#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define WTS_CURRENT_SERVER ((HANDLE)NULL)
#define WTS_CURRENT_SERVER_HANDLE ((HANDLE)NULL)
#define WTS_CURRENT_SERVER_NAME (NULL)

typedef struct _WTS_PROCESS_INFO {
  DWORD SessionId;
  DWORD ProcessId;
  LPWSTR pProcessName;
  PSID pUserSid;
} WTS_PROCESS_INFO, *PWTS_PROCESS_INFO;

BOOL WINAPI WTSEnumerateProcesses(HANDLE hServer, DWORD Reserved, DWORD Version,
                                  PWTS_PROCESS_INFO *ppProcessInfo,
                                  DWORD *pCount);

#define WTS_CURRENT_SESSION ((DWORD) - 1)

typedef enum _WTS_CONNECTSTATE_CLASS {
  WTSActive,
  WTSConnected,
  WTSConnectQuery,
  WTSShadow,
  WTSDisconnected,
  WTSIdle,
  WTSListen,
  WTSReset,
  WTSDown,
  WTSInit,
} WTS_CONNECTSTATE_CLASS;

typedef struct _WTS_SESSION_INFO {
  DWORD SessionId;
  LPWSTR pWinStationName;
  WTS_CONNECTSTATE_CLASS State;
} WTS_SESSION_INFO, *PWTS_SESSION_INFO;

BOOL WINAPI WTSEnumerateSessions(HANDLE hServer, DWORD Reserved, DWORD Version,
                                 PWTS_SESSION_INFO *ppSessionInfo,
                                 DWORD *pCount);

typedef enum _WTS_INFO_CLASS {
  WTSInitialProgram,
  WTSApplicationName,
  WTSWorkingDirectory,
  WTSOEMId,
  WTSSessionId,
  WTSUserName,
  WTSWinStationName,
  WTSDomainName,
  WTSConnectState,
  WTSClientBuildNumber,
  WTSClientName,
  WTSClientDirectory,
  WTSClientProductId,
  WTSClientHardwareId,
  WTSClientAddress,
  WTSClientDisplay,
  WTSClientProtocolType,
} WTS_INFO_CLASS;

BOOL WINAPI WTSQuerySessionInformation(HANDLE hServer, DWORD SessionId,
                                       WTS_INFO_CLASS WTSInfoClass,
                                       LPWSTR *ppBuffer, DWORD *pBytesReturned);

BOOL WINAPI WTSLogoffSession(HANDLE hServer, DWORD SessionId, BOOL bWait);

VOID WINAPI WTSFreeMemory(PVOID pMemory);

BOOL WINAPI WTSQueryUserToken(ULONG SessionId, PHANDLE phToken);

#ifdef __cplusplus
}
#endif
