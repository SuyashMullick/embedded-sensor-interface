#include "uart_protocol.h"
#include "parameters.h"
#include "sensor_sim.h"
#include "state_machine.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/crc.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(uart, LOG_LEVEL_DBG);

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0)); // native_sim PTY uart

#define MAX_PAYLOAD_LEN 64
#define RX_BUF_SIZE 128
static uint8_t rx_buf[RX_BUF_SIZE];
static size_t rx_idx = 0;

static uint32_t rx_err_cnt = 0;
static uint32_t tx_err_cnt = 0;

typedef enum {
    SYNC0, SYNC1, VERSION, TYPE, LEN0, LEN1, PAYLOAD, CRC0, CRC1
} parser_state_t;

static parser_state_t p_state = SYNC0;
static uint8_t p_type = 0;
static uint16_t p_len = 0;
static uint8_t p_payload[MAX_PAYLOAD_LEN];
static uint16_t p_payload_idx = 0;
static uint16_t p_crc_rx = 0;

static void send_frame(uint8_t type, uint8_t *payload, uint16_t len) {
    if (!device_is_ready(uart_dev)) return;

    uint8_t header[6] = {
        FRAME_PREAMBLE_0, FRAME_PREAMBLE_1,
        FRAME_VERSION, type,
        (uint8_t)(len >> 8), (uint8_t)(len & 0xFF)
    };
    
    // CRC over version + type + length + payload
    uint16_t crc = crc16_ccitt(0xFFFF, &header[2], 4);
    if (len > 0) {
        crc = crc16_ccitt(crc, payload, len);
    }
    
    // Write header
    for (int i=0; i<6; i++) uart_poll_out(uart_dev, header[i]);
    // Write Payload
    for (int i=0; i<len; i++) uart_poll_out(uart_dev, payload[i]);
    // Write CRC
    uart_poll_out(uart_dev, (uint8_t)(crc >> 8));
    uart_poll_out(uart_dev, (uint8_t)(crc & 0xFF));
}

static void handle_get_status(void) {
    uint8_t payload[18];
    
    payload[0] = (uint8_t)state_machine_get_current();
    uint32_t uptime = state_machine_get_uptime_ms();
    payload[1] = (uptime >> 24) & 0xFF;
    payload[2] = (uptime >> 16) & 0xFF;
    payload[3] = (uptime >> 8) & 0xFF;
    payload[4] = uptime & 0xFF;
    
    uint32_t err_flags = state_machine_get_error_flags();
    payload[5] = (err_flags >> 24) & 0xFF;
    payload[6] = (err_flags >> 16) & 0xFF;
    payload[7] = (err_flags >> 8) & 0xFF;
    payload[8] = err_flags & 0xFF;
    
    payload[9] = (rx_err_cnt >> 24) & 0xFF;
    payload[10] = (rx_err_cnt >> 16) & 0xFF;
    payload[11] = (rx_err_cnt >> 8) & 0xFF;
    payload[12] = rx_err_cnt & 0xFF;
    
    payload[13] = (tx_err_cnt >> 24) & 0xFF;
    payload[14] = (tx_err_cnt >> 16) & 0xFF;
    payload[15] = (tx_err_cnt >> 8) & 0xFF;
    payload[16] = tx_err_cnt & 0xFF;
    
    // Placeholder flags for sensor faults
    payload[17] = 0; 
    
    send_frame(MSG_STATUS_RSP, payload, 18);
}

static void handle_set_param(uint8_t *data, uint16_t len) {
    if (len < 1) goto err;
    uint8_t param_id = data[0];
    bool ok = false;
    
    if (param_id == PARAM_SENSOR_SAMPLE_RATE && len == 3) {
        uint16_t rate = (data[1] << 8) | data[2];
        ok = parameters_set_sample_rate(rate);
    } else if (param_id == PARAM_STATUS_PERIOD_MS && len == 3) {
        uint16_t period = (data[1] << 8) | data[2];
        ok = parameters_set_status_period(period);
    } else if (param_id == PARAM_SENSOR_ENABLE && len == 2) {
        ok = parameters_set_sensor_enable(data[1] ? true : false);
    }
    
    if (ok) {
        // Echo back GET param style essentially or just empty success
        uint8_t rsp[1] = {0x00}; // 0 = OK
        send_frame(MSG_PARAM_RSP, rsp, 1);
        return;
    }
err:
    {
        uint8_t r[1] = {0x01}; // Error
        send_frame(MSG_ERROR_RSP, r, 1);
    }
}

static void handle_get_param(uint8_t *data, uint16_t len) {
    if (len < 1) return;
    uint8_t param_id = data[0];
    
    uint8_t rsp[4];
    rsp[0] = param_id;
    
    if (param_id == PARAM_SENSOR_SAMPLE_RATE) {
        uint16_t v = parameters_get_sample_rate();
        rsp[1] = (v >> 8) & 0xFF;
        rsp[2] = v & 0xFF;
        send_frame(MSG_PARAM_RSP, rsp, 3);
    } else if (param_id == PARAM_STATUS_PERIOD_MS) {
        uint16_t v = parameters_get_status_period();
        rsp[1] = (v >> 8) & 0xFF;
        rsp[2] = v & 0xFF;
        send_frame(MSG_PARAM_RSP, rsp, 3);
    } else if (param_id == PARAM_SENSOR_ENABLE) {
        bool v = parameters_get_sensor_enable();
        rsp[1] = v ? 1 : 0;
        send_frame(MSG_PARAM_RSP, rsp, 2);
    }
}

static void process_frame(void) {
    uint8_t header[4] = {FRAME_VERSION, p_type, (uint8_t)(p_len >> 8), (uint8_t)(p_len & 0xFF)};
    uint16_t expected_crc = crc16_ccitt(0xFFFF, header, 4);
    if (p_len > 0) {
        expected_crc = crc16_ccitt(expected_crc, p_payload, p_len);
    }
    
    if (expected_crc != p_crc_rx) {
        LOG_WRN("CRC mismatch");
        rx_err_cnt++;
        return;
    }

    switch (p_type) {
        case MSG_GET_STATUS:
            handle_get_status();
            break;
        case MSG_SET_PARAM:
            handle_set_param(p_payload, p_len);
            break;
        case MSG_GET_PARAM:
            handle_get_param(p_payload, p_len);
            break;
        case MSG_RESET_MOD:
            state_machine_trigger_reset();
            break;
        default:
            rx_err_cnt++;
            LOG_WRN("Unknown message type");
            break;
    }
}

static void uart_callback(const struct device *dev, void *user_data) {
    if (!uart_irq_update(dev)) return;

    if (uart_irq_rx_ready(dev)) {
        uint8_t c;
        while (uart_fifo_read(dev, &c, 1) == 1) {
            switch(p_state) {
                case SYNC0: if(c==FRAME_PREAMBLE_0) p_state=SYNC1; break;
                case SYNC1: if(c==FRAME_PREAMBLE_1) p_state=VERSION; else p_state=SYNC0; break;
                case VERSION: if(c==FRAME_VERSION) p_state=TYPE; else p_state=SYNC0; break;
                case TYPE: p_type = c; p_state=LEN0; break;
                case LEN0: p_len = (c << 8); p_state=LEN1; break;
                case LEN1: p_len |= c; 
                           if(p_len > MAX_PAYLOAD_LEN) p_state=SYNC0; 
                           else if(p_len > 0) { p_state=PAYLOAD; p_payload_idx=0; }
                           else p_state=CRC0; 
                           break;
                case PAYLOAD: p_payload[p_payload_idx++] = c; 
                              if(p_payload_idx == p_len) p_state=CRC0; 
                              break;
                case CRC0: p_crc_rx = (c << 8); p_state=CRC1; break;
                case CRC1: p_crc_rx |= c; process_frame(); p_state=SYNC0; break;
            }
        }
    }
}

void uart_protocol_init(void) {
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not ready");
        return;
    }
    uart_irq_callback_set(uart_dev, uart_callback);
    uart_irq_rx_enable(uart_dev);
    LOG_INF("UART Protocol RX listening");
}
