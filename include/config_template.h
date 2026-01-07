#ifndef CONFIG_H
#define CONFIG_H

// WiFi configuration
#define WIFI_SSID "YourSSID"       // Replace with your WiFi SSID
#define WIFI_PASSWORD "YourPassword" // Replace with your WiFi password

// MQTT configuration
#define MQTT_SERVER "YourMQTTServer"   // Replace with your MQTT server address
#define MQTT_PORT 1883                 // Default MQTT port
#define MQTT_USER "mqttuser"           // Replace with your MQTT username
#define MQTT_PASSWORD "mqttpassword"   // Replace with your MQTT password

// Sensor configuration
#define DHTTYPE DHT22  // DHT 22 (AM2302), AM2321
#define DHT22_PIN 18   // DHT22 sensor pin

// LED pins
#define LED_RED_PIN 1
#define LED_BLUE_PIN 2

// Button pin
#define BUTTON_PIN 3 // Button pin for feeding

// Motor pins
#define MOTOR_PIN_1 4 // Motor control pin 1
#define MOTOR_PIN_2 5 // Motor control pin 2

// Servo pin
#define SERVO_PIN 6

// NTP configuration
#define NTP_SERVER "cz.pool.ntp.org" // NTP server for time synchronization

// FOTA (Firmware Over The Air) configuration
// Uncomment and configure based on your motor type:

#ifdef USING_MOTOR
#define FIRMWARE_VERSION "0.0.8"
#define FOTA_MANIFEST_URL "http://your-server.local/fota/feeder-motor/fota.json"
#define FOTA_FIRMWARE_TYPE "esp32-feeder-motor"
#else
#define FIRMWARE_VERSION "0.0.11"
#define FOTA_MANIFEST_URL "http://your-server.local/fota/feeder-servo/fota.json"
#define FOTA_FIRMWARE_TYPE "esp32-feeder-servo"
#endif

// Feeding schedule configuration
#define FEEDING_DURATION 10000 // Default feeding duration in milliseconds

#define FEEDING_HOUR_01 6
#define FEEDING_MINUTE_01 30
#define FEEDING_PORTIONS_01 3 // Default number of feeding portions

#define FEEDING_HOUR_02 15
#define FEEDING_MINUTE_02 0
#define FEEDING_PORTIONS_02 4 // Default number of feeding portions

#endif