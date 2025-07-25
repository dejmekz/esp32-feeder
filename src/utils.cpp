#include "utils.h"
#include <esp32-hal-log.h>

// short deep sleep triggers a soft 
// reboot and thus keeps RTC data
void restartSystem() {
    log_d(": Restarting system... [%d]", millis());
    delay(1000);
    //Serial.flush();
    esp_sleep_enable_timer_wakeup(1000000);
    esp_deep_sleep_start();
}


// reset system (RTC data will be lost)
void resetSystem() {
    log_d(": System reset... [%d]", millis());
    delay(500);
    ESP.restart();
}
