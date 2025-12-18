#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum {
    STATE_BOOT,
    STATE_INIT,
    STATE_RUN,
    STATE_ERROR,
    STATE_RECOVERY
} system_state_t;

void state_machine_init(void);
system_state_t state_machine_get_current(void);
void state_machine_run_iteration(void);
void state_machine_trigger_error(void);

#endif /* STATE_MACHINE_H */
