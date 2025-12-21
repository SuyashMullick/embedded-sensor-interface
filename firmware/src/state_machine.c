#include "state_machine.h"
#include "parameters.h"
#include "sensor_sim.h"
#include "uart_protocol.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sm, LOG_LEVEL_DBG);

static system_state_t current_state;
static uint32_t uptime_ms;
static uint32_t error_flags;
static int64_t last_tick;

void state_machine_init(void) {
    current_state = STATE_BOOT;
    uptime_ms = 0;
    error_flags = 0;
    last_tick = k_uptime_get();
    LOG_INF("State Machine Initialized in BOOT");
}

system_state_t state_machine_get_current(void) {
    return current_state;
}

uint32_t state_machine_get_uptime_ms(void) {
    return uptime_ms;
}

uint32_t state_machine_get_error_flags(void) {
    return error_flags;
}

void state_machine_trigger_error(void) {
    if (current_state != STATE_ERROR) {
        LOG_ERR("Triggering ERROR state");
        current_state = STATE_ERROR;
        error_flags |= 0x01; // General error flag
    }
}

void state_machine_trigger_reset(void) {
    LOG_WRN("Triggering RESET -> RECOVERY state");
    current_state = STATE_RECOVERY;
}

static void transition_to(system_state_t new_state) {
    LOG_INF("Transition: %d -> %d", current_state, new_state);
    current_state = new_state;
}

void state_machine_run_iteration(void) {
    int64_t now = k_uptime_get();
    uptime_ms += (now - last_tick);
    last_tick = now;

    switch (current_state) {
        case STATE_BOOT:
            // Simulate trivial bootup checks
            transition_to(STATE_INIT);
            break;

        case STATE_INIT:
            parameters_init();
            sensor_sim_init();
            uart_protocol_init();
            transition_to(STATE_RUN);
            break;

        case STATE_RUN:
            // Main active state handled by interrupts/timers
            // Nothing explicitly blocked here in the tick
            break;

        case STATE_ERROR:
            // Wait for reset command or watchdog (if we had one)
            // Just sit in error unless reset requested
            break;

        case STATE_RECOVERY:
            // Best effort clear and re-init
            error_flags = 0;
            transition_to(STATE_INIT);
            break;

        default:
            transition_to(STATE_ERROR);
            break;
    }
}
