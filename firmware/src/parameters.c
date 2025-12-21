#include "parameters.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(params, LOG_LEVEL_DBG);

typedef struct {
    uint16_t sample_rate;
    uint16_t status_period_ms;
    bool sensor_enable;
} param_store_t;

static param_store_t current_params;

void parameters_init(void) {
    current_params.sample_rate = 100;
    current_params.status_period_ms = 1000;
    current_params.sensor_enable = true;
    LOG_INF("Parameters initialized to defaults");
}

uint16_t parameters_get_sample_rate(void) {
    return current_params.sample_rate;
}

bool parameters_set_sample_rate(uint16_t rate) {
    if (rate >= 1 && rate <= 1000) {
        current_params.sample_rate = rate;
        LOG_INF("Set SENSOR_SAMPLE_RATE = %u", rate);
        return true;
    }
    LOG_WRN("Invalid SENSOR_SAMPLE_RATE: %u", rate);
    return false;
}

uint16_t parameters_get_status_period(void) {
    return current_params.status_period_ms;
}

bool parameters_set_status_period(uint16_t period_ms) {
    if (period_ms >= 100 && period_ms <= 5000) {
        current_params.status_period_ms = period_ms;
        LOG_INF("Set STATUS_PERIOD_MS = %u", period_ms);
        return true;
    }
    LOG_WRN("Invalid STATUS_PERIOD_MS: %u", period_ms);
    return false;
}

bool parameters_get_sensor_enable(void) {
    return current_params.sensor_enable;
}

bool parameters_set_sensor_enable(bool enable) {
    current_params.sensor_enable = enable;
    LOG_INF("Set SENSOR_ENABLE = %d", enable);
    return true;
}
