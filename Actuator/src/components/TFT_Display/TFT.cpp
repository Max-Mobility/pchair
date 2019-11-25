
#include "TFT.hpp"
#include "Display.hpp"
#include "DisplayTask.hpp"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_intr_alloc.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MS_TO_TICKS(xTimeInMs)                                                 \
  (uint32_t)(((TickType_t)xTimeInMs * configTICK_RATE_HZ) / (TickType_t)1000)

int lcd_detected_type = 0;
int lcd_type;
spi_device_handle_t myspi;
/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd) {
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));                   // Zero out the transaction
  t.length = 8;                               // Command is 8 bits
  t.tx_buffer = &cmd;                         // The data is the cmd itself
  t.user = (void *)0;                         // D/C needs to be set to 0
  ret = spi_device_polling_transmit(spi, &t); // Transmit!
  assert(ret == ESP_OK);                      // Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len) {
  esp_err_t ret;
  spi_transaction_t t;
  if (len == 0)
    return;                 // no need to send anything
  memset(&t, 0, sizeof(t)); // Zero out the transaction
  t.length = len * 8;       // Len is in bytes, transaction length is in bits.
  t.tx_buffer = data;       // Data
  t.user = (void *)1;       // D/C needs to be set to 1
  ret = spi_device_polling_transmit(spi, &t); // Transmit!
  assert(ret == ESP_OK);                      // Should have had no issues.
}

// This function is called (in irq context!) just before a transmission starts.
// It will set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
  int dc = (int)t->user;
  gpio_set_level(PIN_NUM_DC, dc);
}

void lcd_spi_post_transfer_callback(spi_transaction_t *t) {}

uint32_t lcd_get_id(spi_device_handle_t spi) {
  // get_id cmd
  lcd_cmd(spi, 0x04);

  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8 * 3;
  t.flags = SPI_TRANS_USE_RXDATA;
  t.user = (void *)1;

  esp_err_t ret = spi_device_polling_transmit(spi, &t);
  assert(ret == ESP_OK);

  return *(uint32_t *)t.rx_data;
}

// Initialize the display
void lcd_init(spi_device_handle_t spi) {
  int cmd = 0;
  const lcd_init_cmd_t *lcd_init_cmds;

  // Initialize non-SPI GPIOs
  gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

  // Reset the display
  gpio_set_level(PIN_NUM_RST, 0);
  vTaskDelay(100 / portTICK_RATE_MS);
  gpio_set_level(PIN_NUM_RST, 1);
  vTaskDelay(100 / portTICK_RATE_MS);

  // detect LCD type
  uint32_t lcd_id = lcd_get_id(spi);

  printf("LCD ID: %08X\n", lcd_id);
  if (lcd_id == 0) {
    // zero, ili
    lcd_detected_type = LCD_TYPE_ILI;
    printf("ILI9341 detected.\n");
  } else {
    // none-zero, ST
    lcd_detected_type = LCD_TYPE_ST;
    printf("ST7789V detected.\n");
  }
  lcd_type = lcd_detected_type;
  if (lcd_type == LCD_TYPE_ST) {
    printf("LCD ST7789V initialization.\n");
    lcd_init_cmds = st_init_cmds;
  } else {
    printf("LCD ILI9341 initialization.\n");
    lcd_init_cmds = ili_init_cmds;
  }

  // Send all the commands
  while (lcd_init_cmds[cmd].databytes != 0xff) {
    lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
    lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
    if (lcd_init_cmds[cmd].databytes & 0x80) {
      vTaskDelay(100 / portTICK_RATE_MS);
    }
    cmd++;
  }

  /// Enable backlight
  gpio_set_level(PIN_NUM_BCKL, 0);
}

/* To send a set of lines we have to send a command, 2 data bytes, another
 * command, 2 more data bytes and another command before sending the line data
 * itself; a total of 6 transactions. (We can't put all of this in just one
 * transaction because the D/C line needs to be toggled in the middle.) This
 * routine queues these commands up as interrupt transactions so they get sent
 * faster (compared to calling spi_device_transmit several times), and at the
 * mean while the lines for next transactions can get calculated.
 */
void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata) {
  esp_err_t ret;
  int x;
  // Transaction descriptors. Declared static so they're not allocated on the
  // stack; we need this memory even when this function is finished because the
  // SPI driver needs access to it even while we're already calculating the next
  // line.
  static spi_transaction_t trans[6];

  // In theory, it's better to initialize trans and data only once and hang on
  // to the initialized variables. We allocate them on the stack, so we need to
  // re-init them each call.
  for (x = 0; x < 6; x++) {
    memset(&trans[x], 0, sizeof(spi_transaction_t));
    if ((x & 1) == 0) {
      // Even transfers are commands
      trans[x].length = 8;
      trans[x].user = (void *)0;
    } else {
      // Odd transfers are data
      trans[x].length = 8 * 4;
      trans[x].user = (void *)1;
    }
    trans[x].flags = SPI_TRANS_USE_TXDATA;
  }
  trans[0].tx_data[0] = 0x2A;                           // Column Address Set
  trans[1].tx_data[0] = 0;                              // Start Col High
  trans[1].tx_data[1] = 0;                              // Start Col Low
  trans[1].tx_data[2] = (240) >> 8;                     // End Col High
  trans[1].tx_data[3] = (240) & 0xff;                   // End Col Low
  trans[2].tx_data[0] = 0x2B;                           // Page address set
  trans[3].tx_data[0] = ypos >> 8;                      // Start page high
  trans[3].tx_data[1] = ypos & 0xff;                    // start page low
  trans[3].tx_data[2] = (ypos + PARALLEL_LINES) >> 8;   // end page high
  trans[3].tx_data[3] = (ypos + PARALLEL_LINES) & 0xff; // end page low
  trans[4].tx_data[0] = 0x2C;                           // memory write
  trans[5].tx_buffer = linedata;                  // finally send the line data
  trans[5].length = 240 * 2 * 8 * PARALLEL_LINES; // Data length, in bits
  trans[5].flags = 0; // undo SPI_TRANS_USE_TXDATA flag

  // Queue all transactions.
  for (x = 0; x < 6; x++) {
    ret = spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
    assert(ret == ESP_OK);
  }

  // When we are here, the SPI driver is busy (in the background) getting the
  // transactions sent. That happens mostly using DMA, so the CPU doesn't have
  // much to do here. We're not going to wait for the transaction to finish
  // because we may as well spend the time calculating the next line. When that
  // is done, we can call send_line_finish, which will wait for the transfers to
  // be done and check their status.
}

void send_line_finish(spi_device_handle_t spi) {
  spi_transaction_t *rtrans;
  esp_err_t ret;
  // Wait for all 6 transactions to be done and get back the results.
  for (int x = 0; x < 6; x++) {
    ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
    assert(ret == ESP_OK);
    // We could inspect rtrans now if we received any info back. The LCD is
    // treated as write-only, though.
  }
}

void TFT_taskFunction(void *pvParameter) // task that draw screen
{
  uint16_t *lines;
  // Allocate memory for the pixel buffers
  lines = (uint16_t *)heap_caps_malloc(240 * PARALLEL_LINES * sizeof(uint16_t),
                                       MALLOC_CAP_DMA);
  assert(lines != NULL);

  // Indexes of the line currently being sent to the LCD and the line we're
  // calculating.
  int sending_line = -1;

  while (1) {

    for (int y = 0; y < 320; y += PARALLEL_LINES) {
      //        	printf("%d\n",y);
      // Calculate a line.

      // Finish up the sending process of the previous line, if any
      if (sending_line != -1)
        send_line_finish(myspi);
      get_line_data(lines, y, PARALLEL_LINES);
      // Swap sending_line and calc_line
      sending_line = 1;
      // Send the line we currently calculated.
      send_lines(myspi, y, lines);
      // The line set is queued up for sending now; the actual sending happens
      // in the background. We can go on to calculate the next line set as long
      // as we do not touch line[sending_line]; the SPI sending process is still
      // reading from that.
    }
  }
}

// Simple routine to generate some patterns and send them to the LCD. Don't
// expect anything too impressive. Because the SPI driver handles transactions in
// the background, we can calculate the next line while the previous one is being
// sent.
void STDisplay_vram() {
  xTaskCreate(&TFT_taskFunction, // function the task runs
              "taskFunction_6",  // name of the task (should be short)
              2048,              // stack size for the task
              NULL,              // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
}

void get_line_data(uint16_t *dest, int line, int linect) {
  for (int y = line; y < line + linect; y++) {
    // printf("%d; %d\n",y,(line+linect));

    for (int x = 0; x < 240; x++) {
      //*dest++=myPalette[(unsigned char)(vram[y + x*320])];
      if (y < DISPLAY_HEIGHT) {
        *dest++ = myPalette[(unsigned char)(vram[y + x * 240])];
      } else {
        *dest++ = 0;
      }
    }
  }
}

void TFT_init() {
  esp_err_t ret;

  spi_bus_config_t buscfg = {

      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = PARALLEL_LINES * DISPLAY_HEIGHT * 2 + 8,
      .flags = NULL,
      .intr_flags = NULL,

  };

  spi_device_interface_config_t devcfg = {

      .command_bits =
          0, ///< Default amount of bits in command phase (0-16), used when
             ///< ``SPI_TRANS_VARIABLE_CMD`` is not used, otherwise ignored.
      .address_bits =
          0, ///< Default amount of bits in address phase (0-64), used when
             ///< ``SPI_TRANS_VARIABLE_ADDR`` is not used, otherwise ignored.
      .dummy_bits =
          0, ///< Amount of dummy bits to insert between address and data phase
      .mode = 0, // SPI mode 0
      .duty_cycle_pos =
          0, ///< Duty cycle of positive clock, in 1/256th increments (128 =
             ///< 50%/50% duty). Setting this to 0 (=not setting it) is
             ///< equivalent to setting this to 128.
      .cs_ena_pretrans = 0, ///< Amount of SPI bit-cycles the cs should be
                            ///< activated before the transmission (0-16). This
                            ///< only works on half-duplex transactions.
      .cs_ena_posttrans = 0,
      .clock_speed_hz = 10 * 1000 * 1000, // Clock out at 10 MHz
      .input_delay_ns = 0,
      .spics_io_num = PIN_NUM_CS, // CS pin
      .flags = 0,
      .queue_size = 7, // We want to be able to queue 7 transactions at a time
      .pre_cb = lcd_spi_pre_transfer_callback, // Specify pre-transfer callback
                                               // to handle D/C line
      .post_cb = lcd_spi_post_transfer_callback,

  };
  // Initialize the SPI bus
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
  ESP_ERROR_CHECK(ret);
  // Attach the LCD to the SPI bus
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &myspi);
  ESP_ERROR_CHECK(ret);
  // Initialize the LCD
  lcd_init(myspi);

  clear_vram();

  STDisplay_vram();
}
