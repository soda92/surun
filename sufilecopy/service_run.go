// Copyright 2012 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

//go:build windows

package main

import (
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	"golang.org/x/sys/windows/svc"
	"golang.org/x/sys/windows/svc/debug"
	"golang.org/x/sys/windows/svc/eventlog"
)

var elog debug.Log

type exampleService struct{}

func (m *exampleService) Execute(args []string, r <-chan svc.ChangeRequest, changes chan<- svc.Status) (ssec bool, errno uint32) {
	const cmdsAccepted = svc.AcceptStop | svc.AcceptShutdown | svc.AcceptPauseAndContinue
	changes <- svc.Status{State: svc.StartPending}
	RunServer()
	changes <- svc.Status{State: svc.Running, Accepts: cmdsAccepted}
loop:
	for {
		c := <-r
		switch c.Cmd {
		case svc.Interrogate:
			changes <- c.CurrentStatus
			// Testing deadlock from https://code.google.com/p/winsvc/issues/detail?id=4
			time.Sleep(100 * time.Millisecond)
			changes <- c.CurrentStatus
		case svc.Stop, svc.Shutdown:
			// golang.org/x/sys/windows/svc.TestExample is verifying this output.
			testOutput := strings.Join(args, "-")
			testOutput += fmt.Sprintf("-%d", c.Context)
			elog.Info(1, testOutput)
			break loop
		case svc.Pause:
			changes <- svc.Status{State: svc.Paused, Accepts: cmdsAccepted}
		case svc.Continue:
			changes <- svc.Status{State: svc.Running, Accepts: cmdsAccepted}
		default:
			elog.Error(1, fmt.Sprintf("unexpected control request #%d", c))
		}

	}
	changes <- svc.Status{State: svc.StopPending}
	return
}

func runService(name string, isDebug bool) {
	var err error
	if isDebug {
		elog = debug.New(name)
	} else {
		elog, err = eventlog.Open(name)
		if err != nil {
			return
		}
	}
	defer elog.Close()

	elog.Info(1, fmt.Sprintf("starting %s service", name))
	run := svc.Run
	if isDebug {
		run = debug.Run
	}
	err = run(name, &exampleService{})
	if err != nil {
		elog.Error(1, fmt.Sprintf("%s service failed: %v", name, err))
		return
	}
	elog.Info(1, fmt.Sprintf("%s service stopped", name))
}

func Deploy() {
	p, _ := os.Executable()
	running_name, err := determineRunningService()
	if err != nil {
		log.Fatal(err)
	}
	target_name := GetTargetName(running_name)
	err = remoteFileCopy(p, target_name)
	if err != nil {
		log.Fatal(err)
	}
	err = remoteSwitchService()
	if err != nil {
		log.Fatal(err)
	}
}

func InitServices() {
	RemoveServices()
	p, _ := os.Executable()
	dest_a := "C:/sufilecopy_a.exe"
	dest_b := "C:/sufilecopy_b.exe"
	_, err := copy(p, dest_a)
	if err != nil {
		log.Printf("could not copy %s to %s", p, dest_a)
	}
	_, err = copy(p, dest_b)
	if err != nil {
		log.Printf("could not copy %s to %s", p, dest_b)
	}
	err = installService(NAME_A, dest_a, "Super User File Copy (slot A)")
	if err != nil {
		log.Fatal(err)
	}
	err = installService(NAME_B, dest_b, "Super User File Copy (slot B)")
	if err != nil {
		log.Fatal(err)
	}

	err = startService(NAME_A)
	if err != nil {
		log.Fatal(err)
	}
	log.Print("init completed")
}

func RemoveServices() {
	err := controlService(NAME_A, svc.Stop, svc.Stopped)
	if err != nil {
		log.Printf("cound not stop %s, error:%s", NAME_A, err)
	}
	err = controlService(NAME_B, svc.Stop, svc.Stopped)
	if err != nil {
		log.Printf("cound not stop %s, error: %s", NAME_B, err)
	}
	err = removeService(NAME_A)
	if err != nil {
		log.Printf("cound not remove service %s, error %s", NAME_A, err)
	}
	err = removeService(NAME_B)
	if err != nil {
		log.Printf("cound not remove service %s, error %s", NAME_B, err)
	}
	log.Print("remove completed")
}
