#ifndef SENSOR_SIM_H
#define SENSOR_SIM_H

#include <stdint.h>
#include <stdbool.h>

void sensor_sim_init(void);
uint16_t sensor_sim_read(void);

#endif /* SENSOR_SIM_H */
