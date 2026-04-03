#!/usr/bin/env python3
"""
Kalibrasi Gamepad v3 — PAKSA event2, TANPA FILTER sama sekali.
"""

import evdev
from evdev import ecodes

# PAKSA device path — ubah kalau beda
DEVICE_PATH = '/dev/input/event2'

ABS_LABELS = {
    0: "LX", 1: "LY", 2: "LT_ana", 3: "RX", 4: "RY", 5: "RT_ana",
    16: "DPad_H", 17: "DPad_V",
}

BTN_LABELS = {
    304: "A", 305: "B", 307: "Y", 308: "X",
    310: "LB", 311: "RB", 312: "LT_dig", 313: "RT_dig",
    314: "Back", 315: "Start", 316: "Home", 317: "L3", 318: "R3",
}

def main():
    dev = evdev.InputDevice(DEVICE_PATH)
    print("[V] Device: {} ({})".format(dev.name, dev.path))

    # Show axis capabilities
    caps = dev.capabilities(verbose=False)
    if ecodes.EV_ABS in caps:
        print("\n--- AXIS CAPABILITIES ---")
        for item in caps[ecodes.EV_ABS]:
            code = item[0] if isinstance(item, tuple) else item
            info = item[1] if isinstance(item, tuple) else None
            label = ABS_LABELS.get(code, "ABS_{}".format(code))
            if info:
                print("  {:2d} {:8s} | min={:6d} max={:6d} flat={} fuzz={}".format(
                    code, label, info.min, info.max, info.flat, info.fuzz))

    print("\n--- MULAI (Ctrl+C stop) ---\n")

    try:
        for event in dev.read_loop():
            if event.type == ecodes.EV_KEY:
                label = BTN_LABELS.get(event.code, "btn_{}".format(event.code))
                print("[BTN] {:8s} (code={:3d}) = {}".format(
                    label, event.code, "TEKAN" if event.value else "LEPAS"))

            elif event.type == ecodes.EV_ABS:
                label = ABS_LABELS.get(event.code, "abs_{}".format(event.code))
                print("[AXS] {:8s} (code={:2d}) = {:6d}".format(
                    label, event.code, event.value))

    except KeyboardInterrupt:
        print("\nSelesai.")
    finally:
        dev.close()

if __name__ == "__main__":
    main()
