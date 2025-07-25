#ifndef CONFIG_H
#define CONFIG_H

// WiFi configuration
#define WIFI_SSID     "YourSSID" // Replace with your WiFi SSID
#define WIFI_PASSWORD "YourPassword" // Replace with your WiFi password

// MQTT configuration
#define MQTT_SERVER   "YourMQTTServer" // Replace with your MQTT server address
#define MQTT_PORT     1883 // Default MQTT port
#define MQTT_USER     "mqttuser" // Replace with your MQTT username
#define MQTT_PASSWORD "mqttpassword" // Replace with your MQTT password

#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
#define DHT22_PIN 18 // DHT22 sensor pin

#define LED_RED_PIN 1
#define LED_BLUE_PIN 2

#define BUTTON_PIN 3 // Button pin for feeding

#define MOTOR_PIN_1 4 // Motor control pin 1
#define MOTOR_PIN_2 5 // Motor control pin 2

#define NTP_SERVER "cz.pool.ntp.org" // NTP server for time synchronization

#define FIRMWARE_VERSION "0.0.1"
#define FOTA_MANIFEST_URL "http://<your_server>/fota/feeder/fota.json"

#endif