#include "I2C.hpp"

namespace I2C {
uint8_t data_wr[DATA_LENGTH];
uint8_t data_rd[DATA_LENGTH];
esp_err_t err_check;

uint8_t joystickX = 127;
uint8_t joystickY = 127;
bool joystickButtonHold_low = false;
bool joystickButtonHold_up = false;
bool joystickError = false;
void taskFunction(void *pvParameter) {
  while (1)

  {
    data_wr[0] = 0x00;
    err_check = i2c_master_write_slave(I2C_MASTER_NUM, data_wr, (size_t)1);
    vTaskDelay((1 * (1)) / portTICK_RATE_MS);
    i2c_master_read_slave(I2C_MASTER_NUM, data_rd, (size_t)6);

    if (err_check != 0 ||
        (data_rd[2] == 255 && data_rd[3] == 255 && data_rd[4] == 255)) {
      printf("error code: %d. Re-initializing.\n", err_check);
      printf("[%d] [%d] [%d]", data_rd[2], data_rd[3], data_rd[4]);
      joystick_init();
      joystickError = true;

    } else {
      joystickError = false;
      joystickX = data_rd[0];
      joystickY = data_rd[1];

      joystickButtonHold_low = ((data_rd[5] & joystickButtonLowBit) == 0);
      joystickButtonHold_up = ((data_rd[5] & joystickButtonUpBit) == 0);

      //						printf("X=%d; ",
      //joystickX); 						printf("Y=%d; ", joystickY); 						if (joystickButtonHold_low)
      //						{
      //							printf("Button_Low
      //pressed; ");
      //						}
      //						else
      //						{
      //							printf("Button_Low
      //released; ");
      //						}
      //
      //						if
      //(joystickButtonHold_up)
      //						{
      //							printf("Button_Up
      //pressed; ");
      //						}
      //						else
      //						{
      //							printf("Button_Up
      //released; ");
      //						}
      //
      //
      //						printf("\n");
    }

    vTaskDelay((20 * (1)) / portTICK_RATE_MS);
  }
}
void start_I2C()

{
  ESP_ERROR_CHECK(i2c_master_init());
  printf("I2C initialized\n");
  vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (1)) / portTICK_RATE_MS);

  joystick_init();
  //	vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (1)) / portTICK_RATE_MS);
  //	data_wr[0]=0x00;
  //	i2c_master_write_slave(I2C_MASTER_NUM,data_wr,(size_t)1);
  //	printf("start joystick convertion.\n");
  //	vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (1)) / portTICK_RATE_MS);
  //	printf("start reading joystick.\n");
  xTaskCreate(&taskFunction,    // function the task runs
              "taskFunction_5", // name of the task (should be short)
              2048,             // stack size for the task
              NULL,             // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
}

void joystick_init() {

  data_wr[0] = 0xF0;
  data_wr[1] = 0x55;
  //		printf("start joystick.\n");
  i2c_master_write_slave(I2C_MASTER_NUM, data_wr, (size_t)2);
  //
  vTaskDelay((1 * (1)) / portTICK_RATE_MS);
  data_wr[0] = 0xFB;
  data_wr[1] = 0x00;
  printf("start joystick.\n");
  i2c_master_write_slave(I2C_MASTER_NUM, data_wr, (size_t)2);
  vTaskDelay((1 * (1)) / portTICK_RATE_MS);
}
/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read
 * them out.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte +
 * nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd,
                                size_t size) {
  if (size == 0) {
    return ESP_OK;
  }
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (ESP_SLAVE_READ_ADDR << 1) | READ_BIT,
                        ACK_CHECK_EN);
  if (size > 1) {
    i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
  }
  i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr,
                                 size_t size) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (ESP_SLAVE_WRITE_ADDR << 1) | WRITE_BIT,
                        ACK_CHECK_EN);
  i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init() {
  i2c_port_t i2c_master_port = I2C_MASTER_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  return i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}

} // namespace I2C
