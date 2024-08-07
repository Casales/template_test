#include "api.h"

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "rion_application_interface.h"
#include "serial.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS AND STATIC VARIABLES
#define SOFTWARE_VERSION {0,0,0}
#define LOG_MODULE_NAME api
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);
static uint8_t input_buffer[256];
static int64_t last_time_received = 0;
typedef struct send_notification_msgq_item
{
	rion_encoding_key_short_t key;
	rion_encoding_object_t const* args;
	bool verbose;
}send_notification_msgq_item_t;
K_MSGQ_DEFINE(send_notifications_msgq, sizeof(send_notification_msgq_item_t), 8, 4);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTION DECLARATIONS
static void send(uint8_t const* p_data, int len);
static void function_hello(rion_encoding_object_t const* args, bool verbose);
static void function_echo(rion_encoding_object_t const* args, bool verbose);
static void function_counter(rion_encoding_object_t const* args, bool verbose);
static void send_notifications_work_queue_handler(struct k_work * work);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION DEPENDENT STATIC VARIABLES
static rion_application_interface_function_t functions[API_FUNCTION_COUNT] = {
	[API_FUNCTION_HELLO] = { .name = "hello", .function = function_hello },
	[API_FUNCTION_ECHO] = { .name = "echo", .function = function_echo },
	[API_FUNCTION_COUNTER] = { .name = "counter", .description = "counts up when called", .function = function_counter },
};

K_WORK_DEFINE(send_notification_work, send_notifications_work_queue_handler);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EVENT HANDLERS

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTION DEFINITIONS
static void send(uint8_t const* p_data, int len)
{
	static int const min_pause_between_transmissions = 5;
	static uint64_t last_time_sent = 0;
	uint64_t time_since_last_sent = k_uptime_get() - last_time_sent;
	if (time_since_last_sent < min_pause_between_transmissions)
	{
		k_sleep(K_MSEC(min_pause_between_transmissions - time_since_last_sent));
	}
	serial_send(K_FOREVER, p_data, len);
	serial_wait_for_tx_complete(K_FOREVER);
	last_time_sent = k_uptime_get();
}

static void function_hello(rion_encoding_object_t const* args, bool verbose)
{
	char const hello_world_text[] = "hello world";
	rion_application_interface_response_append_string(NULL, hello_world_text, sizeof(hello_world_text) - 1);
}

static void function_echo(rion_encoding_object_t const* args, bool verbose)
{
	if (args->type == RION_ENCODING_FIELD_TYPE_UTF8_SHORT
	|| args->type == RION_ENCODING_FIELD_TYPE_UTF8)
	{
		rion_application_interface_response_append_string(NULL, (char const*)(args->p_content), args->content_size);
	}
}

static void function_counter(rion_encoding_object_t const* args, bool verbose)
{
	static int counter = 0;
	rion_application_interface_response_append_int(NULL, ++counter);
}

static void send_notifications_work_queue_handler(struct k_work * work)
{
	for (int i = 0; i < send_notifications_msgq.max_msgs; i++)
	{
		send_notification_msgq_item_t item;
		if (k_msgq_get(&send_notifications_msgq, &item, K_NO_WAIT) != 0)
		{
			break;
		}
		rion_application_interface_send_notification(&api, item.key, item.args, item.verbose);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rion_application_interface_t api = {
	.version = SOFTWARE_VERSION,
	.name = "template_test",
	.send_function = send,
	.function_list = functions,
	.function_count = sizeof(functions)/sizeof(rion_application_interface_function_t),
};

void api_init()
{
	serial_enable(SERIAL_TYPE_UART | SERIAL_TYPE_BLE);
}

void api_handle_events()
{
	static int bytes_in_input_buffer = 0;
	serial_line_t const* p_line = serial_get_line(K_NO_WAIT);
	
	//if no data was received before timeout discard the input buffer and wait for new data
	int const receive_timeout = 1000;
	if (p_line->len == 0) 
	{
		if ((bytes_in_input_buffer > 0) && ((k_uptime_get() - last_time_received) > receive_timeout))
		{
			LOG_INF("Did not receive data for %d msecs, discarding %d bytes", receive_timeout, bytes_in_input_buffer);
			bytes_in_input_buffer = 0;
		}
		return;
	}
	else
	{
		last_time_received = k_uptime_get();
	}
		
	//if the new data does not fit into the input buffer discard the input buffer
	if (bytes_in_input_buffer + p_line->len > sizeof(input_buffer))
	{
		bytes_in_input_buffer = 0;
	}
	
	//if it fits into the input buffer copy the data and handle the message
	if (p_line->len <= sizeof(input_buffer))
	{
		memcpy(input_buffer + bytes_in_input_buffer, p_line->p_data, p_line->len);
		bytes_in_input_buffer += p_line->len;
		
		if (rion_application_interface_handle_message(&api, input_buffer, bytes_in_input_buffer) == RION_APPLICATION_INTERFACE_CODE_SUCCESS)
		{
			bytes_in_input_buffer = 0;
		}
	}
}

void api_send_notification(api_function_t function, rion_encoding_object_t const * args, bool verbose)
{
	send_notification_msgq_item_t item = { 
		.key = RION_APPLICATION_INTERFACE_GET_KEY_SHORT(function),
		.args = args,
		.verbose = verbose,
	};
	int result = k_msgq_put(&send_notifications_msgq, &item, K_NO_WAIT);
	if (result != 0)
	{
		LOG_ERR("Could not put item on send notification message queue! (Error: %d)", result);
	}
	k_work_submit(&send_notification_work);
}
