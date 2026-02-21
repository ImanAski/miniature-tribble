#!/usr/bin/env python3
"""
hmic Host Tester
================
A command-line tool for testing the hmic Display Manager firmware from a PC.

Supports:
  - Serial port (UART / RS485 via USB adapter)
  - Loopback mode (no hardware required – for unit-testing the encoder/decoder)

Usage:
  python3 host_tester.py --port /dev/ttyUSB0 --baud 115200
  python3 host_tester.py --port /dev/ttyUSB0 --test all
  python3 host_tester.py --loopback   # offline frame encode/decode test
"""

import argparse
import serial
import struct
import time
import sys
import threading
from typing import Optional

# ── Protocol constants ─────────────────────────────────────────────────────

START_BYTE       = 0xAA
PROTOCOL_VERSION = 0x01

# Commands (host → device)
CMD_PING              = 0x01
CMD_GET_VERSION       = 0x02
CMD_RESET             = 0x03
CMD_ENTER_BOOTLOADER  = 0x04
CMD_SHOW_PAGE         = 0x10
CMD_SET_TEXT          = 0x20
CMD_SET_VALUE         = 0x21
CMD_SET_VISIBLE       = 0x22
CMD_SET_ENABLED       = 0x23

# Events (device → host)
EVT_BUTTON_PRESSED    = 0x80
EVT_SLIDER_CHANGED    = 0x81
EVT_PAGE_CHANGED      = 0x82
EVT_TOUCH_EVENT       = 0x83
EVT_ACK               = 0xF0
EVT_NACK              = 0xF1

CMD_NAMES = {
    CMD_PING: "CMD_PING",
    CMD_GET_VERSION: "CMD_GET_VERSION",
    CMD_RESET: "CMD_RESET",
    CMD_ENTER_BOOTLOADER: "CMD_ENTER_BOOTLOADER",
    CMD_SHOW_PAGE: "CMD_SHOW_PAGE",
    CMD_SET_TEXT: "CMD_SET_TEXT",
    CMD_SET_VALUE: "CMD_SET_VALUE",
    CMD_SET_VISIBLE: "CMD_SET_VISIBLE",
    CMD_SET_ENABLED: "CMD_SET_ENABLED",
    EVT_BUTTON_PRESSED: "EVT_BUTTON_PRESSED",
    EVT_SLIDER_CHANGED: "EVT_SLIDER_CHANGED",
    EVT_PAGE_CHANGED: "EVT_PAGE_CHANGED",
    EVT_TOUCH_EVENT: "EVT_TOUCH_EVENT",
    EVT_ACK: "EVT_ACK",
    EVT_NACK: "EVT_NACK",
}

# ── CRC16-CCITT ────────────────────────────────────────────────────────────

def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

# ── Frame encoder ───────────────────────────────────────────────────────────

def build_frame(cmd: int, seq: int, payload: bytes = b"") -> bytes:
    """Build a complete wire frame."""
    header = bytes([PROTOCOL_VERSION, cmd, seq & 0xFF, len(payload)])
    body   = header + payload
    crc    = crc16_ccitt(body)
    return bytes([START_BYTE]) + body + bytes([crc >> 8, crc & 0xFF])

# ── Frame decoder ────────────────────────────────────────────────────────────

class FrameDecoder:
    """Byte-at-a-time state machine decoder (mirrors dm_parser.c)."""

    WAIT_START = 0
    VERSION    = 1
    COMMAND    = 2
    SEQ_ID     = 3
    LENGTH     = 4
    PAYLOAD    = 5
    CRC_HIGH   = 6
    CRC_LOW    = 7

    def __init__(self, on_frame):
        self._state    = self.WAIT_START
        self._frame    = {}
        self._payload  = bytearray()
        self._crc_acc  = 0xFFFF
        self._crc_high = 0
        self._on_frame = on_frame

    def feed(self, byte: int):
        s = self._state
        if s == self.WAIT_START:
            if byte == START_BYTE:
                self._payload  = bytearray()
                self._crc_acc  = 0xFFFF
                self._state    = self.VERSION
        elif s == self.VERSION:
            self._frame["version"] = byte
            self._crc_acc = self._update_crc(self._crc_acc, byte)
            self._state = self.COMMAND
        elif s == self.COMMAND:
            self._frame["command"] = byte
            self._crc_acc = self._update_crc(self._crc_acc, byte)
            self._state = self.SEQ_ID
        elif s == self.SEQ_ID:
            self._frame["seq"] = byte
            self._crc_acc = self._update_crc(self._crc_acc, byte)
            self._state = self.LENGTH
        elif s == self.LENGTH:
            self._frame["length"] = byte
            self._crc_acc = self._update_crc(self._crc_acc, byte)
            self._state = self.PAYLOAD if byte > 0 else self.CRC_HIGH
        elif s == self.PAYLOAD:
            self._payload.append(byte)
            self._crc_acc = self._update_crc(self._crc_acc, byte)
            if len(self._payload) >= self._frame["length"]:
                self._state = self.CRC_HIGH
        elif s == self.CRC_HIGH:
            self._crc_high = byte
            self._state = self.CRC_LOW
        elif s == self.CRC_LOW:
            received_crc = (self._crc_high << 8) | byte
            if received_crc == self._crc_acc:
                self._frame["payload"] = bytes(self._payload)
                self._on_frame(dict(self._frame))
            else:
                print(f"[RX] CRC mismatch (got {received_crc:#06x}, expected {self._crc_acc:#06x})")
            self._state = self.WAIT_START

    @staticmethod
    def _update_crc(crc: int, byte: int) -> int:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else (crc << 1)
            crc &= 0xFFFF
        return crc

# ── Terminal helpers ────────────────────────────────────────────────────────

def fmt_frame(frame: dict) -> str:
    cmd_name = CMD_NAMES.get(frame["command"], f"0x{frame['command']:02X}")
    payload_hex = frame["payload"].hex(" ") if frame["payload"] else "(empty)"
    return (f"cmd={cmd_name:22s} seq={frame['seq']:3d} "
            f"payload=[{payload_hex}]")

def on_rx_frame(frame: dict):
    print(f"\033[32m[RX]\033[0m {fmt_frame(frame)}")

# ── Host session ────────────────────────────────────────────────────────────

class HostSession:
    def __init__(self, port: Optional[str], baud: int):
        self._seq     = 0
        self._decoder = FrameDecoder(on_rx_frame)
        self._ser     = None
        self._running = False

        if port:
            self._ser = serial.Serial(port, baud, timeout=0)
            print(f"[+] Connected to {port} @ {baud}")
        else:
            print("[+] Loopback mode (no serial port)")

    def send(self, cmd: int, payload: bytes = b"") -> int:
        """Send a command and return the sequence ID used."""
        seq   = self._seq
        frame = build_frame(cmd, seq, payload)
        print(f"\033[33m[TX]\033[0m cmd={CMD_NAMES.get(cmd, f'0x{cmd:02X}'):22s} "
              f"seq={seq:3d} payload=[{payload.hex(' ') if payload else '(empty)'}]")
        print(f"     raw: {frame.hex(' ')}")
        if self._ser:
            self._ser.write(frame)
        else:
            # Loopback: decode our own frame
            for b in frame:
                self._decoder.feed(b)
        self._seq = (self._seq + 1) & 0xFF
        return seq

    def start_rx(self):
        """Start background RX thread (serial mode only)."""
        if not self._ser:
            return
        self._running = True
        t = threading.Thread(target=self._rx_loop, daemon=True)
        t.start()

    def _rx_loop(self):
        while self._running:
            data = self._ser.read(64)
            for b in data:
                self._decoder.feed(b)
            time.sleep(0.001)

    def stop(self):
        self._running = False
        if self._ser:
            self._ser.close()

# ── Built-in tests ──────────────────────────────────────────────────────────

def test_ping(s: HostSession):
    print("\n--- PING ---")
    s.send(CMD_PING)
    time.sleep(0.2)

def test_get_version(s: HostSession):
    print("\n--- GET_VERSION ---")
    s.send(CMD_GET_VERSION)
    time.sleep(0.2)

def test_show_page(s: HostSession, page_id: int = 1):
    print(f"\n--- SHOW_PAGE {page_id} ---")
    s.send(CMD_SHOW_PAGE, bytes([page_id]))
    time.sleep(0.3)

def test_set_text(s: HostSession, widget: int = 0, text: str = "Hello from host!"):
    print(f"\n--- SET_TEXT widget={widget} text='{text}' ---")
    payload = bytes([widget]) + text.encode("utf-8")
    s.send(CMD_SET_TEXT, payload)
    time.sleep(0.2)

def test_set_value(s: HostSession, widget: int = 4, value: int = 75):
    print(f"\n--- SET_VALUE widget={widget} value={value} ---")
    payload = bytes([widget]) + struct.pack(">h", value)
    s.send(CMD_SET_VALUE, payload)
    time.sleep(0.2)

def test_crc_error(s: HostSession):
    """Send a frame with a deliberate CRC error – device must drop it gracefully."""
    print("\n--- CRC ERROR TEST (expect no crash, may get NACK) ---")
    frame = bytearray(build_frame(CMD_PING, 0xFF))
    frame[-1] ^= 0xFF  # corrupt last CRC byte
    print(f"     raw: {bytes(frame).hex(' ')}")
    if s._ser:
        s._ser.write(frame)
    time.sleep(0.2)

def run_all_tests(s: HostSession):
    test_ping(s)
    test_get_version(s)
    test_show_page(s, 0)
    test_show_page(s, 1)
    test_set_text(s, 0, "Remote text!")
    test_set_value(s, 4, 42)
    test_crc_error(s)
    print("\n[+] All tests sent.")

# ── CLI ─────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="hmic host tester")
    parser.add_argument("--port",     help="Serial port (e.g. /dev/ttyUSB0)")
    parser.add_argument("--baud",     type=int, default=115200)
    parser.add_argument("--loopback", action="store_true",
                        help="Run in loopback mode without serial hardware")
    parser.add_argument("--test",     choices=["all", "ping", "version",
                                               "page", "text", "value", "crc"],
                        help="Run a specific test suite")
    args = parser.parse_args()

    port = None if args.loopback else args.port
    if not args.loopback and not args.port:
        print("Specify --port /dev/ttyXXX or --loopback")
        sys.exit(1)

    session = HostSession(port, args.baud)
    session.start_rx()

    try:
        if args.test == "all":
            run_all_tests(session)
        elif args.test == "ping":
            test_ping(session)
        elif args.test == "version":
            test_get_version(session)
        elif args.test == "page":
            test_show_page(session)
        elif args.test == "text":
            test_set_text(session)
        elif args.test == "value":
            test_set_value(session)
        elif args.test == "crc":
            test_crc_error(session)
        else:
            # Interactive mode
            print("\nInteractive mode. Commands: ping, version, page <n>, "
                  "text <widget> <msg>, value <widget> <val>, crc, quit")
            while True:
                try:
                    line = input("> ").strip()
                except EOFError:
                    break
                if not line:
                    continue
                parts = line.split(None, 2)
                cmd = parts[0].lower()
                if cmd == "quit":
                    break
                elif cmd == "ping":
                    test_ping(session)
                elif cmd == "version":
                    test_get_version(session)
                elif cmd == "page" and len(parts) > 1:
                    test_show_page(session, int(parts[1]))
                elif cmd == "text" and len(parts) > 2:
                    test_set_text(session, int(parts[1]), parts[2])
                elif cmd == "value" and len(parts) > 2:
                    test_set_value(session, int(parts[1]), int(parts[2]))
                elif cmd == "crc":
                    test_crc_error(session)
                else:
                    print("Unknown command.")

        time.sleep(0.5)  # drain RX
    finally:
        session.stop()

if __name__ == "__main__":
    main()
