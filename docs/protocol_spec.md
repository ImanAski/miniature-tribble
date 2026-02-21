# Protocol Specification – hmic Display Manager v1

**Protocol Version:** `0x01`  
**Interface:** UART (RS485) or USB-CDC at 115200 baud  
**Byte order:** Big-endian for multi-byte integers

---

## 1. Frame Structure

Every message – both host→device commands and device→host events – uses the same frame format.

| Byte(s) | Field          | Description                                      |
|---------|----------------|--------------------------------------------------|
| 0       | `START`        | Always `0xAA`. Used for re-synchronisation.      |
| 1       | `VERSION`      | Protocol version (`0x01`).                       |
| 2       | `COMMAND`      | Command or event ID (see tables below).          |
| 3       | `SEQ_ID`       | Sequence number 0–255 (wraps). Device echoes this in ACK/NACK. |
| 4       | `PAYLOAD_LEN`  | Number of payload bytes (0–`DM_MAX_PAYLOAD`).    |
| 5..N    | `PAYLOAD`      | Command-specific data (may be empty).            |
| N+1     | `CRC_HIGH`     | CRC16-CCITT MSB.                                 |
| N+2     | `CRC_LOW`      | CRC16-CCITT LSB.                                 |

### CRC Calculation

- **Algorithm:** CRC16-CCITT (polynomial `0x1021`, initial value `0xFFFF`)
- **Input:** bytes `VERSION` through end of `PAYLOAD` (i.e. `frame[1..N]`)
- **No output XOR**
- The `START` byte is **not** included in the CRC.

---

## 2. Command IDs (Host → Device)

### 2.1 System Commands

| ID     | Name                  | Payload             | Response         |
|--------|-----------------------|---------------------|------------------|
| `0x01` | `CMD_PING`            | _(empty)_           | `EVT_ACK`        |
| `0x02` | `CMD_GET_VERSION`     | _(empty)_           | `EVT_ACK` + `[major, minor, patch]` |
| `0x03` | `CMD_RESET`           | _(empty)_           | `EVT_ACK` then reboot |
| `0x04` | `CMD_ENTER_BOOTLOADER`| _(empty)_           | `EVT_NACK` (unless supported by board) |

### 2.2 Navigation

| ID     | Name           | Payload        | Response  |
|--------|----------------|----------------|-----------|
| `0x10` | `CMD_SHOW_PAGE`| `[page_id:u8]` | `EVT_ACK` + `EVT_PAGE_CHANGED` |

### 2.3 UI Manipulation

| ID     | Name              | Payload                          | Response  |
|--------|-------------------|----------------------------------|-----------|
| `0x20` | `CMD_SET_TEXT`    | `[widget_idx:u8][text:str]`      | `EVT_ACK` |
| `0x21` | `CMD_SET_VALUE`   | `[widget_idx:u8][value:i16 BE]`  | `EVT_ACK` |
| `0x22` | `CMD_SET_VISIBLE` | `[widget_idx:u8][visible:u8]`    | `EVT_ACK` |
| `0x23` | `CMD_SET_ENABLED` | `[widget_idx:u8][enabled:u8]`    | `EVT_ACK` |

**Notes:**
- `text` is a raw UTF-8 string, **not** null-terminated in the frame (length comes from `PAYLOAD_LEN`).
- `visible` / `enabled`: `0` = false, non-zero = true.
- `widget_idx` is a compile-time index into the UI widget table defined in `app/ui/ui_pages.c`.

---

## 3. Event IDs (Device → Host)

| ID     | Name                  | Payload                                   |
|--------|-----------------------|-------------------------------------------|
| `0x80` | `EVT_BUTTON_PRESSED`  | `[widget_idx:u8]`                         |
| `0x81` | `EVT_SLIDER_CHANGED`  | `[widget_idx:u8][value:i16 BE]`           |
| `0x82` | `EVT_PAGE_CHANGED`    | `[page_id:u8]`                            |
| `0x83` | `EVT_TOUCH_EVENT`     | `[x:i16 BE][y:i16 BE]`                   |
| `0xF0` | `EVT_ACK`             | `[echo_seq:u8][optional data...]`         |
| `0xF1` | `EVT_NACK`            | _(empty)_                                 |

---

## 4. Sequence ID Behaviour

- The **host** increments `SEQ_ID` with each new command (mod 256).
- The **device** echoes the same `SEQ_ID` in its `EVT_ACK` or `EVT_NACK`.
- Device-originated events (buttons, sliders, etc.) use an **independent** device-side counter that starts at 0 and increments for each unsolicited event.

---

## 5. Error Handling & Re-synchronisation

| Condition            | Device Action                                   |
|----------------------|-------------------------------------------------|
| CRC mismatch         | Frame silently dropped, parser resets to `WAIT_START` |
| Unknown `COMMAND`    | `EVT_NACK` sent with the offending `SEQ_ID`     |
| `PAYLOAD_LEN` > max  | Frame dropped, parser resets                    |
| Partial frame / timeout | Parser stays in current state; re-syncs on next `0xAA` |

The presence of a unique start byte (`0xAA`) and per-frame CRC provides two independent layers of protection against stream corruption.

---

## 6. Example Transactions

### 6.1 PING / PONG

```
Host → Device:  AA 01 01 01 00  [CRC_H CRC_L]
Device → Host:  AA 01 F0 01 00  [CRC_H CRC_L]
```
_(CMD_PING with SEQ=1, ACK echoes SEQ=1, no payload)_

### 6.2 Show Page 1

```
Host → Device:  AA 01 10 02 01 01  [CRC_H CRC_L]
Device → Host:  AA 01 F0 02 00     [CRC_H CRC_L]   ← ACK
Device → Host:  AA 01 82 00 01 01  [CRC_H CRC_L]   ← EVT_PAGE_CHANGED page=1
```

### 6.3 Set label text "Hello"

```
Host → Device:  AA 01 20 03 06 00 48 65 6C 6C 6F  [CRC_H CRC_L]
                                   ^widget 0  ^ "Hello"
Device → Host:  AA 01 F0 03 00  [CRC_H CRC_L]  ← ACK
```

---

## 7. Configuration Constants

| Constant            | Default | Description                        |
|---------------------|---------|------------------------------------|
| `DM_MAX_PAYLOAD`    | 128     | Max payload bytes per frame        |
| `DM_PROTOCOL_VERSION` | 1     | Wire version number                |
| `DM_MAX_WIDGET_ID`  | 32      | Max widget ID string length        |
| `DM_MAX_TEXT_LEN`   | 64      | Max text payload string length     |
| `DM_MAX_PAGES`      | 8       | Max number of UI pages             |

All constants are defined in `core/dm_config.h` and can be overridden via CMake `target_compile_definitions`.
