#ifndef VARS_H
#define VARS_H

#define FALSE 0
#define TRUE 1

enum packet_state
{
    START,
    F,
    A,
    C,
    BCC,
    STOP,
    DATA
};

#define FLAG        0x7E
#define ESC         0x7D
#define TX_ADD      0x03
#define RX_ADD      0x01
#define INF_FRAME_0 0x00
#define INF_FRAME_1 0x40
#define SET         0x03
#define UA          0x07
#define RR0         0x05
#define RR1         0x85
#define REJ0        0x01
#define REJ1        0x81
#define DISC        0x0B
//      BCC1        A XOR C
//      BCC2        D1 XOR D2 XOR D3 … XOR DN

#endif