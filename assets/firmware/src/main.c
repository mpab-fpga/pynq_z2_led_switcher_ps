/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * main.c: 'wiring' application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "platform.h"

#include "sleep.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"

#define DIP_ID XPAR_SWITCHES_DEVICE_ID
#define DIP_CHANNEL 1
#define DIP_DIR 0b1111

#define BTN_ID XPAR_BUTTONS_DEVICE_ID
#define BTN_CHANNEL 1
#define BTN_DIR 0b1111

#define LED_ID XPAR_LEDS_DEVICE_ID
#define LED_CHANNEL 1
#define LED_DIR 0b0

#define MODE_DELAY 250000

typedef struct {
  XGpio led_device, btn_device, dip_device;
  u32 dip_data, led_data, btn_data;
} Device;

int device_init(Device *pd) {
  XGpio_Config *cfg_ptr;

  int error = 0;

  cfg_ptr = XGpio_LookupConfig(DIP_ID);
  if (!cfg_ptr) {
    xil_printf("FATAL: DIP_ID device configuration not found\n\r");
    error |= 1;
  }
  XGpio_CfgInitialize(&pd->dip_device, cfg_ptr, cfg_ptr->BaseAddress);
  XGpio_SetDataDirection(&pd->dip_device, DIP_CHANNEL, DIP_DIR);

  cfg_ptr = XGpio_LookupConfig(LED_ID);
  if (!cfg_ptr) {
    xil_printf("FATAL: LED_ID device configuration not found\n\r");
    error |= 2;
  }
  XGpio_CfgInitialize(&pd->led_device, cfg_ptr, cfg_ptr->BaseAddress);
  XGpio_SetDataDirection(&pd->led_device, LED_CHANNEL, LED_DIR); // write

  cfg_ptr = XGpio_LookupConfig(BTN_ID);
  if (!cfg_ptr) {
    xil_printf("FATAL: BTN_ID device configuration not found\n\r");
    error |= 4;
  }
  XGpio_CfgInitialize(&pd->btn_device, cfg_ptr, cfg_ptr->BaseAddress);
  XGpio_SetDataDirection(&pd->btn_device, BTN_CHANNEL, BTN_DIR);

  return error;
}

void device_read(Device *pd) {
  pd->dip_data = XGpio_DiscreteRead(&pd->dip_device, DIP_CHANNEL);
  pd->btn_data = XGpio_DiscreteRead(&pd->btn_device, BTN_CHANNEL);
}

void device_write(Device *pd) {
  XGpio_DiscreteWrite(&pd->led_device, LED_CHANNEL, pd->led_data);
}

void device_print(Device *pd) {
  xil_printf("DIP:0x%01X, BTN:0x%01X, LED:0x%01X\n\r", pd->dip_data,
             pd->btn_data, pd->led_data);
}

void print_mode(int mode) {
  switch (mode) { // mode

  case 0:
    xil_printf("mode 0 - LEDs = buttons\n\r");
    break;

  case 1:
    xil_printf("mode 1 - LED counter\n\r");
    break;

  case 2:
    xil_printf("mode 2 - LED ripple\n\r");
    break;

  case 3:
    xil_printf("mode 3 - exit\n\r");
    break;

  default:
    xil_printf("unknown mode 0x%04X\n\r", mode);
    break;
  }
}

void start_mode(Device *pd) {
  switch (pd->dip_data) { // mode

  case 0:
    pd->led_data = pd->btn_data;
    break;

  case 1:
    pd->led_data = 0; // reset counter
    break;

  case 2:
    pd->led_data = 1; // reset bit
    break;

  case 3:
  default:
    break;
  }

  device_write(pd);
}

void run_mode(Device *pd) {
  switch (pd->dip_data) { // mode

  case 0:
    pd->led_data = pd->btn_data;
    device_write(pd);
    // device_print(pd);
    usleep(MODE_DELAY);
    break;

  case 1:
    if (++pd->led_data > 0xF)
      pd->led_data = 0;
    device_write(pd);
    // device_print(pd);
    usleep(MODE_DELAY);
    break;

  case 2:
    pd->led_data = pd->led_data << 1;
    if (pd->led_data > 0x8)
      pd->led_data = 1;
    device_write(pd);
    // device_print(pd);
    usleep(MODE_DELAY);
    break;

  case 3:
  default:
    break;
  }
}

int main() {
  xil_printf("\r\n========================================\r\n");
  init_platform();
  xil_printf("init_platform()\r\n\r\n");

  Device d;

  int error = device_init(&d);
  if (error) {
    xil_printf("FATAL: error %d - device not configured\n\r", error);
    cleanup_platform();
    return 0;
  }

  xil_printf("Use DIP switches to select mode\n\r");
  xil_printf("----------------------------------------\r\n");
  print_mode(0);
  print_mode(1);
  print_mode(2);
  print_mode(3);

  device_read(&d);

  xil_printf("Current mode\n\r");
  print_mode(d.dip_data);

  u32 dip_data = d.dip_data; // save current DIP state

  while (dip_data != 3) {
    device_read(&d);

    if (d.dip_data != dip_data) {
      print_mode(d.dip_data);
      // device_print(&d);
      dip_data = d.dip_data;
      start_mode(&d);
      continue; // ensure first data value is displayed
    }
    run_mode(&d);
  }

  cleanup_platform();
  xil_printf("cleanup_platform()\r\n");
  return 0;
}
