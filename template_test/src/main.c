/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>

#include "api.h"

int main(void)
{
	api_init();
	
	static const struct device *const uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t last_rts = 0;
	uint32_t last_dtr = 0;
	uint32_t last_dcd = 0;
	uint32_t last_dsr = 0;
	
	while (1)
	{
		api_handle_events();
		k_sleep(K_MSEC(1));
		
		uint32_t rts = 0;
		uint32_t dtr = 0;
		uint32_t dcd = 0;
		uint32_t dsr = 0;
		uart_line_ctrl_get(uart, UART_LINE_CTRL_RTS, &rts);
		uart_line_ctrl_get(uart, UART_LINE_CTRL_DTR, &dtr);
		uart_line_ctrl_get(uart, UART_LINE_CTRL_DCD, &dcd);
		uart_line_ctrl_get(uart, UART_LINE_CTRL_DSR, &dsr);
		if (
			last_rts != rts ||
			last_dtr != dtr ||
			last_dcd != dcd ||
			last_dsr != dsr
			)
		{
			last_rts = rts;
			last_dtr = dtr;
			last_dcd = dcd;
			last_dsr = dsr;
			api_send_notification(API_FUNCTION_UART, NULL, false);
		}
	}
	return 0;
}
