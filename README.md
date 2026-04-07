# GP2040-CE UART Input Bridge 🎮

This repository is a modified version of [GP2040-CE](https://gp2040-ce.info/) that includes a custom **UART Input Injection Addon**.

This feature allows a host device (like a Raspberry Pi running Linux) to read raw gamepad inputs using `evdev` and inject them directly into the RP2040 via high-speed UART (500kbps). The RP2040 then acts as an authentication pass-through or a native gamepad (PS4, PS5, Xbox 360, etc.) for your gaming console.

Originally designed for bridging **GameSir Nova 2 Lite** / Sony DualShock controllers via a headless Raspberry Pi to an Xbox console natively.

## ✨ Features

- **Pure UART Bridge Mode:** Physical GP2040 buttons are disabled (`NONE`) to prevent floating pin noise and phantom presses.
- **Two-Way Communication (Rumble Telemetry):** Implements a 4-byte return packet protocol (`0xBB` header) that proxies XInput Host Rumble/Haptic feedback commands back to the Python bridge to vibrate the physical gamepad.
- **High-Speed Serial:** Operates at 500,000 baud with a compact 15-byte packet structure.
- **Hardware FIFO Enabled:** The RP2040 utilizes its 32-byte UART FIFO, preventing buffer overflow or byte dropping even during intense analog stick sweeps.
- **Python EVDEV Bridge (`bridge2.py`):**
  - Asynchronous payload streaming directly from the Linux Kernel to UART (`ser.flush()` blocked).
  - Background `threading` listener perfectly isolates incoming serial Rumble pulses to Linux `evdev.ff` without stuttering the gamepad poll loop.
  - Handles 0-255 absolute axis range calibration efficiently to signed 16-bit RP2040 standards.
  - **No artificial software rate-limits:** Assures sub-millisecond latencies for analog ticks.
- **Web Configurator Remap & Automation:** The default Start (S2) hotkey has been moved to **GP04**. You can open the native interface manually or completely automate the RNDIS enumeration via the remote GP-Injector Dashboard utilizing an intelligent GPIO tie-down logic.
- **Debug Tools Included:** Python scripts for discovering and testing input parameters (`calibrate.py`), plus a built-in packet debugging mode.

## 🔌 Hardware Setup

You will need a Linux Host (e.g., Raspberry Pi 4/5) and an RP2040 (e.g., Waveshare RP2040 Zero).

### Wiring Configuration

_Note: A common Ground (GND) is **MANDATORY** for the UART to function correctly at 500kbps without data corruption._

| Raspberry Pi Pin (Host) | Waveshare RP2040 Zero | Description                                                  |
| :---------------------- | :-------------------- | :----------------------------------------------------------- |
| **Pin 8 (GPIO 14)**     | **GP01 (RX)**         | Pi transmits data (TX) to RP2040 receive (RX).               |
| **Pin 10 (GPIO 15)**    | **GP00 (TX)**         | Pi receives Rumble telemetry (RX) from RP2040 transmit (TX). |
| **Pin 6 / 39 (GND)**    | **GND**               | Common Ground.                                               |
| **Pin 12 (GPIO 18)**    | **GP04**              | Automates Web Configurator Bypass (Optional).                |

**(Web Configurator):** GP00 is consumed by UART. By default, GP-Injector dashboard manages the shorting of **GP04** to GND. You can also manually short it while plugging in USB to enter the GP2040-CE Web Configurator at `http://192.168.7.1`.

**(Optional LED Debug):** GP25 on the RP2040 will violently blink every time a valid authenticated packet is received from the Pi.

## 🛠️ Build and Compilation

The Python proto-generator scripts have been patched to support modern Linux environments using Python 3.12+.

1. Clone this repository to your Linux build host.
2. Ensure Pico SDK and CMake are properly installed.
3. Run the automated compile script:

```bash
./compile.sh
```

4. Find the resulting firmware in `/build/GP2040-CE_0.7.12_WaveshareZero.uf2`.
5. Flash it to your RP2040 (Hold `BOOTSEL` button while plugging in the USB).

## 🚀 Running the Python Bridge

### Prerequisites

Make sure your USB controller or Bluetooth Dongle is plugged into your Raspberry Pi.
Install the requisite dependencies via pip (Python 3):

```bash
pip3 install evdev pyserial
```

### 1. Identify and Calibrate your Controller

If you're using a GameSir/Sony controller, you can check what axes are mapped.

```bash
python3 calibrate.py
```

This utility will bypass the OS noise filters and print out exact `[BTN]` and `[AXS]` changes live.

### 2. Start the Injection Bridge

You have two options for running the host bridge on your Raspberry Pi:

**Option A: Simple Bridge Script (`bridge2.py`)**
A lightweight, terminal-only script for quickly streaming inputs directly to the UART. It auto-detects the gamepad, compiles the custom 15-byte protocol, and performs XOR checksums.

```bash
python3 bridge2.py
```

**Option B: Advanced GP-Injector (Recommended)**
A full-featured engine with a responsive Web GUI. It provides a local dashboard at `http://localhost:8080/` where you can graphically manage Profiles, Button Remapping, Anti-Recoil, Turbo, and remotely trigger the Web Config bypass.

👉 **[Download & Set Up GP-Injector](https://github.com/sardentrasi/gp-injector)**

_(See the `README.md` in that repository for automated service installation instructions)._

If you experience any delay or drift on Xbox/Gamepad Testers, double-check your common ground wiring or review the noise ranges in `calibrate.py`.

## 📦 Packet Protocol Reference

The script sends a fixed 15-byte Little-Endian packet down the UART line:

- `[0]` Header **(0xA5)**
- `[1]` btnL (Bitmask: X \| A \| B \| Y \| LB \| RB \| LT_btn \| RT_btn)
- `[2]` btnH (Bitmask: Home \| Start \| Back \| L3 \| R3)
- `[3]` dpad (Bitmask: Up \| Right \| Down \| Left)
- `[4-5]` Left Stick X (int16_t)
- `[6-7]` Left Stick Y (int16_t)
- `[8-9]` Right Stick X (int16_t)
- `[10-11]` Right Stick Y (int16_t)
- `[12]` Left Trigger (uint8_t)
- `[13]` Right Trigger (uint8_t)
- `[14]` Checksum (XOR `packet[0]` ... `packet[13]`)

### Telemetry / Rumble Packet `(RP2040 -> Raspberry Pi)`

When XInput host commands Haptic Feedback (e.g. from an Xbox), the RP2040 passes a 4-byte protocol immediately to the Pi:

- `[0]` Header **(0xBB)**
- `[1]` Left Motor Intensity (`0x00`-`0xFF`)
- `[2]` Right Motor Intensity (`0x00`-`0xFF`)
- `[3]` Checksum (XOR `packet[0]` ... `packet[2]`)

---

_Credit to the OpenStickCommunity for the foundational GP2040-CE architecture._
