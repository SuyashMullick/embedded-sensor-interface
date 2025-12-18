#!/usr/bin/env python3

import os
import argparse
import struct
import binascii
import sys
import serial
import time

# Message types
MSG_GET_STATUS = 0x01
MSG_STATUS_RSP = 0x02
MSG_SET_PARAM = 0x03
MSG_GET_PARAM = 0x04
MSG_PARAM_RSP = 0x05
MSG_RESET_MOD = 0x06
MSG_ERROR_RSP = 0x07

PARAM_SENSOR_SAMPLE_RATE = 0x01
PARAM_STATUS_PERIOD_MS = 0x02
PARAM_SENSOR_ENABLE = 0x03

def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc = crc << 1
            crc &= 0xFFFF
    return crc

def build_frame(msg_type: int, payload: bytes = b'') -> bytes:
    version = 0x01
    length = len(payload)
    header = struct.pack(">BBBBH", 0xAA, 0x55, version, msg_type, length)
    crc_data = header[2:] + payload
    crc = crc16_ccitt(crc_data)
    frame = header + payload + struct.pack(">H", crc)
    return frame

def parse_frame(ser, timeout=1.0) -> tuple:
    ser.timeout = timeout
    # Sync
    while True:
        b = ser.read(1)
        if not b: return None, b''
        if b[0] == 0xAA:
            b2 = ser.read(1)
            if b2 and b2[0] == 0x55:
                break
    
    header = ser.read(4)
    if len(header) < 4: return None, b''
    version, msg_type, length = struct.unpack(">BBH", header)
    
    payload = ser.read(length) if length > 0 else b''
    if len(payload) < length: return None, b''
    
    crc_bytes = ser.read(2)
    if len(crc_bytes) < 2: return None, b''
    rx_crc = struct.unpack(">H", crc_bytes)[0]
    
    calc_crc = crc16_ccitt(header + payload)
    if calc_crc != rx_crc:
        print(f"CRC Error! Calc: {calc_crc:04X}, Rx: {rx_crc:04X}", file=sys.stderr)
        return None, b''
        
    return msg_type, payload

class EsimClient:
    def __init__(self, port):
        self.ser = serial.Serial(port, 115200, timeout=1.0)
        
    def get_status(self):
        self.ser.write(build_frame(MSG_GET_STATUS))
        ctype, payload = parse_frame(self.ser)
        if ctype == MSG_STATUS_RSP and len(payload) >= 18:
            state = payload[0]
            uptime = struct.unpack(">I", payload[1:5])[0]
            err_flags = struct.unpack(">I", payload[5:9])[0]
            rx_err = struct.unpack(">I", payload[9:13])[0]
            tx_err = struct.unpack(">I", payload[13:17])[0]
            sfault = payload[17]
            return {"state": state, "uptime": uptime, "err_flags": err_flags, 
                    "rx_err": rx_err, "tx_err": tx_err, "sensor_fault": sfault}
        return None

    def get_param(self, param_id):
        self.ser.write(build_frame(MSG_GET_PARAM, bytes([param_id])))
        ctype, payload = parse_frame(self.ser)
        if ctype == MSG_PARAM_RSP and len(payload) >= 1:
            if payload[0] != param_id: return None
            if param_id in (PARAM_SENSOR_SAMPLE_RATE, PARAM_STATUS_PERIOD_MS) and len(payload) == 3:
                return struct.unpack(">H", payload[1:3])[0]
            elif param_id == PARAM_SENSOR_ENABLE and len(payload) == 2:
                return payload[1] != 0
        return None

    def set_param(self, param_id, value):
        if param_id in (PARAM_SENSOR_SAMPLE_RATE, PARAM_STATUS_PERIOD_MS):
            payload = bytes([param_id]) + struct.pack(">H", value)
        else:
            payload = bytes([param_id, 1 if value else 0])
            
        self.ser.write(build_frame(MSG_SET_PARAM, payload))
        ctype, p = parse_frame(self.ser)
        if ctype == MSG_PARAM_RSP:
            return p[0] == 0
        elif ctype == MSG_ERROR_RSP:
            return False
        return False

    def reset_module(self):
        self.ser.write(build_frame(MSG_RESET_MOD))
        # Doesn't explicitly respond immediately depending on firmware reset timing

def main():
    parser = argparse.ArgumentParser(description="ESIM CLI")
    parser.add_argument("--pty", required=True, help="Path to PTY or serial port")
    parser.add_argument("cmd", choices=["status", "get_param", "set_param", "reset"])
    parser.add_argument("--param", type=int, help="Parameter ID")
    parser.add_argument("--value", type=int, help="Value to set")
    
    args = parser.parse_args()
    
    client = EsimClient(args.pty)
    
    if args.cmd == "status":
        st = client.get_status()
        print("Status:", st)
    elif args.cmd == "get_param":
        if args.param is None:
            print("Missing --param")
            return
        val = client.get_param(args.param)
        print(f"Param {args.param} = {val}")
    elif args.cmd == "set_param":
        if args.param is None or args.value is None:
            print("Missing --param or --value")
            return
        ok = client.set_param(args.param, args.value)
        print("Success" if ok else "Error")
    elif args.cmd == "reset":
        client.reset_module()
        print("Reset triggered")

if __name__ == "__main__":
    main()
