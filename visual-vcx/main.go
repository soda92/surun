package main

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

func main() {
	myApp := app.New()
	myWindow := myApp.NewWindow("TabContainer Widget")
	tabs := container.NewAppTabs()

	p := NewParser("../SuRun.sln")
	solution := p.ParseSolution()
	for _, proj := range solution.projects {
		item := container.NewTabItem(proj.name, widget.NewLabel("content"))
		tabs.Append(item)
	}

	tabs.SetTabLocation(container.TabLocationLeading)

	myWindow.SetContent(tabs)
	myWindow.Resize(fyne.NewSize(800, 400))
	myWindow.ShowAndRun()
}
