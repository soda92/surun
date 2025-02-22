cmake_minimum_required(VERSION 3.23)
project("aaa")

set(SOURCES "DBGTrace.cpp"
"DynWTSAPI.cpp"
"Helpers.cpp"
"IsAdmin.cpp"
"LogonDlg.cpp"
"LSALogon.cpp"
"lsa_laar.cpp"
"main.cpp"
"ReqAdmin.cpp"
"Service.cpp"
"Setup.cpp"
"sspi_auth.cpp"
"TrayMsgWnd.cpp"
"TrayShowAdmin.cpp"
"UserGroups.cpp"
"WatchDog.cpp"
"WinStaDesk.cpp")

add_executable(aaa ${SOURCES})
