/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Waveshare Zero UART Bridge"

// ============================================================================
// UART Input Injection Configuration
// Raspberry Pi → RP2040 bridge mode via UART0 on GP00/GP01
// ============================================================================
#define UART_INPUT_ENABLED   1
#define UART_INPUT_PORT      uart0
#define UART_INPUT_TX_PIN    0       // GP00 (TX)
#define UART_INPUT_RX_PIN    1       // GP01 (RX)
#define UART_INPUT_BAUDRATE  500000  // 500kbps

// ============================================================================
// Main pin mapping Configuration
// GP00 and GP01 are reserved for UART — set to NONE
// ============================================================================
// ALL PINS NONE — Pure UART bridge mode, no physical buttons
//                                                  // GP2040 | Function                                      |
#define GPIO_PIN_00 GpioAction::NONE                // Reserved for UART TX                                   |
#define GPIO_PIN_01 GpioAction::NONE                // Reserved for UART RX                                   |
#define GPIO_PIN_02 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_03 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_04 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_05 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_06 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_07 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_08 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_09 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_10 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_11 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_12 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_13 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_14 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_15 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_26 GpioAction::NONE                // Unused — bridge mode                                   |
#define GPIO_PIN_27 GpioAction::NONE                // Unused — bridge mode                                   |

// Keyboard Mapping Configuration
//                                            // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define KEY_DPAD_UP     HID_KEY_ARROW_UP      // UP     | UP     | UP      | UP       | UP     | UP     |
#define KEY_DPAD_DOWN   HID_KEY_ARROW_DOWN    // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define KEY_DPAD_RIGHT  HID_KEY_ARROW_RIGHT   // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define KEY_DPAD_LEFT   HID_KEY_ARROW_LEFT    // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define KEY_BUTTON_B1   HID_KEY_SHIFT_LEFT    // B1     | A      | B       | Cross    | 2      | K1     |
#define KEY_BUTTON_B2   HID_KEY_Z             // B2     | B      | A       | Circle   | 3      | K2     |
#define KEY_BUTTON_R2   HID_KEY_X             // R2     | RT     | ZR      | R2       | 8      | K3     |
#define KEY_BUTTON_L2   HID_KEY_V             // L2     | LT     | ZL      | L2       | 7      | K4     |
#define KEY_BUTTON_B3   HID_KEY_CONTROL_LEFT  // B3     | X      | Y       | Square   | 1      | P1     |
#define KEY_BUTTON_B4   HID_KEY_ALT_LEFT      // B4     | Y      | X       | Triangle | 4      | P2     |
#define KEY_BUTTON_R1   HID_KEY_SPACE         // R1     | RB     | R       | R1       | 6      | P3     |
#define KEY_BUTTON_L1   HID_KEY_C             // L1     | LB     | L       | L1       | 5      | P4     |
#define KEY_BUTTON_S1   HID_KEY_5             // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define KEY_BUTTON_S2   HID_KEY_1             // S2     | Start  | Plus    | Start    | 10     | Start  |
#define KEY_BUTTON_L3   HID_KEY_EQUAL         // L3     | LS     | LS      | L3       | 11     | LS     |
#define KEY_BUTTON_R3   HID_KEY_MINUS         // R3     | RS     | RS      | R3       | 12     | RS     |
#define KEY_BUTTON_A1   HID_KEY_9             // A1     | Guide  | Home    | PS       | 13     | ~      |
#define KEY_BUTTON_A2   HID_KEY_F2            // A2     | ~      | Capture | ~        | 14     | ~      |
#define KEY_BUTTON_FN   -1                    // Hotkey Function                                        |

#endif