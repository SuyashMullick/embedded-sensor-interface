#ifndef SENSOR_SIM_H
#define SENSOR_SIM_H

#include <stdint.h>
#include <stdbool.h>

void sensor_sim_init(void);
uint16_t sensor_sim_read(void);

// Test hooks
typedef enum {
    FAULT_NONE,
    FAULT_NO_RESPONSE,
    FAULT_OUT_OF_RANGE,
    FAULT_STUCK
} sensor_fault_mode_t;

void sensor_sim_inject_fault(sensor_fault_mode_t mode);

#endif /* SENSOR_SIM_H */
