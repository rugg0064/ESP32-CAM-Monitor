# ESP32-CAM-Monitor
A simple stack for esp32-cam based "security cameras"

# Camera Hardware

Currently using an ESP32-CAM with an OV2640 camera. Power supplies are cheap aliexpress 5v power supplies spliced onto the board.

# Camera Software

wifi-http-cam-server is an ESP-IDF program for the hardware that will expose an http endpoint `/capture` to download a jpeg image from the camera.

# Recording Software

TBD details. This will be a Go based program that periodically pulls captures off the cameras and saves them to disk.