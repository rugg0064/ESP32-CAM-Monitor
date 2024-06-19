package main

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"github.com/joho/godotenv"
	"go.uber.org/zap"
)

type Server struct {
	Server_Url  string `json:"URL"`
	Server_Name string `json:"NAME"`
}

type Config struct {
	CaptureDirectory string   `json:"CAPTURE_DIRECTORY"`
	CaptureHosts     []Server `json:"CAPTURE_HOSTS"`
	IntervalMS       int64    `json:"INTERVAL_MS"`
}

var (
	logger *zap.Logger
	config *Config
	ticker *time.Ticker // Ticker can be controlled to adjust the capture rate
)

func getEnvVar(key string) string {
	err := godotenv.Load("./.env")
	if err != nil {
		panic("godotenv.Load failed")
	}
	value := os.Getenv(key)
	return value
}

func main() {
	var err error

	// Start logger
	logger, err = zap.NewProduction()
	if err != nil {
		panic("Failed to initialize logger")
	}
	defer logger.Sync() // flushes buffer, if any
	logger.Info("Logger started")

	// Load config
	loadedConfig, err := loadConfig()
	if err != nil {
		logger.Fatal("Failed to load config file", zap.Error(err))
	}
	config = loadedConfig

	// Create necessary directories
	err = createDirectories()
	if err != nil {
		logger.Fatal("Failed to create directories", zap.Error(err))
	}
	logger.Info("Confirmed directories")

	startTimer()
	// Wait forever while timer runs
	for {
		time.Sleep(time.Minute)
	}
}

func loadConfig() (*Config, error) {
	filename := getEnvVar("CONFIG_PATH")
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	byteValue, err := io.ReadAll(file)
	if err != nil {
		return nil, err
	}

	var config Config
	if err := json.Unmarshal(byteValue, &config); err != nil {
		return nil, err
	}

	return &config, nil
}

func createDirectories() error {
	var err error
	err = os.MkdirAll(config.CaptureDirectory, os.ModePerm)
	if err != nil {
		logger.Error("Failed to create capture directory", zap.Error(err))
		return err
	}
	for _, host := range config.CaptureHosts {
		err = os.MkdirAll(config.CaptureDirectory+"/"+host.Server_Name, os.ModePerm)
		if err != nil {
			logger.Error("Failed to create capture directory", zap.Error(err))
			return err
		}
	}
	return nil
}

func startTimer() {
	go func() {
		ticker = time.NewTicker(time.Millisecond * time.Duration(config.IntervalMS))
		for range ticker.C {
			capture()
		}
	}()
}

func capture() {
	timestamp := time.Now().Format("2006-01-02T15-04-05")
	fmt.Printf("Saving for timestamp %s\n", timestamp)

	for _, host := range config.CaptureHosts {
		hLog := logger.With(zap.Any("host", host))
		filename := filepath.Join(config.CaptureDirectory, host.Server_Name, fmt.Sprintf("%s.jpg", timestamp))

		// Download the image
		if err := downloadImage(host.Server_Url, filename); err != nil {
			hLog.Error("Failed to download image", zap.Error(err))
		} else {
			hLog.Info("Image saved", zap.String("filename", filename))
		}
	}
}

func downloadImage(url, filepath string) error {
	// Make the HTTP request
	resp, err := http.Get(url)
	if err != nil {
		return fmt.Errorf("failed to make HTTP request: %w", err)
	}
	defer resp.Body.Close()

	// Check if the response status is OK
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("bad status: %s", resp.Status)
	}

	// Create the file
	out, err := os.Create(filepath)
	if err != nil {
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer out.Close()

	// Copy the response body to the file
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		return fmt.Errorf("failed to write to file: %w", err)
	}

	return nil
}
