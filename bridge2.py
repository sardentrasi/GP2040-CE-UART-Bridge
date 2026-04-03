#!/usr/bin/env python3
"""
UART Bridge DEBUG MODE — Print packet yang dikirim ke RP2040.
Tampilkan hex dump + decoded values setiap packet.
"""

import serial
import struct
import evdev
from evdev import ecodes
import time

# --- KONFIGURASI ---
SERIAL_PORT = '/dev/ttyAMA0'
BAUD_RATE = 500000
DEVICE_PATH = '/dev/input/event2'

# Debug: print setiap N detik (supaya tidak banjir)
DEBUG_INTERVAL = 0.5  # detik
last_debug = 0

def debug_packet(packet):
    """Print decoded packet contents."""
    global last_debug
    now = time.time()
    if now - last_debug < DEBUG_INTERVAL:
        return
    last_debug = now

    btn_l = packet[1]
    btn_h = packet[2]
    dpad = packet[3]
    lx = struct.unpack_from('<h', packet, 4)[0]
    ly = struct.unpack_from('<h', packet, 6)[0]
    rx = struct.unpack_from('<h', packet, 8)[0]
    ry = struct.unpack_from('<h', packet, 10)[0]
    lt_val = packet[12]
    rt_val = packet[13]
    chk = packet[14]

    # Verify checksum
    calc_chk = 0
    for i in range(14):
        calc_chk ^= packet[i]
    chk_ok = "OK" if calc_chk == chk else "FAIL({:02X})".format(calc_chk)

    # Decode buttons
    btns = []
    if btn_l & 0x01: btns.append("X")
    if btn_l & 0x02: btns.append("A")
    if btn_l & 0x04: btns.append("B")
    if btn_l & 0x08: btns.append("Y")
    if btn_l & 0x10: btns.append("LB")
    if btn_l & 0x20: btns.append("RB")
    if btn_l & 0x40: btns.append("LTd")
    if btn_l & 0x80: btns.append("RTd")
    if btn_h & 0x01: btns.append("Home")
    if btn_h & 0x02: btns.append("Start")
    if btn_h & 0x04: btns.append("Back")
    if btn_h & 0x08: btns.append("L3")
    if btn_h & 0x10: btns.append("R3")

    # Decode dpad
    dpads = []
    if dpad & 0x01: dpads.append("U")
    if dpad & 0x02: dpads.append("R")
    if dpad & 0x04: dpads.append("D")
    if dpad & 0x08: dpads.append("L")

    btn_str = ",".join(btns) if btns else "none"
    dpad_str = "+".join(dpads) if dpads else "none"

    # Hex dump
    hex_str = " ".join("{:02X}".format(b) for b in packet)

    print("[PKT] {} | chk={} | btn=[{}] dpad=[{}] LX={:6d} LY={:6d} RX={:6d} RY={:6d} LT={:3d} RT={:3d}".format(
        hex_str, chk_ok, btn_str, dpad_str, lx, ly, rx, ry, lt_val, rt_val))

def main():
    try:
        dev = evdev.InputDevice(DEVICE_PATH)
    except Exception as e:
        print("[X] Gamepad gagal: " + str(e))
        return

    print("[V] Gamepad: {} ({})".format(dev.name, dev.path))

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
    except Exception as e:
        print("[X] UART gagal: " + str(e))
        return

    print("[!] Bridge DEBUG mode aktif. Ctrl+C untuk berhenti.\n")

    # --- State ---
    btn_l = 0; btn_h = 0; dpad_h = 0; dpad_v = 0
    lx = 0; ly = 0; rx = 0; ry = 0
    lt = 0; rt = 0

    last_send_time = 0

    try:
        for event in dev.read_loop():

            if event.type == ecodes.EV_KEY:
                v = event.value
                if   event.code == 308: btn_l = (btn_l | (1<<0)) if v else (btn_l & ~(1<<0))
                elif event.code == 304: btn_l = (btn_l | (1<<1)) if v else (btn_l & ~(1<<1))
                elif event.code == 305: btn_l = (btn_l | (1<<2)) if v else (btn_l & ~(1<<2))
                elif event.code == 307: btn_l = (btn_l | (1<<3)) if v else (btn_l & ~(1<<3))
                elif event.code == 310: btn_l = (btn_l | (1<<4)) if v else (btn_l & ~(1<<4))
                elif event.code == 311: btn_l = (btn_l | (1<<5)) if v else (btn_l & ~(1<<5))
                elif event.code == 312: btn_l = (btn_l | (1<<6)) if v else (btn_l & ~(1<<6))
                elif event.code == 313: btn_l = (btn_l | (1<<7)) if v else (btn_l & ~(1<<7))
                elif event.code == 316: btn_h = (btn_h | (1<<0)) if v else (btn_h & ~(1<<0))
                elif event.code == 315: btn_h = (btn_h | (1<<1)) if v else (btn_h & ~(1<<1))
                elif event.code == 314: btn_h = (btn_h | (1<<2)) if v else (btn_h & ~(1<<2))
                elif event.code == 317: btn_h = (btn_h | (1<<3)) if v else (btn_h & ~(1<<3))
                elif event.code == 318: btn_h = (btn_h | (1<<4)) if v else (btn_h & ~(1<<4))

            elif event.type == ecodes.EV_ABS:
                c = event.code; val = event.value
                if   c == 0: lx = max(-32768, min(32767, (val - 128) * 256))
                elif c == 1: ly = max(-32768, min(32767, (val - 128) * 256))
                elif c == 3: rx = max(-32768, min(32767, (val - 128) * 256))
                elif c == 4: ry = max(-32768, min(32767, (val - 128) * 256))
                elif c == 2: lt = max(0, min(255, val))
                elif c == 5: rt = max(0, min(255, val))
                elif c == 16:
                    if   val < 0: dpad_h = 0x08
                    elif val > 0: dpad_h = 0x02
                    else:         dpad_h = 0x00
                elif c == 17:
                    if   val < 0: dpad_v = 0x01
                    elif val > 0: dpad_v = 0x04
                    else:         dpad_v = 0x00
            elif event.type == ecodes.EV_SYN:
                # Build packet
                dpad = dpad_h | dpad_v
                packet = bytearray(15)
                packet[0] = 0xA5
                packet[1] = btn_l & 0xFF
                packet[2] = btn_h & 0xFF
                packet[3] = dpad & 0xFF
                struct.pack_into('<hhhh', packet, 4, lx, ly, rx, ry)
                packet[12] = lt & 0xFF
                packet[13] = rt & 0xFF
                chk = 0
                for i in range(14):
                    chk ^= packet[i]
                packet[14] = chk

                # DEBUG: Print decoded packet
                debug_packet(packet)

                # Send
                ser.write(packet)
                # Dihapus: ser.flush() — karena flush bersifat BLOCKING dan membuat evdev lag/delay saat memproses jutaan event sumbu analog.
                # Biarkan Linux OS yang mengatur buffer asinkron ke UART.

    except KeyboardInterrupt:
        print("\n[!] Dihentikan.")
    finally:
        ser.close()
        dev.close()

if __name__ == "__main__":
    main()
