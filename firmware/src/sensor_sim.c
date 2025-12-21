#include "sensor_sim.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensor, LOG_LEVEL_DBG);

static uint16_t current_value;
static sensor_fault_mode_t current_fault;

void sensor_sim_init(void) {
    current_value = 0;
    current_fault = FAULT_NONE;
    LOG_INF("Sensor simulator initialized (I2C interface mocked)");
}

uint16_t sensor_sim_read(void) {
    switch (current_fault) {
        case FAULT_NO_RESPONSE:
            LOG_ERR("Sensor reading failed: NO_RESPONSE on mock I2C bus");
            // Could return 0xFFFF or trigger state machine error,
            // let's return a distinct invalid looking value
            return 0xFFFF;
            
        case FAULT_OUT_OF_RANGE:
            return 0xFFFE;
            
        case FAULT_STUCK:
            return current_value; // Return the same value as last time
            
        case FAULT_NONE:
        default:
            current_value++;
            if (current_value >= 0xFF00) { current_value = 0; }
            return current_value;
    }
}

void sensor_sim_inject_fault(sensor_fault_mode_t mode) {
    current_fault = mode;
    LOG_WRN("Injected sensor fault mode: %d", mode);
}
