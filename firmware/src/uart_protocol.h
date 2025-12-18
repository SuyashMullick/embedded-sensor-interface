#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// Frame constants
#define FRAME_PREAMBLE_0 0xAA
#define FRAME_PREAMBLE_1 0x55
#define FRAME_VERSION    0x01

// Message Types
#define MSG_GET_STATUS     0x01
#define MSG_STATUS_RSP     0x02
#define MSG_SET_PARAM      0x03
#define MSG_GET_PARAM      0x04
#define MSG_PARAM_RSP      0x05
#define MSG_RESET_MOD      0x06
#define MSG_ERROR_RSP      0x07

void uart_protocol_init(void);

#endif /* UART_PROTOCOL_H */
