---
date: '2019-10-04T19:01:44+08:00'
title: 'GPEdit keys and values'
---

HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\SeCEdit\Reg Values

*Richtlinien f�r Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Sicherheitsoptionen

  Systemobjekte: Standardbesitzer f�r Objekte, die von Mitgliedern der Administratorengruppe erstellt werden	
  ->Administratorengruppe
  ->>in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa
     nodefaultadminowner,DWORD: 1=Objektersteller;  0=Administratorengruppe; default=1

*Richtlinien f�r Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Sicherheitsoptionen

  Konten: Lokale Kontenverwendung von leeren Kennw�rtern auf Konsolenanmeldung beschr�nken
  ->>in HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Lsa
    limitblankpassworduse,DWORD: 1=leere Kennw�rter nicht zul�ssig;  0=leere Kennw�rter zul�ssig; default=1

*Richtlinien f�r Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Zuweisen von Benutzerrechten

  �ndern der Systemzeit:  Administratoren,Hauptbenutzer
  ->"SuRunners" hinzuf�gen
  ->>ntrights.exe -u SuRunners +r SeSystemtimePrivilege

*Richtlinien f�r Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Zuweisen von Benutzerrechten

  Laden und entfernen von Ger�tetreibern:  Administratoren...
  ->"SuRunners" hinzuf�gen
  ->>???

*Richtlinien f�r Lokaler Computer
  Computerkonfiguration
    Administrative Vorlagen
      Windows Componenten
        Windows Update

  Nicht-Administratoren gestatten, Updatebenachrichtigungen zu erhalten	
  ->Aktiviert
  ->>HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU
    ElevateNonAdmins,DWORD: 1=Aktiviert, 0=Deaktiviert

  Keinen automatischen Neustart f�r geplante Installation durchf�hren
  ->Aktiviert
  ->>HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU
    NoAutoRebootWithLoggedOnUsers,DWORD: 1=Aktiviert, 0=Deaktiviert


Energieoptionen f�r nicht-Admins
In HKLM\Software\Microsoft\Windows\CurrentVersion\Controls Folder\PowerCfg
  SuRunners Schreibrechte geben
