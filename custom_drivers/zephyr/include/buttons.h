#ifndef INCLUDE_DRIVERS_BUTTONS_H_
#define INCLUDE_DRIVERS_BUTTONS_H_

#include <zephyr/types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif


struct button_cb {
    //Every time there is a new tilt this function is called
    void (*button_pressed)(uint16_t button);

    //Every time there is a update in the raise lower position this function is called
    void (*button_unpressed)(uint16_t button);
};

typedef int (*buttons_register_cb_t)(const struct device *dev,struct button_cb *callbacks);

__subsystem struct button_api {
    buttons_register_cb_t register_cb;
};

__syscall int buttons_register_cb(const struct device *dev,struct button_cb *callbacks);
static inline int z_impl_buttons_register_cb(const struct device *dev,struct button_cb *callbacks)
{
        const struct button_api *api =
        (const struct button_api *)dev->api;
        return api->register_cb(dev,callbacks);
}



#ifdef __cplusplus
}
#endif


#include <syscalls/buttons.h>

#endif