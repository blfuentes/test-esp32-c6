#include <driver/gpio.h>
#include <rom/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>
#include <esp_err.h>
#include <esp_log.h>

extern "C" void app_main();

static const char *TAG = "example";

#define BLINK_GPIO GPIO_NUM_8

static uint8_t s_led_state = 0;

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST
    };
    spi_config.flags.with_dma = true;
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

// Define task handle
TaskHandle_t Task1;
TaskHandle_t Task2;

// Task function for LED blink
void Task1code(void* parameter) {
    while (1) {
        printf("Hello from the LED task!\n");
        blink_led();
        s_led_state = !s_led_state;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2code(void* parameter) {
    while(1) {
        printf("Hello from the task!\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main() {
    // configure led
    configure_led();

    xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 5, &Task1, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 10, &Task2, 0);
    while (true) {
        printf("Hello, world!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}