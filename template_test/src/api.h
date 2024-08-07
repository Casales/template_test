/*
 * application_interface.h
 *
 * Created: 12.02.2024 15:57:38
 *  Author: manuel.martin
 */ 


#ifndef API_H_
#define API_H_

#include "rion_application_interface.h"

typedef enum api_function {
	API_FUNCTION_HELLO,
	API_FUNCTION_ECHO,
	API_FUNCTION_COUNTER,
	API_FUNCTION_COUNT,
} api_function_t;

extern rion_application_interface_t api;

void api_init(void);
void api_handle_events(void);

void api_send_notification(api_function_t function, rion_encoding_object_t const * args, bool verbose);

#endif /* API_H_ */