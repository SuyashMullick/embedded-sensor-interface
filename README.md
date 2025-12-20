# Embedded Sensor Interface Module (ESIM)

A simulation-first deterministic embedded sensor interface module representative of subsystems used in defense and aerospace platforms.

## Overview

The ESIM bridges a host system and a digital sensor via a simulated interface. It manages configuration parameters, samples sensor data deterministically, reports health status, and conforms to a strict binary UART protocol.

This repository demonstrates the **pre-hardware development phase** using Zephyr `native_sim`, producing firmware, python test suites, and design documentation before physical hardware is available.

## Key Attributes & Claims

### The project claims:
- **Interface rigor:** Strict adherence to a binary ICD with CRC checks and error handling.
- **Deterministic behavior:** Well-defined state machine with a 1 ms tick rate and fixed transition logic.
- **Pre-hardware verification discipline:** Test-driven development with Python host tools against a simulated Zephyr runtime.

### The project explicitly DOES NOT claim:
- Real hardware validation.
- Signal integrity verification.
- EMI/EMC compliance.
- Hard real-time guarantees (due to native_sim execution in Linux user-space).

## Repository Structure

- `firmware/` - Zephyr C firmware implementation for the `native_sim` target.
- `host_tools/` - Python CLI and `pytest` test suite.
- `docs/` - System, interface, timing, and verification documentation.

## Requirements

- Zephyr RTOS base and West workspace (for compiling the native_sim firmware)
- Python 3.11 with `pyserial`, `pytest`

## Execution

1. **Build the firmware:**
   ```bash
   west build -b native_sim firmware
   ```

2. **Run the firmware:**
   ```bash
   ./build/zephyr/zephyr.exe
   ```
   (Note the exposed `/dev/pts/X` virtual UART port)

3. **Verify via Tests:**
   ```bash
   pytest host_tools/tests/test_protocol.py --pty=/dev/pts/X
   ```
