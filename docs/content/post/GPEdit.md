---
date: '2019-10-04T19:01:44+08:00'
title: 'GPEdit keys and values'
---

HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\SeCEdit\Reg Values

*Richtlinien für Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Sicherheitsoptionen

  Systemobjekte: Standardbesitzer für Objekte, die von Mitgliedern der Administratorengruppe erstellt werden	
  ->Administratorengruppe
  ->>in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Lsa
     nodefaultadminowner,DWORD: 1=Objektersteller;  0=Administratorengruppe; default=1

*Richtlinien für Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Sicherheitsoptionen

  Konten: Lokale Kontenverwendung von leeren Kennwörtern auf Konsolenanmeldung beschränken
  ->>in HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Lsa
    limitblankpassworduse,DWORD: 1=leere Kennwörter nicht zulässig;  0=leere Kennwörter zulässig; default=1

*Richtlinien für Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Zuweisen von Benutzerrechten

  Ändern der Systemzeit:  Administratoren,Hauptbenutzer
  ->"SuRunners" hinzufügen
  ->>ntrights.exe -u SuRunners +r SeSystemtimePrivilege

*Richtlinien für Lokaler Computer
  Computerkonfiguration
    Windows-Einstellungen
      Sicherheitseinstellungen
        Lokale Richtlinien
          Zuweisen von Benutzerrechten

  Laden und entfernen von Gerätetreibern:  Administratoren...
  ->"SuRunners" hinzufügen
  ->>???

*Richtlinien für Lokaler Computer
  Computerkonfiguration
    Administrative Vorlagen
      Windows Componenten
        Windows Update

  Nicht-Administratoren gestatten, Updatebenachrichtigungen zu erhalten	
  ->Aktiviert
  ->>HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU
    ElevateNonAdmins,DWORD: 1=Aktiviert, 0=Deaktiviert

  Keinen automatischen Neustart für geplante Installation durchführen
  ->Aktiviert
  ->>HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU
    NoAutoRebootWithLoggedOnUsers,DWORD: 1=Aktiviert, 0=Deaktiviert


Energieoptionen für nicht-Admins
In HKLM\Software\Microsoft\Windows\CurrentVersion\Controls Folder\PowerCfg
  SuRunners Schreibrechte geben
