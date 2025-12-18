#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
    LOG_INF("Starting Embedded Sensor Interface Module");
    
    while (1) {
        k_sleep(K_MSEC(1000));
    }
    return 0;
}
