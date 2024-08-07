/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "api.h"

int main(void)
{
	api_init();
	
	while (1)
	{
		api_handle_events();
		k_sleep(K_MSEC(10));
	}
	return 0;
}
