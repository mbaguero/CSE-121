#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "SHTC3";

#define I2C_MASTER_SCL_IO		8
#define I2C_MASTER_SDA_IO		10
#define I2C_MASTER_NUM			I2C_NUM_0
#define I2C_MASTER_FREQ_HZ		400000
#define I2C_MASTER_TX_BUF_DISABLE  	0
#define I2C_MASTER_RX_BUF_DISABLE	0
#define I2C_MASTER_TIMEOUT_MS		1000

#define SHTC3_SENSOR_ADDR		0x70
#define SHTC3_WAKEUP_CMD		0x3517
#define SHTC3_SLEEP_CMD			0xB098
#define SHTC3_MEASURE_CMD		0x7866



static uint8_t shtc3_crc8(const uint8_t *data, int len)
{
	uint8_t crc = 0xFF;
	for (int i = 0; i < len; i++){
		crc ^= data[i];
		for (int j = 0; j < 8; j++)
			crc =(crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
	}
	return crc;
}

static esp_err_t shtc3_read_cmd(i2c_master_dev_handle_t dev_handle, uint16_t *temp_raw, uint16_t *humd_raw)
{
	esp_err_t err;
	uint8_t read_buf[6] = {0};

	err = i2c_master_receive(
		dev_handle,
		read_buf,
		sizeof(read_buf),
		pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)
		);	
	
	if (err !=  ESP_OK) return err;

	if (shtc3_crc8(read_buf, 2) != read_buf [2] || shtc3_crc8(read_buf + 3, 2) != read_buf[5]) {
		ESP_LOGE(TAG, "CRC check failed");
		return ESP_FAIL;
	}


	*temp_raw = (read_buf[0] << 8) | read_buf[1];
	*humd_raw = (read_buf[3] << 8) | read_buf[4];

	return  ESP_OK;

}

static esp_err_t shtc3_write_cmd(i2c_master_dev_handle_t dev_handle, uint16_t cmd)
{
	uint8_t write_buf[2] = {(cmd&0xff00>>8),(cmd&0xff)};

	return i2c_master_transmit(
		dev_handle,
		write_buf,
		sizeof(write_buf),
		pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)
	);

}


static float shtc3_raw_to_celsius(uint16_t raw)
{
	return -45 + 175 * ( (float)raw / (65535.0));
}

static float shtc3_raw_to_humd(uint16_t raw)
{
	return 100 * ( (float)raw / 65535.0);
}

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
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

	i2c_device_config_t dev_config = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = SHTC3_SENSOR_ADDR,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};

	ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

void app_main(void)
{
	i2c_master_bus_handle_t bus_handle;
	i2c_master_dev_handle_t dev_handle;
	i2c_master_init(&bus_handle, &dev_handle);
	ESP_LOGI(TAG, "I2C init PASS");
	
	uint16_t temp_raw, humd_raw;

	while(1) {
		shtc3_write_cmd(dev_handle, SHTC3_WAKEUP_CMD);
		
		vTaskDelay(pdMS_TO_TICKS(1));

		shtc3_write_cmd(dev_handle, SHTC3_MEASURE_CMD);

		vTaskDelay(pdMS_TO_TICKS(15));

		if (shtc3_read_cmd(dev_handle, &temp_raw, &humd_raw) == ESP_OK){
			float temp_C = shtc3_raw_to_celsius(temp_raw);
			float humd = shtc3_raw_to_humd(humd_raw);
			float temp_F = temp_C * 9 / 5 + 32;

			ESP_LOGI(TAG, "Temperature is %.0fF with a %.0f%% humdity", temp_F, humd);
		}

		shtc3_write_cmd(dev_handle, SHTC3_SLEEP_CMD);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

