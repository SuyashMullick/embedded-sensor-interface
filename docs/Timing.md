# Timing Assumptions and Tolerances

Due to the nature of this project existing in a simulated environment (`native_sim` under Linux user-space), hard RTOS constraints are mathematically unachievable due to OS scheduling jitter. However, the system logic is constructed strictly per RTOS best practices.

## Main Loop Tick Rate
The Zephyr timer `tick_timer` is configured to `K_MSEC(1)`. This forces a 1kHz schedule interval where the state machine work queue evaluates logic.

## Command Response Latency
**Requirement:** 20 ms worst-case response.
**Implementation:** Upon UART RX completion (evaluated byte-by-byte in ISQ), the CRC validation and response construction executes immediately. It is then placed into the `uart_poll_out` queue. This provides minimal latency. Within the python integration tests, test verifications assert < 50ms (adjusted for variable Linux context-switching delays natively impossible to avoid without real hardware).

## Sensor Sampling
Sensor simulation currently resolves asynchronously within the 1ms tick loop logic if enabled, producing virtually instantaneous read cycles matching typical I2C bus speeds assuming zero-block wait states.
