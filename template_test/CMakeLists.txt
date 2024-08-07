# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BKLIBC_URL https://gitlab.bigkaiser.tel/r-d-group/libraries/bklibc.git)
set(BKLIBC_TAG main)
set(BKLIBC_PATH ${CMAKE_SOURCE_DIR}/../../bklibc/${BKLIBC_TAG})

# Run the batch script with arguments
execute_process(
    COMMAND clone_repository.bat ${BKLIBC_URL} ${BKLIBC_PATH} ${BKLIBC_TAG}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../scripts
    RESULT_VARIABLE result
    OUTPUT_VARIABLE script_output
)

if(result EQUAL 0)
    message(STATUS "Script executed successfully")
    message(STATUS "${script_output}")
elseif(result EQUAL 1)
    message(WARNING "Script executed with warning:")
    message(WARNING "${script_output}")
else()
    message(FATAL_ERROR "Failed to run clone_repository.bat. Return code: ${result}")
    message(FATAL_ERROR "${script_output}")
endif()

#prepend current directory to board root list to allow adding new boards and overwriting boards that exist in the zephyr tree
#list(PREPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
#append board root to include boards from bklibc
list(APPEND BOARD_ROOT ${BKLIBC_PATH}/include/zephyr)

add_compile_definitions(SERIAL_LOG_LEVEL=3)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(template_test)

target_include_directories(app
    PRIVATE
        ${BKLIBC_PATH}/include
        ${BKLIBC_PATH}/include/zephyr

)
target_sources(app
    PRIVATE
    src/main.c
    ${BKLIBC_PATH}/src/rion_application_interface.c
    ${BKLIBC_PATH}/src/rion_encoding.c
    ${BKLIBC_PATH}/src/zephyr/ble_serial.c
    ${BKLIBC_PATH}/src/zephyr/serial.c
    ${BKLIBC_PATH}/src/zephyr/serial_internal.c
    ${BKLIBC_PATH}/src/zephyr/uart_serial.c
    ${BKLIBC_PATH}/src/rion_object_buffer.c
    src/api.c)
