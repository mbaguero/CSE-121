#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO            8
#define I2C_MASTER_SDA_IO            10
#define I2C_MASTER_NUM               I2C_NUM_0
#define I2C_MASTER_FREQ_HZ           100000
#define I2C_MASTER_TIMEOUT_MS        1000

static const char *TAG = "I2C_SCAN";

void i2c_master_init(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_init(&bus_handle);

    ESP_LOGI(TAG, "Starting I2C scan...");

    for (int addr = 1; addr < 127; addr++) {
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };

        i2c_master_dev_handle_t dev_handle = NULL;
        esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);

        if (err == ESP_OK) {
            uint8_t dummy = 0;
            err = i2c_master_transmit(dev_handle, &dummy, 1, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));

            if (err == ESP_OK) {
                ESP_LOGI(TAG, " Found I2C device at 0x%02X", addr);
            }

            i2c_master_bus_rm_device(dev_handle);
        }
    }

    ESP_LOGI(TAG, "I2C scan complete");
}
