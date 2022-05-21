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
#ifndef INCLUDE_SERVO_H_
#define INCLUDE_SERVO_H_

#include <zephyr/types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef int (*servo_start_t)(const struct device *dev); //attach equivlent

typedef int (*servo_write_t)(const struct device *dev,uint16_t angle);

typedef int (*servo_write_us_t)(const struct device *dev,uint16_t us);

typedef int (*servo_read_t)(const struct device *dev);

typedef int (*servo_started_t)(const struct device *dev); //attached equivlent

typedef int (*servo_stop_t)(const struct device *dev); //detach equivlent


__subsystem struct servo_api {
    servo_start_t    start;
    servo_write_t    write;
    servo_write_us_t write_us;
    servo_read_t     read;
    servo_started_t  started;
    servo_stop_t     stop;
};


__syscall int servo_start(const struct device *dev); //attach equivlent
static inline int z_impl_servo_start(const struct device *dev)
{
    const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->start(dev);

}

__syscall int servo_write(const struct device *dev,uint16_t angle);
static inline int z_impl_servo_write(const struct device *dev,uint16_t angle)
{
        const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->write(dev,angle);
}

__syscall int servo_write_us(const struct device *dev,uint16_t us);
static inline int z_impl_servo_write_us(const struct device *dev,uint16_t us)
{
        const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->write_us(dev,us);

}

__syscall int servo_read(const struct device *dev);
static inline int z_impl_servo_read(const struct device *dev)
{
        const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->read(dev);

}

__syscall int servo_started(const struct device *dev); //attached equivlent
static inline int z_impl_servo_started(const struct device *dev)
{
        const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->started(dev);

}

__syscall int servo_stop(const struct device *dev); //detach equivlent
static inline int z_impl_servo_stop(const struct device *dev)
{
        const struct servo_api *api =
    (const struct servo_api *)dev->api;

    return api->stop(dev);

}


#ifdef __cplusplus
}
#endif


#include <syscalls/servo.h>


//#include "C:\Code\ODL_Git\ODL-BlindFirmware\build_2\zephyr\include\generated\syscalls\rot_fb.h"

#endif //INCLUDE_DRIVERS_ROTATION_H_

