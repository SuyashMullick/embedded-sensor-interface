#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stdint.h>
#include <stdbool.h>

// Parameter IDs
#define PARAM_SENSOR_SAMPLE_RATE 0x01
#define PARAM_STATUS_PERIOD_MS   0x02
#define PARAM_SENSOR_ENABLE      0x03

void parameters_init(void);

uint16_t parameters_get_sample_rate(void);
bool parameters_set_sample_rate(uint16_t rate);

uint16_t parameters_get_status_period(void);
bool parameters_set_status_period(uint16_t period_ms);

bool parameters_get_sensor_enable(void);
bool parameters_set_sensor_enable(bool enable);

#endif /* PARAMETERS_H */
