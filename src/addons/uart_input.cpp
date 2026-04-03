/*
 * UART Input Addon for GP2040-CE
 *
 * Receives 15-byte gamepad state packets over UART from an external
 * source (e.g. Raspberry Pi) and injects them directly into the
 * GP2040-CE gamepad state. Designed for use as a transparent HID
 * bridge — the RP2040 appears as a native controller while the Pi
 * provides all input data.
 *
 * Protocol (15 bytes, little-endian):
 *   [0]     Header      0xA5
 *   [1]     btnL        X|A|B|Y|LB|RB|LT_btn|RT_btn
 *   [2]     btnH        Home|Start|Back|L3|R3
 *   [3]     dpad        Up|Right|Down|Left
 *   [4-5]   LX          int16_t LE
 *   [6-7]   LY          int16_t LE
 *   [8-9]   RX          int16_t LE
 *   [10-11] RY          int16_t LE
 *   [12]    LT          uint8_t (0-255)
 *   [13]    RT          uint8_t (0-255)
 *   [14]    Checksum    XOR of bytes 0..13
 */

#include "addons/uart_input.h"
#include "storagemanager.h"
#include "helper.h"

#include "hardware/uart.h"
#include "hardware/gpio.h"

// Debug: toggle GP25 (onboard LED on many RP2040 boards) on valid packet
#define UART_DEBUG_LED_PIN 25

bool UARTInput::available() {
    return UART_INPUT_ENABLED &&
           isValidPin(UART_INPUT_TX_PIN) &&
           isValidPin(UART_INPUT_RX_PIN);
}

void UARTInput::setup() {
    // Initialize packet state machine
    rxIndex = 0;
    hasValidPacket = false;
    debugLedState = false;
    validPacketCount = 0;
    badChecksumCount = 0;

    // Reset parsed values to neutral
    buttons = 0;
    dpad = 0;
    lx = GAMEPAD_JOYSTICK_MID;
    ly = GAMEPAD_JOYSTICK_MID;
    rx = GAMEPAD_JOYSTICK_MID;
    ry = GAMEPAD_JOYSTICK_MID;
    lt = 0;
    rt = 0;

    // Debug LED
    gpio_init(UART_DEBUG_LED_PIN);
    gpio_set_dir(UART_DEBUG_LED_PIN, GPIO_OUT);
    gpio_put(UART_DEBUG_LED_PIN, 0);

    // Initialize UART hardware
    uart_init(UART_INPUT_PORT, UART_INPUT_BAUDRATE);

    // Set GPIO function to UART
    gpio_set_function(UART_INPUT_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_INPUT_RX_PIN, GPIO_FUNC_UART);

    // Enable UART FIFO (32 bytes) so we don't drop bytes during bursts of 15-byte packets
    uart_set_fifo_enabled(UART_INPUT_PORT, true);

    // Set format: 8N1
    uart_set_format(UART_INPUT_PORT, 8, 1, UART_PARITY_NONE);

    // Enable hardware flow control? No — simple 2-wire connection
    uart_set_hw_flow(UART_INPUT_PORT, false, false);
}

void UARTInput::readUART() {
    // Drain all available bytes from UART, parsing packets as we go.
    // This is non-blocking — uart_is_readable() returns immediately.
    while (uart_is_readable(UART_INPUT_PORT)) {
        uint8_t byte = uart_getc(UART_INPUT_PORT);

        if (rxIndex == 0) {
            // Waiting for header
            if (byte == UART_PACKET_HEADER) {
                rxBuffer[0] = byte;
                rxIndex = 1;
            }
            // else: discard, keep looking for header
        } else {
            rxBuffer[rxIndex] = byte;
            rxIndex++;

            if (rxIndex >= UART_PACKET_SIZE) {
                // Full packet received — validate checksum
                uint8_t checksum = 0;
                for (int i = 0; i < UART_PACKET_SIZE - 1; i++) {
                    checksum ^= rxBuffer[i];
                }

                if (checksum == rxBuffer[UART_PACKET_SIZE - 1]) {
                    // Valid packet — parse it
                    parsePacket();
                    hasValidPacket = true;
                    validPacketCount++;

                    // Toggle debug LED on every valid packet
                    debugLedState = !debugLedState;
                    gpio_put(UART_DEBUG_LED_PIN, debugLedState);
                } else {
                    badChecksumCount++;
                }

                // Reset state machine for next packet
                rxIndex = 0;
            }
        }
    }
}

void UARTInput::parsePacket() {
    uint8_t btnL = rxBuffer[1];
    uint8_t btnH = rxBuffer[2];
    uint8_t dpadByte = rxBuffer[3];

    // --- Parse buttons (btnL byte) ---
    // Bit 0: X       → B3  (Square/X)
    // Bit 1: A       → B1  (Cross/A)
    // Bit 2: B       → B2  (Circle/B)
    // Bit 3: Y       → B4  (Triangle/Y)
    // Bit 4: LB      → L1
    // Bit 5: RB      → R1
    // Bit 6: LT btn  → L2
    // Bit 7: RT btn  → R2
    buttons = 0;
    if (btnL & 0x01) buttons |= GAMEPAD_MASK_B3;   // X
    if (btnL & 0x02) buttons |= GAMEPAD_MASK_B1;   // A
    if (btnL & 0x04) buttons |= GAMEPAD_MASK_B2;   // B
    if (btnL & 0x08) buttons |= GAMEPAD_MASK_B4;   // Y
    if (btnL & 0x10) buttons |= GAMEPAD_MASK_L1;   // LB
    if (btnL & 0x20) buttons |= GAMEPAD_MASK_R1;   // RB
    if (btnL & 0x40) buttons |= GAMEPAD_MASK_L2;   // LT button
    if (btnL & 0x80) buttons |= GAMEPAD_MASK_R2;   // RT button

    // --- Parse buttons (btnH byte) ---
    // Bit 0: Home/Guide → A1
    // Bit 1: Start      → S2
    // Bit 2: Back       → S1
    // Bit 3: L3         → L3
    // Bit 4: R3         → R3
    if (btnH & 0x01) buttons |= GAMEPAD_MASK_A1;   // Home/Guide
    if (btnH & 0x02) buttons |= GAMEPAD_MASK_S2;   // Start
    if (btnH & 0x04) buttons |= GAMEPAD_MASK_S1;   // Back
    if (btnH & 0x08) buttons |= GAMEPAD_MASK_L3;   // L3
    if (btnH & 0x10) buttons |= GAMEPAD_MASK_R3;   // R3

    // --- Parse D-Pad ---
    // Bit 0: Up    → GAMEPAD_MASK_UP
    // Bit 1: Right → GAMEPAD_MASK_RIGHT
    // Bit 2: Down  → GAMEPAD_MASK_DOWN
    // Bit 3: Left  → GAMEPAD_MASK_LEFT
    dpad = 0;
    if (dpadByte & 0x01) dpad |= GAMEPAD_MASK_UP;
    if (dpadByte & 0x02) dpad |= GAMEPAD_MASK_RIGHT;
    if (dpadByte & 0x04) dpad |= GAMEPAD_MASK_DOWN;
    if (dpadByte & 0x08) dpad |= GAMEPAD_MASK_LEFT;

    // --- Parse Analog Sticks (signed int16 LE → unsigned uint16) ---
    // Conversion: unsigned_value = signed_value + 32768
    int16_t rawLX = (int16_t)(rxBuffer[4] | (rxBuffer[5] << 8));
    int16_t rawLY = (int16_t)(rxBuffer[6] | (rxBuffer[7] << 8));
    int16_t rawRX = (int16_t)(rxBuffer[8] | (rxBuffer[9] << 8));
    int16_t rawRY = (int16_t)(rxBuffer[10] | (rxBuffer[11] << 8));

    lx = (uint16_t)((int32_t)rawLX + 32768);
    ly = (uint16_t)((int32_t)rawLY + 32768);
    rx = (uint16_t)((int32_t)rawRX + 32768);
    ry = (uint16_t)((int32_t)rawRY + 32768);

    // --- Parse Analog Triggers (already uint8_t, direct copy) ---
    lt = rxBuffer[12];
    rt = rxBuffer[13];
}

void UARTInput::process() {
    // Read and parse any available UART data (non-blocking)
    readUART();

    // Only inject state if we've received at least one valid packet
    if (!hasValidPacket) return;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    // Full override — UART is the sole input source for this bridge device
    gamepad->state.buttons = buttons;
    gamepad->state.dpad    = dpad;
    gamepad->state.lx      = lx;
    gamepad->state.ly      = ly;
    gamepad->state.rx      = rx;
    gamepad->state.ry      = ry;
    gamepad->state.lt      = lt;
    gamepad->state.rt      = rt;

    // Signal analog stick and trigger support
    gamepad->hasLeftAnalogStick  = true;
    gamepad->hasRightAnalogStick = true;
    gamepad->hasAnalogTriggers   = true;
}
