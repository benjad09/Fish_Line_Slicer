/**
 * @file drivers/servo.h
 *
 * @brief Public APIs for the phase 1 motor drives driver. stollen mercilusly from arduino \m/ ._. \m/
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef INCLUDE_STEPPER_H_
#define INCLUDE_STEPPER_H_

#include <zephyr/types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef int (*stepper_enable_t)(const struct device *dev); //attach equivlent

typedef int (*stepper_set_dir_t)(const struct device *dev,uint8_t inst,uint8_t dir);

typedef int (*stepper_step_t)(const struct device *dev,uint8_t inst);

typedef int (*stepper_disable_t)(const struct device *dev);



__subsystem struct stepper_api {
    stepper_enable_t     enable;
    stepper_set_dir_t    set_dir;
    stepper_step_t       step;
    stepper_disable_t    disable;
};

__syscall int stepper_enable(const struct device *dev); //attach equivlent
static inline int z_impl_stepper_enable(const struct device *dev)
{
        const struct stepper_api *api =
    (const struct stepper_api *)dev->api;
    return api->enable(dev);
}

__syscall int stepper_set_dir(const struct device *dev,uint8_t inst,uint8_t dir);
static inline int z_impl_stepper_set_dir(const struct device *dev,uint8_t inst,uint8_t dir)
{
    const struct stepper_api *api =
    (const struct stepper_api *)dev->api;
    return api->set_dir(dev,inst,dir);
}

__syscall int stepper_step(const struct device *dev,uint8_t inst);
static inline int z_impl_stepper_step(const struct device *dev,uint8_t inst)
{
    const struct stepper_api *api =
    (const struct stepper_api *)dev->api;
    return api->step(dev,inst);
}

__syscall int stepper_disable(const struct device *dev);
static inline int z_impl_stepper_disable(const struct device *dev)
{
    const struct stepper_api *api =
    (const struct stepper_api *)dev->api;
    return api->disable(dev);
}

#ifdef __cplusplus
}
#endif


#include <syscalls/stepper.h>



#endif //INCLUDE_DRIVERS_ROTATION_H_