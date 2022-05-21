# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)


list(APPEND ZEPHYR_EXTRA_MODULES
    C:/Code/ncs/v1.7.1/hal_stm32/
  )
  
list(APPEND ZEPHYR_EXTRA_MODULES
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_drivers/
)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c
                           src/lcd_manager.c
)
