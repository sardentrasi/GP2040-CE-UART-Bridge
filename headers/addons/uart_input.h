#ifndef _UARTInput_H
#define _UARTInput_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"

// UART Input Addon — receives 15-byte gamepad packets from an external
// controller (e.g. Raspberry Pi) over UART and injects them into the
// GP2040-CE gamepad state.

#ifndef UART_INPUT_ENABLED
#define UART_INPUT_ENABLED 0
#endif

#ifndef UART_INPUT_PORT
#define UART_INPUT_PORT uart0
#endif

#ifndef UART_INPUT_TX_PIN
#define UART_INPUT_TX_PIN -1
#endif

#ifndef UART_INPUT_RX_PIN
#define UART_INPUT_RX_PIN -1
#endif

#ifndef UART_INPUT_BAUDRATE
#define UART_INPUT_BAUDRATE 500000
#endif

// Protocol constants
#define UART_PACKET_HEADER  0xA5
#define UART_PACKET_SIZE    15

// Module Name
#define UARTInputName "UARTInput"

class UARTInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return UARTInputName; }

private:
    // Packet receive buffer and state machine
    uint8_t rxBuffer[UART_PACKET_SIZE];
    uint8_t rxIndex;

    // Parsed gamepad values (applied to state each process() cycle)
    uint32_t buttons;
    uint8_t  dpad;
    uint16_t lx;
    uint16_t ly;
    uint16_t rx;
    uint16_t ry;
    uint8_t  lt;
    uint8_t  rt;

    // Track whether we've received at least one valid packet
    bool hasValidPacket;

    // Debug
    bool debugLedState;
    uint32_t validPacketCount;
    uint32_t badChecksumCount;

    // Read all available bytes from UART and parse complete packets
    void readUART();

    // Parse a complete 15-byte packet into internal state
    void parsePacket();
};

#endif  // _UARTInput_H
