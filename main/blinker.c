#include "esp_timer.h"
#include "esp_log.h"
#include "light_driver.h"


static const char *TAG = "BLINKER_SHIFT_SENSOR";

static esp_timer_handle_t blink_timer;

void toggle_light_cb(void* arg) {
    static int toggle_count = 0;
    toggle_count++;
    light_driver_set_power(toggle_count % 2); // Alterner l’état
    
    if (toggle_count >= 4) { // Stop après 2 clignotements
        ESP_LOGI(TAG, "light stop %d", toggle_count);
        esp_timer_stop(blink_timer);
        toggle_count = 0;
        light_driver_set_power(false);
    }
    else{ESP_LOGI(TAG, "light %d", toggle_count);}
}

void start_blinking() {
    esp_timer_create_args_t timer_args = {
        .callback = &toggle_light_cb,
        .arg = &blink_timer,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "blink_timer"
    };
    esp_timer_create(&timer_args, &blink_timer);
    esp_timer_start_periodic(blink_timer, 500000); // 500 ms
}