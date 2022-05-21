

#define DT_DRV_COMPAT phase1_servo



#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <drivers/pwm.h>
#include <pm/device.h>
#include "../include/servo.h"
#include <logging/log.h>
#include "errno.h"


struct standard_servo_data {
    bool                          is_on;
    uint32_t                      period_us;
};

struct standard_servo_config {
    struct device                 *pwm_device;
    uint32_t                      pwm_pin;
    pwm_flags_t                   flags;

    uint32_t                      us_at_0;
    uint32_t                      us_at_180;
    uint32_t                      us_of_cycle;
};




static int standard_servo_start(const struct device *dev)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;
    if(data->is_on == false)
    {
        data->is_on = true;
        pwm_pin_set_usec(config->pwm_device,config->pwm_pin,config->us_of_cycle,data->period_us,config->flags);
    }

}

static int standard_servo_write(const struct device *dev,uint16_t angle)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;
    if(angle > 180)
    {
        return -EINVAL;
    }
    data->period_us = (((config->us_at_180-config->us_at_0)*angle)/180)+(config->us_at_0);
    if(data->is_on)
    {
        pwm_pin_set_usec(config->pwm_device,config->pwm_pin,config->us_of_cycle,data->period_us,config->flags);
    }
    return 0;
}

static int standard_servo_write_us(const struct device *dev,uint16_t us)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;

    if(us > config->us_at_180)
    {
        return -EINVAL;
    }
    data->period_us = us;
    if(data->is_on)
    {
        pwm_pin_set_usec(config->pwm_device,config->pwm_pin,config->us_of_cycle,data->period_us,config->flags);
    }

    return 0;
}

static int standard_servo_read(const struct device *dev)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;

    return((data->period_us-config->us_at_0)*180)/(config->us_at_180-config->us_at_0);
}

static int standard_servo_started(const struct device *dev)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;
    if(data->is_on == true)
    {
        return 1;
    }
    return 0;
}

static int standard_servo_stop(const struct device *dev)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;
    if(data->is_on == true)
    {
        data->is_on = false;
        
    }
    return 0;
}

static const struct servo_api standard_servo_api = {
    .start = standard_servo_start,
    .write = standard_servo_write,
    .write_us = standard_servo_write_us,
    .read = standard_servo_read,
    .started = standard_servo_started,
    .stop = standard_servo_stop,
};


static int standard_servo_init(const struct device *dev)
{
    struct standard_servo_data *data = dev->data;
    const struct standard_servo_config *config = dev->config;

    if(config->us_of_cycle < config->us_at_180)
    {
        return -1;
    }
    if(config->us_at_180 < config->us_at_0)
    {
        return -1;
    }
    data->is_on = false;
    data->period_us = ((config->us_at_180-config->us_at_0)/2)+(config->us_at_0);;
    return 0;
}


#define INITSERVO(i)                        \
static struct standard_servo_data standard_servo_data_##i; \
static const struct standard_servo_config standard_servo_config_##i = { \
    .pwm_device = DEVICE_DT_GET(DT_INST_PWMS_CTLR_BY_IDX(i,0)), \
    .pwm_pin = DT_INST_PWMS_CHANNEL_BY_IDX(i,0), \
    .flags = DT_INST_PWMS_FLAGS_BY_IDX(i,0), \
    .us_at_0 = DT_INST_PROP(i,pulse_width_min_us), \
    .us_at_180 = DT_INST_PROP(i,pulse_width_max_us), \
    .us_of_cycle = DT_INST_PROP(i,pulse_width_us), \
};                                                                                      \
DEVICE_DT_INST_DEFINE(i, standard_servo_init, NULL,		                                 \
    &standard_servo_data_##i,			                                                         \
    &standard_servo_config_##i, POST_KERNEL,	                                                 \
    CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &standard_servo_api);





DT_INST_FOREACH_STATUS_OKAY(INITSERVO)


