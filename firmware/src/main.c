#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "state_machine.h"
#include "parameters.h"
#include "sensor_sim.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static struct k_timer tick_timer;
static struct k_work_q tick_work_q;
static K_THREAD_STACK_DEFINE(tick_work_q_stack, CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE);

// Work item for running statemachine and sampling
static struct k_work state_machine_work;

static void tick_work_handler(struct k_work *work) {
    state_machine_run_iteration();
}

static void tick_timer_handler(struct k_timer *timer_id) {
    k_work_submit_to_queue(&tick_work_q, &state_machine_work);
}

int main(void)
{
    LOG_INF("Starting Embedded Sensor Interface Module");
    
    // Explicit initialization of state machine logic.
    // It is up to state machine to call parameters_init(), etc on STATE_INIT.
    state_machine_init();

    // Start a dedicated work queue for our 1ms ticks
    k_work_queue_start(&tick_work_q, tick_work_q_stack,
                       K_THREAD_STACK_SIZEOF(tick_work_q_stack),
                       K_PRIO_COOP(7), NULL);

    k_work_init(&state_machine_work, tick_work_handler);
    
    // Timer running at 1ms (1000Hz)
    k_timer_init(&tick_timer, tick_timer_handler, NULL);
    k_timer_start(&tick_timer, K_MSEC(1), K_MSEC(1));

    // The main thread can just sleep, all action happens in interrupts/work_queue
    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
