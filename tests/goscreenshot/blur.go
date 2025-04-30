package main

import (
	"image"
	"image/color"
)

func BlurSimilar(img *image.RGBA) *image.RGBA {
	bounds := img.Bounds()
	w, h := bounds.Max.X, bounds.Max.Y
	blurred := image.NewRGBA(bounds)

	for y := 1; y < h-1; y++ {
		for x := 1; x < w-1; x++ {
			var rSum, gSum, bSum, aSum uint32

			for ky := -1; ky <= 1; ky++ {
				for kx := -1; kx <= 1; kx++ {
					px := img.RGBAAt(x+kx, y+ky)
					r := uint32(px.R)
					g := uint32(px.G)
					b := uint32(px.B)
					a := uint32(px.A)

					weight := uint32(1)
					if kx != 0 {
						weight *= 2
					}
					if ky != 0 {
						weight *= 2
					}
					if kx == 0 && ky == 0 {
						weight = 4
					}

					rSum += r * weight
					gSum += g * weight
					bSum += b * weight
					aSum += a * weight
				}
			}

			blurred.SetRGBA(x, y, color.RGBA{
				R: uint8(rSum >> 5), // Divide by 32
				G: uint8(gSum >> 5),
				B: uint8(bSum >> 5),
				A: uint8(aSum >> 5),
			})
		}
	}

	return blurred
}