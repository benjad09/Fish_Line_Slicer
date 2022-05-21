#ifndef INCLUDE_DRIVERS_LCDI2C_H_
#define INCLUDE_DRIVERS_LCDI2C_H_

#include <zephyr/types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*lcdi2c_init_t)(const struct device *dev);
typedef int (*lcdi2c_clear_t)(const struct device *dev);
typedef int (*lcdi2c_home_t)(const struct device *dev);
typedef int (*lcdi2c_noDisplay_t)(const struct device *dev);
typedef int (*lcdi2c_display_t)(const struct device *dev);
typedef int (*lcdi2c_blink_off_t)(const struct device *dev);
typedef int (*lcdi2c_blink_on_t)(const struct device *dev);
typedef int (*lcdi2c_cursor_off_t)(const struct device *dev);
typedef int (*lcdi2c_cursor_on_t)(const struct device *dev);
typedef int (*lcdi2c_createChar_t)(const struct device *dev,uint8_t inst, uint8_t *array);
typedef int (*lcdi2c_setCursor_t)(const struct device *dev,uint8_t row, uint8_t column);
typedef int (*lcdi2c_printstr_t)(const struct device *dev,uint8_t *str);
typedef int (*lcdi2c_printint_t)(const struct device *dev,uint16_t num);

__subsystem struct lcdi2c_api {
    lcdi2c_init_t  init;
    lcdi2c_clear_t clear;
    lcdi2c_home_t home;
    lcdi2c_noDisplay_t noDisplay;
    lcdi2c_display_t display;
    lcdi2c_blink_off_t blink_off;
    lcdi2c_blink_on_t blink_on;
    lcdi2c_cursor_off_t cursor_off;
    lcdi2c_cursor_on_t cursor_on;
    lcdi2c_createChar_t createChar;
    lcdi2c_setCursor_t setCursor;
    lcdi2c_printstr_t printstr;
    lcdi2c_printint_t printint;
};
__syscall int lcdi2c_init(const struct device *dev);
static inline int z_impl_lcdi2c_init(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->init(dev);
}

__syscall int lcdi2c_clear(const struct device *dev);
static inline int z_impl_lcdi2c_clear(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->clear(dev);
}


__syscall int lcdi2c_home(const struct device *dev);
static inline int z_impl_lcdi2c_home(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->home(dev);
}


__syscall int lcdi2c_noDisplay(const struct device *dev);
static inline int z_impl_lcdi2c_noDisplay(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->noDisplay(dev);
}


__syscall int lcdi2c_display(const struct device *dev);
static inline int z_impl_lcdi2c_display(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->display(dev);
}


__syscall int lcdi2c_blink_off(const struct device *dev);
static inline int z_impl_lcdi2c_blink_off(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->blink_off(dev);
}


__syscall int lcdi2c_blink_on(const struct device *dev);
static inline int z_impl_lcdi2c_blink_on(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->blink_on(dev);
}


__syscall int lcdi2c_cursor_off(const struct device *dev);
static inline int z_impl_lcdi2c_cursor_off(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->cursor_off(dev);
}


__syscall int lcdi2c_cursor_on(const struct device *dev);
static inline int z_impl_lcdi2c_cursor_on(const struct device *dev){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->cursor_on(dev);
}


__syscall int lcdi2c_createChar(const struct device *dev,uint8_t inst, uint8_t *array);
static inline int z_impl_lcdi2c_createChar(const struct device *dev,uint8_t inst, uint8_t *array){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->createChar(dev,inst,array);
}


__syscall int lcdi2c_setCursor(const struct device *dev,uint8_t row, uint8_t column);
static inline int z_impl_lcdi2c_setCursor(const struct device *dev,uint8_t row, uint8_t column){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->setCursor(dev,row,column);
}


__syscall int lcdi2c_printstr(const struct device *dev,uint8_t *str);
static inline int z_impl_lcdi2c_printstr(const struct device *dev,uint8_t *str){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->printstr(dev,str);
}


__syscall int lcdi2c_printint(const struct device *dev,uint16_t num);
static inline int z_impl_lcdi2c_printint(const struct device *dev,uint16_t num){
    const struct lcdi2c_api *api =
        (const struct lcdi2c_api *)dev->api;
        return api->printint(dev,num);
}



#ifdef __cplusplus
}
#endif


#include <syscalls/lcd_i2c.h>

#endif