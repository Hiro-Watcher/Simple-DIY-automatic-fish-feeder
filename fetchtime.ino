#include <WiFi.h>
#include <time.h>

// Replace with your local Wi-Fi credentials
const char* ssid     = "Khong Co Ten";
const char* password = "phamhuong1973";

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;       // Set your timezone offset in seconds (e.g., 7 * 3600 for UTC+7)
const int   daylightOffset_sec = 0;  // Daylight saving offset if applicable

void setup() {
  // Start Serial monitor for debugging
  Serial.begin(115200);

  // Initialize Hardware Serial 2 on ESP32 for board-to-board communication
  // TX2 = GPIO 17, RX2 = GPIO 16 (Connect ESP32 TX2 to Bluepill RX)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Connect to Wi-Fi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED!");

  // Init and get the atomic time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("Synchronizing time with atomic clock...");
  struct tm timeinfo;
  while(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time, retrying...");
    delay(1000);
  }
  Serial.println("Time synchronized successfully!");
}

void loop() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    // Format time into a clean string (e.g., "2026-10-27 14:30:05")
    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Send the atomic timestamp to the Bluepill via UART wire
    Serial2.println(timeBuffer);
    Serial.print("Sent to Bluepill: ");
    Serial.println(timeBuffer);
  }

  // Send update to Bluepill once every hour
  delay(30000); 
}