# Verification Plan

## 1. Automated Testing Strategy
Verification relies on black-box evaluation using Python `pytest` against the simulated UART (PTY) exported by the Zephyr `native_sim` firmware.

### 1.1 Protocol Compliance
- Ensure `MSG_GET_STATUS` responds with correct payload size (18 bytes).
- Ensure random noise or invalid CRC triggers no response or valid drop error metric reporting.
- Ensure all big-endian byte-packing logic correctly evaluates multi-byte variables over PTY.

### 1.2 Parameter Validation
- Read default values to confirm `BOOT/INIT` sequence executed correctly.
- Set bounds-valid parameter (`SENSOR_SAMPLE_RATE` to 500) and read back to confirm.
- Set bounds-invalid parameter (`SENSOR_SAMPLE_RATE` to 2000) and verify `MSG_ERROR_RSP` via parsing return payload, checking that the readback remains unchanged.

### 1.3 State Management
- Verify `STATUS_RESPONSE` reflects the `RUN` (2) state during testing.
- Send `RESET_MODULE` (MSG ID 0x06) and evaluate if uptime wraps or module resets efficiently.

### 1.4 Timing
- Utilize Python `time.perf_counter()` pre and post UART command execution.
- Assert total response roundtrip (including Linux PTY pipe latency) stays below 50ms.
