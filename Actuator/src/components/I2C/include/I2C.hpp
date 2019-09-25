
#ifndef __I2C__INCLUDE_GUARD
#define __I2C__INCLUDE_GUARD

#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "driver/gpio.h"



//#define CONFIG_I2C_MASTER_SCL 19
//#define CONFIG_I2C_MASTER_SDA 18
//#define CONFIG_I2C_MASTER_PORT_NUM 1
#define CONFIG_I2C_MASTER_FREQUENCY 100000
//#define CONFIG_I2C_SLAVE_SCL 26
//#define CONFIG_I2C_SLAVE_SDA 25
//#define CONFIG_I2C_SLAVE_PORT_NUM 0
//#define CONFIG_I2C_SLAVE_ADDRESS 0x28


#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)



#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */


#define I2C_MASTER_SCL_IO GPIO_NUM_16               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_4               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */


#define ESP_SLAVE_READ_ADDR 0x52//0x20//0x52 			//JOYSTICK USE DIFFERENT ADDRESS FOR READ AND WRITE
#define ESP_SLAVE_WRITE_ADDR 0x52//0x20//0x52 			/*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL I2C_MASTER_ACK                             /*!< I2C ack value */
#define NACK_VAL I2C_MASTER_NACK                            /*!< I2C nack value */






#define DATA_LENGTH 6
#define joystickButtonLowBit 0x01
#define joystickButtonUpBit 0x02
namespace I2C{



	extern uint8_t data_wr[DATA_LENGTH];
	extern uint8_t data_rd[DATA_LENGTH];

	esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
	esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);
	esp_err_t i2c_master_init();


	extern uint8_t joystickX;
	extern uint8_t joystickY;
	extern bool joystickButtonHold_low;
	extern bool joystickButtonHold_up;
	extern bool joystickError;

	void  taskFunction ( void *pvParameter );

	void start_I2C();
	void joystick_init();


}
#endif // __I2C__INCLUDE_GUARD
