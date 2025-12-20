# Interface Control Document (ICD)

## 1. Physical Layer 
- **UART over PTY** (Simulated)
- **Baud Rate**: 115200
- **Configuration**: 8 data bits, no parity, 1 stop bit, no hardware flow control.

## 2. Frame Format

All communication utilizes the following deterministic binary frame structure. Multi-byte fields are Big-Endian.

| Field | Size (Bytes) | Description |
|-------|--------------|-------------|
| Preamble | 2 | `0xAA 0x55` |
| Version | 1 | `0x01` |
| Message Type | 1 | ID of the message payload |
| Payload Length | 2 | Length of payload `N` |
| Payload | N | Byte-stream of message data |
| CRC-16 | 2 | CRC-16 (CCITT) computed over Version, Type, Length, and Payload. |

## 3. Message Types

| ID | Name | Direction | Description |
|---|---|---|---|
| `0x01` | GET_STATUS | Host → ESIM | Requests the system status |
| `0x02` | STATUS_RESPONSE| ESIM → Host | Reports system status and faults |
| `0x03` | SET_PARAMETER| Host → ESIM | Write a configuration parameter |
| `0x04` | GET_PARAMETER| Host → ESIM | Read a configuration parameter |
| `0x05` | PARAM_RESPONSE| ESIM → Host | Returns the parameter value |
| `0x06` | RESET_MODULE | Host → ESIM | Triggers state machine recovery |
| `0x07` | ERROR_RESPONSE | ESIM → Host | Indicates an invalid request |

## 4. Parameter Table

| ID | Name | Type | Range | Default |
|---|---|---|---|---|
| `0x01` | SENSOR_SAMPLE_RATE | uint16 | 1–1000 Hz | 100 |
| `0x02` | STATUS_PERIOD_MS | uint16 | 100–5000 | 1000 |
| `0x03` | SENSOR_ENABLE | bool | 0/1 | 1 |

Invalid writes to the parameter table return an `ERROR_RESPONSE`.

## 5. Status Payload

`STATUS_RESPONSE` (Length: 18)

| Offset | Type | Name |
|---|---|---|
| 0 | uint8 | Current State (0=BOOT, 1=INIT, 2=RUN, 3=ERR) |
| 1-4 | uint32 | Uptime (ms) |
| 5-8 | uint32 | Error Flags Bitfield |
| 9-12 | uint32 | UART RX Error Count |
| 13-16 | uint32 | UART TX Error Count |
| 17 | uint8 | Sensor Fault Flags |
