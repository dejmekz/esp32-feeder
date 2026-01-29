#include "utils.h"
#include <esp32-hal-log.h>
#include <WiFi.h>
#include "mqtt_handler.h"

// short deep sleep triggers a soft
// reboot and thus keeps RTC data
void restartSystem() {
    log_i("Restarting system... [%d]", millis());

    // Clean disconnect from MQTT and WiFi
    mqtt_disconnect();
    WiFi.disconnect(true);
    delay(100);

    esp_sleep_enable_timer_wakeup(1000000);
    esp_deep_sleep_start();
}

// reset system (RTC data will be lost)
void resetSystem() {
    log_i("System reset... [%d]", millis());

    // Clean disconnect from MQTT and WiFi
    mqtt_disconnect();
    WiFi.disconnect(true);
    delay(100);

    ESP.restart();
}
