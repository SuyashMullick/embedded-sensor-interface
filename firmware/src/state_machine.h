#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STATE_BOOT = 0,
    STATE_INIT = 1,
    STATE_RUN = 2,
    STATE_ERROR = 3,
    STATE_RECOVERY = 4
} system_state_t;

void state_machine_init(void);
system_state_t state_machine_get_current(void);

// Call this from main loop
void state_machine_run_iteration(void);

// External triggers
void state_machine_trigger_error(void);
void state_machine_trigger_reset(void);

uint32_t state_machine_get_uptime_ms(void);
uint32_t state_machine_get_error_flags(void);

#endif /* STATE_MACHINE_H */
