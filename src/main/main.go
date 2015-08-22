package main

import (
    "image"
    "log"
    "os"
    
    _ "image/png"
)

func main() {
    reader, err := os.Open("data/left.png")
    if err != nil {
        log.Fatal(err)
    }
    
    img, _, err := image.Decode(reader)
    if err != nil {
        log.Fatal(err)
    }
    
    if err := reader.Close(); err != nil {
        log.Fatal(err)
    }
    
    bound := img.Bounds()
    print(bound.Min.X, " ", bound.Max.X, "\n")
    print(bound.Min.Y, " ", bound.Max.Y, "\n")
    r, g, b, _ := img.At(0, 0).RGBA()
    print(r, " ", g, " ", b, "\n")
}