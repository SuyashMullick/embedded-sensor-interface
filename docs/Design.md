# System Design Documentation

## 1. System Overview

The Embedded Sensor Interface Module (ESIM) manages a simulated digital sensor, exposes a UART control interface, maintains an internal state machine, and provides a parameterized execution environment.

### Components:
- **`main.c`**: Initializes Zephyr features, thread queues, and registers the 1ms tick timer.
- **`uart_protocol.c`**: Handles PTY UART interrupts, byte parsing, CRC validation, and constructs response frames.
- **`parameters.c`**: Maintains the RAM dictionary of variables (`SENSOR_SAMPLE_RATE`, `STATUS_PERIOD_MS`, `SENSOR_ENABLE`) with bounds checking.
- **`sensor_sim.c`**: Provides a mocked I2C API that deterministically increments a simulated sensor measurement and exposes fault test hooks.
- **`state_machine.c`**: Core state transition engine holding the authoritative status of the module.

## 2. State Machine

The System utilizes a single definitive state machine executing on a 1ms schedule timer via a Zephyr work-queue.

| ID | State | Description |
|---|---|---|
| 0 | `BOOT` | Cold start entry point. Initial sanity checks. Transits to `INIT` automatically. |
| 1 | `INIT` | Invokes peripheral constructors. Initializes parameters to defaults. Transitions to `RUN`. |
| 2 | `RUN`  | Normal active operational state. Handled primarily through asynchronous interrupts (UART) and asynchronous work queues (Sensors). |
| 3 | `ERROR`| Unrecoverable fault (e.g., CRC error threshold, failed peripheral, mocked invalid sensor condition). Drops parameter changes. |
| 4 | `RECOVERY` | Entered manually via `RESET_MODULE`. Clears error flags and returns to `INIT`. |

## 3. Initialization and Startup Sequence

1. Zephyr `kernel` starts, initializes logging and PTY UART (`uart1`).
2. `main()` thread wakes, executes `state_machine_init()` pushing the system into `BOOT`.
3. `main()` registers and launches the 1ms Zephyr `k_timer`.
4. The asynchronous work queue fires the state machine iteration.
5. In `BOOT`, transition to `INIT`.
6. `INIT` constructs logical blocks (`parameters_init()`, `sensor_sim_init()`, `uart_protocol_init()`).
7. System transitions to `RUN`.
