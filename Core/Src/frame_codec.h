/*
 * frame_codec.h
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */

#ifndef FRAME_CODEC_H
#define FRAME_CODEC_H

/**
 * frame_codec.h
 *
 * Module thuan C, khong phu thuoc FreeRTOS / Xilinx BSP, dung de dong/mo
 * khung tin giao tiep giua firmware (Zynq7000) va phan mem host qua UART.
 *
 * Khung tin (truoc khi escape):
 *   STX | TYPE(1B) | LEN_LO(1B) | LEN_HI(1B) | PAYLOAD[LEN] | CRC16_LO | CRC16_HI | ETX
 *
 * CRC16 (CCITT, poly 0x1021, init 0xFFFF) tinh tren: TYPE + LEN_LO + LEN_HI + PAYLOAD.
 *
 * Byte-stuffing: cac byte STX/ETX/ESC xuat hien trong vung du lieu (TYPE..CRC_HI)
 * duoc thay bang ESC + (byte XOR 0x20). STX/ETX o dau/cuoi frame KHONG bi escape,
 * dung lam bien phan dinh (delimiter) de resync khi mat dong bo.
 *
 * Module nay khong tu doc/ghi UART - caller (task) chiu trach nhiem lay byte
 * tu UART RX va goi Codec_DecodeByte(), hoac lay buffer da encode va tu gui di.
 */



#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef FRAME_MAX_PAYLOAD
#define FRAME_MAX_PAYLOAD   256u
#endif
#define FRAME_MAX_ENCODED   (1u + 2u * (3u + FRAME_MAX_PAYLOAD + 2u) + 1u)
#define FRAME_STX        0x7Eu
#define FRAME_ETX        0x7Fu
#define FRAME_ESC        0x7Du
#define FRAME_ESC_XOR     0x20u

typedef enum {
    CODEC_IN_PROGRESS = 0,
    CODEC_FRAME_READY,
    CODEC_ERROR_CRC,
    CODEC_ERROR_OVERFLOW,
    CODEC_ERROR_SYNC
} CodecStatus_t;


typedef struct {
	uint8_t     type;
    uint16_t    len;
    uint8_t     payload[FRAME_MAX_PAYLOAD];
} Frame_t;

typedef enum {
    RXS_WAIT_STX = 0,
    RXS_TYPE,
    RXS_LEN_LO,
    RXS_LEN_HI,
    RXS_PAYLOAD,
    RXS_CRC_LO,
    RXS_CRC_HI,
    RXS_WAIT_ETX
} RxState_t;

typedef struct {
    RxState_t state;
    bool      escapeActive;

    uint8_t   type;
    uint16_t  len;
    uint16_t  payloadIdx;
    uint8_t   payload[FRAME_MAX_PAYLOAD];

    uint16_t  crcCalc;
    uint16_t  crcRecv;
    uint8_t   crcByteIdx;
} CodecDecoder_t;

uint16_t Codec_Crc16(uint16_t initial, const uint8_t *data, size_t len);

int Codec_EncodeFrame(const Frame_t *frame, uint8_t *outBuf, size_t outCap);

void Codec_DecoderInit(CodecDecoder_t *dec);

CodecStatus_t Codec_DecodeByte(CodecDecoder_t *dec, uint8_t byte, Frame_t *outFrame);

#ifdef __cplusplus
}
#endif

#endif /* FRAME_CODEC_H */
