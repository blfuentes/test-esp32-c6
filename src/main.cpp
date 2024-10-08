#include <driver/gpio.h>
#include <rom/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>
#include <esp_err.h>
#include <esp_log.h>

extern "C" void app_main();

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO GPIO_NUM_8
#define CONFIG_BLINK_LED_STRIP 1
#define CONFIG_BLINK_LED_STRIP_BACKEND_SPI 1

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

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
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST
    };
    spi_config.flags.with_dma = true;
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#endif

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
    configure_led();

    xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 5, &Task1, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 10, &Task2, 0);
    while (true) {
        printf("Hello, world!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}