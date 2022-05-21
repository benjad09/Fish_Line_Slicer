

zephyr_include_directories(include)

#list(APPEND SYSCALL_INCLUDE_DIRS
#    C:/Code/ncs/drivers/include
#)

add_subdirectory_ifdef(CONFIG_BUTTONS   buttons)
add_subdirectory_ifdef(CONFIG_LCD       lcd)
add_subdirectory_ifdef(CONFIG_SERVO     servo)
add_subdirectory_ifdef(CONFIG_STEPPER   stepper)

