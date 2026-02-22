# hmic – Portable Touch TFT Display Manager

This is a custom Display Manager, written with a PAL architecture in mind.
This software is meant to be used for implemented boards.

---

## Architecture

```
┌──────────────────────────────────┐
│         Board Layer (HAL)        │  boards/rp2040 | boards/esp32 | boards/sim
├──────────────────────────────────┤
│   Platform Abstraction Layer     │  dm_platform_t  (injected vtable)
├──────────────────────────────────┤
│   Core Display Manager Library   │  core/  – NO board dependencies
├──────────────────────────────────┤
│        Protocol Layer            │  core/dm_parser + dm_protocol + dm_packet
├──────────────────────────────────┤
│        Application Binder        │  app/dm_binder
├──────────────────────────────────┤
│         LVGL UI Layer            │  app/ui/
└──────────────────────────────────┘
```

## Project Layout

```
hmic/
├── CMakeLists.txt          ← top-level (board selected via -DHMIC_BOARD=xxx)
├── core/                   ← portable library (no board deps)
│   ├── dm_config.h         ← compile-time knobs
│   ├── dm_platform.h       ← HAL vtable definition
│   ├── dm_core.{h,c}       ← public API: dm_init / dm_receive_byte / dm_process
│   ├── dm_parser.{h,c}     ← frame state-machine parser + CRC validation
│   ├── dm_protocol.{h,c}   ← command IDs, dispatcher, weak handler stubs
│   ├── dm_packet.{h,c}     ← packet encoder, event helpers
│   └── crc16.{h,c}         ← CRC16-CCITT (no XOR, seed 0xFFFF)
├── app/                    ← application binder + LVGL pages
│   ├── dm_binder.{h,c}     ← overrides weak handlers, delegates to UI layer
│   └── ui/
│       ├── ui_pages.h
│       └── ui_pages.c      ← two demo pages, index-based widget table
├── boards/
│   ├── rp2040/             ← Raspberry Pi Pico (pico-sdk, UART0)
│   ├── esp32/              ← Espressif ESP32-S3 (ESP-IDF, UART1)
│   └── sim/                ← POSIX desktop simulator (serial or loopback)
├── docs/
│   └── protocol_spec.md    ← full wire protocol documentation
└── tools/
    └── host_tester.py      ← Python host test script
```

---

## Building

### RP2040 (Pico)

```bash
# PICO_SDK_PATH must be set
cmake -B build-rp2040 -DHMIC_BOARD=rp2040
cmake --build build-rp2040
```

### ESP32-S3

```bash
# Requires ESP-IDF
cmake -B build-esp32 -DHMIC_BOARD=esp32 -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32s3.cmake
cmake --build build-esp32
```

### STM Family

>[!WIP] This is a Work in progress

### PC Simulator (POSIX/SDL)

```bash
cmake -B build-sim -DHMIC_BOARD=sim
cmake --build build-sim
./build-sim/hmic_sim                      # loopback / no serial
./build-sim/hmic_sim --port /dev/pts/3    # with a serial pty
```

---

## Protocol Quick Reference

| Byte(s) | Field             | Value / Notes          |
|---------|-------------------|------------------------|
| 0       | Start Byte        | `0xAA`                 |
| 1       | Version           | `0x01`                 |
| 2       | Command           | See command table below |
| 3       | Sequence ID       | 0–255 (wraps)          |
| 4       | Payload Length    | 0–`DM_MAX_PAYLOAD`     |
| 5..N    | Payload           | Command-specific       |
| N+1..N+2| CRC16-CCITT       | Big-endian, no XOR     |

### Commands (Host → Device)

| ID     | Name              |
|--------|-------------------|
| `0x01` | `CMD_PING`        |
| `0x02` | `CMD_GET_VERSION` |
| `0x03` | `CMD_RESET`       |
| `0x04` | `CMD_ENTER_BOOTLOADER` |
| `0x10` | `CMD_SHOW_PAGE`   |
| `0x20` | `CMD_SET_TEXT`    |
| `0x21` | `CMD_SET_VALUE`   |
| `0x22` | `CMD_SET_VISIBLE` |
| `0x23` | `CMD_SET_ENABLED` |

### Events (Device → Host)

| ID     | Name                 |
|--------|----------------------|
| `0x80` | `EVT_BUTTON_PRESSED` |
| `0x81` | `EVT_SLIDER_CHANGED` |
| `0x82` | `EVT_PAGE_CHANGED`   |
| `0x83` | `EVT_TOUCH_EVENT`    |
| `0xF0` | `EVT_ACK`            |
| `0xF1` | `EVT_NACK`           |

Full specification: [`docs/protocol_spec.md`](docs/protocol_spec.md)

---

## Host Tester

```bash
# Install dependency
pip install pyserial

# Run against hardware
python3 tools/host_tester.py --port /dev/ttyUSB0 --baud 115200 --test all

# Offline loopback (no hardware required)
python3 tools/host_tester.py --loopback --test all

# Interactive console
python3 tools/host_tester.py --loopback
```

---

## How to write driver for new boards?

Well you can start write respected files in the [`boards`](boards) directory.

1. Implement `dm_platform_t`:

```c
static dm_platform_t my_platform = {
    .write_bytes = my_uart_write,
    .millis      = my_get_ms,
    .log         = my_log_str,
};
```

1. In `main()` / your primary task:

```c
dm_init(&my_platform);
dm_binder_init(&my_platform);

while (1) {
    uint8_t b;
    while (uart_rx(&b)) dm_receive_byte(b);
    dm_process();
    lv_timer_handler();
    // feed watchdog
}
```

1. Add `boards/<your_board>/CMakeLists.txt` and link `hmic_core` + `hmic_app`.

## tools

You can use the scripts and tools in the [`tools`](tools) directory

- **[`host_tester.py`](tools/host_tester.py)**: python script for testing the
packet handler in the core
- **[`res_calc.py`](tools/res_calc.py)**: python script for calculating the
needed resource for the project
- **[`res_calc.html`](tools/res_calc.html)**: html page for calculating the
needed resource for the project
