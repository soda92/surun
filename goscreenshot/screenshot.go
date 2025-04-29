package main

import (
	"fmt"
	"image/png"
	"log"
	"os"
	"time"

	"github.com/kbinani/screenshot"
)

func main() {
	n := screenshot.NumActiveDisplays()

	for i := 0; i < n; i++ {
		bounds := screenshot.GetDisplayBounds(i)

		img, err := screenshot.CaptureRect(bounds)
		if err != nil {
			log.Fatal(err)
		}

		timestamp := time.Now().Format("20060102_150405")
		filename := fmt.Sprintf("screenshot_%d_%s.png", i, timestamp)

		file, err := os.Create(filename)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()

		err = png.Encode(file, img)
		if err != nil {
			log.Fatal(err)
		}

		fmt.Printf("Screenshot %d saved to %s\n", i, filename)
	}
}