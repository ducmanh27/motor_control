/*
 * frame_codec.c
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */
#include "frame_codec.h"
#include <string.h>

uint16_t Codec_Crc16(uint16_t initial, const uint8_t *data, size_t len)
{
    uint16_t crc = initial;
    size_t i;
    int b;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (b = 0; b < 8; b++) {
            if (crc & 0x8000u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

/* -------------------------------------------------------------------- */
/* Encode                                                                */
/* -------------------------------------------------------------------- */

/* Ghi 1 byte "raw" vao outBuf, tu dong escape neu trung STX/ETX/ESC.
 * Tra ve false neu khong con cho trong outBuf (out of capacity).
 */
static bool AppendEscaped(uint8_t *outBuf, size_t outCap, size_t *idx, uint8_t rawByte)
{
    if (rawByte == FRAME_STX || rawByte == FRAME_ETX || rawByte == FRAME_ESC) {
        if (*idx + 2u > outCap) {
            return false;
        }
        outBuf[(*idx)++] = FRAME_ESC;
        outBuf[(*idx)++] = (uint8_t)(rawByte ^ FRAME_ESC_XOR);
    } else {
        if (*idx + 1u > outCap) {
            return false;
        }
        outBuf[(*idx)++] = rawByte;
    }
    return true;
}

int Codec_EncodeFrame(const Frame_t *frame, uint8_t *outBuf, size_t outCap)
{
    if (frame == NULL || outBuf == NULL) {
        return -1;
    }
    if (frame->len > FRAME_MAX_PAYLOAD) {
        return -1;
    }

    /* Tinh CRC tren du lieu chua escape: TYPE + LEN_LO + LEN_HI + PAYLOAD */
    uint8_t header[3];
    header[0] = (uint8_t)frame->type;
    header[1] = (uint8_t)(frame->len & 0xFFu);
    header[2] = (uint8_t)((frame->len >> 8) & 0xFFu);

    uint16_t crc = Codec_Crc16(0xFFFFu, header, sizeof(header));
    if (frame->len > 0u) {
        crc = Codec_Crc16(crc, frame->payload, frame->len);
    }

    size_t idx = 0;

    if (idx + 1u > outCap) {
        return -1;
    }
    outBuf[idx++] = FRAME_STX; /* delimiter - khong escape */

    if (!AppendEscaped(outBuf, outCap, &idx, header[0])) return -1;
    if (!AppendEscaped(outBuf, outCap, &idx, header[1])) return -1;
    if (!AppendEscaped(outBuf, outCap, &idx, header[2])) return -1;

    for (uint16_t i = 0; i < frame->len; i++) {
        if (!AppendEscaped(outBuf, outCap, &idx, frame->payload[i])) return -1;
    }

    if (!AppendEscaped(outBuf, outCap, &idx, (uint8_t)(crc & 0xFFu)))        return -1;
    if (!AppendEscaped(outBuf, outCap, &idx, (uint8_t)((crc >> 8) & 0xFFu))) return -1;

    if (idx + 1u > outCap) {
        return -1;
    }
    outBuf[idx++] = FRAME_ETX; /* delimiter - khong escape */

    return (int)idx;
}

/* -------------------------------------------------------------------- */
/* Decode                                                                */
/* -------------------------------------------------------------------- */

void Codec_DecoderInit(CodecDecoder_t *dec)
{
    if (dec == NULL) {
        return;
    }
    memset(dec, 0, sizeof(*dec));
    dec->state = RXS_WAIT_STX;
}

/* Dua bo giai ma ve trang thai san sang cho frame tiep theo, giu nguyen
 * con tro dec (khong mat allocation), chi reset field lam viec.
 */
static void ResetForNextFrame(CodecDecoder_t *dec)
{
    dec->state       = RXS_WAIT_STX;
    dec->escapeActive = false;
    dec->type        = 0;
    dec->len         = 0;
    dec->payloadIdx  = 0;
    dec->crcCalc     = 0xFFFFu;
    dec->crcRecv     = 0;
    dec->crcByteIdx  = 0;
}

CodecStatus_t Codec_DecodeByte(CodecDecoder_t *dec, uint8_t byte, Frame_t *outFrame)
{
    if (dec == NULL) {
        return CODEC_ERROR_SYNC;
    }

    /* --- Cho STX: bo qua moi thu cho den khi thay STX --- */
    if (dec->state == RXS_WAIT_STX) {
        if (byte == FRAME_STX) {
            ResetForNextFrame(dec);
            dec->state = RXS_TYPE;
        }
        return CODEC_IN_PROGRESS;
    }

    /* --- Trong frame: xu ly escape truoc, roi moi xu ly byte thuc --- */
    if (byte == FRAME_ESC && !dec->escapeActive) {
        dec->escapeActive = true;
        return CODEC_IN_PROGRESS;
    }

    uint8_t realByte;
    if (dec->escapeActive) {
        realByte = (uint8_t)(byte ^ FRAME_ESC_XOR);
        dec->escapeActive = false;
    } else if (byte == FRAME_STX) {
        /* STX "song" giua chung frame -> mat dong bo, coi day la STX cua
         * mot frame moi va bat dau lai. */
        ResetForNextFrame(dec);
        dec->state = RXS_TYPE;
        return CODEC_ERROR_SYNC;
    } else {
        realByte = byte;
    }

    switch (dec->state) {

    case RXS_TYPE:
        dec->type = realByte;
        dec->crcCalc = Codec_Crc16(dec->crcCalc, &realByte, 1);
        dec->state = RXS_LEN_LO;
        return CODEC_IN_PROGRESS;

    case RXS_LEN_LO:
        dec->len = realByte;
        dec->crcCalc = Codec_Crc16(dec->crcCalc, &realByte, 1);
        dec->state = RXS_LEN_HI;
        return CODEC_IN_PROGRESS;

    case RXS_LEN_HI:
        dec->len |= (uint16_t)((uint16_t)realByte << 8);
        dec->crcCalc = Codec_Crc16(dec->crcCalc, &realByte, 1);
        if (dec->len > FRAME_MAX_PAYLOAD) {
            ResetForNextFrame(dec);
            return CODEC_ERROR_OVERFLOW;
        }
        dec->payloadIdx = 0;
        dec->state = (dec->len == 0u) ? RXS_CRC_LO : RXS_PAYLOAD;
        return CODEC_IN_PROGRESS;

    case RXS_PAYLOAD:
        dec->payload[dec->payloadIdx++] = realByte;
        dec->crcCalc = Codec_Crc16(dec->crcCalc, &realByte, 1);
        if (dec->payloadIdx >= dec->len) {
            dec->state = RXS_CRC_LO;
        }
        return CODEC_IN_PROGRESS;

    case RXS_CRC_LO:
        dec->crcRecv = realByte;
        dec->state = RXS_CRC_HI;
        return CODEC_IN_PROGRESS;

    case RXS_CRC_HI:
        dec->crcRecv |= (uint16_t)((uint16_t)realByte << 8);
        dec->state = RXS_WAIT_ETX;
        return CODEC_IN_PROGRESS;

    case RXS_WAIT_ETX:
        /* ETX phai la delimiter "song" (khong escape). Neu byte hien tai
         * khong phai ETX thi frame hong -> resync ve WAIT_STX. */
        if (byte != FRAME_ETX) {
            ResetForNextFrame(dec);
            return CODEC_ERROR_SYNC;
        }
        if (dec->crcCalc != dec->crcRecv) {
            ResetForNextFrame(dec);
            return CODEC_ERROR_CRC;
        }

        if (outFrame != NULL) {
            outFrame->type = dec->type;
            outFrame->len  = dec->len;
            if (dec->len > 0u) {
                memcpy(outFrame->payload, dec->payload, dec->len);
            }
        }
        ResetForNextFrame(dec);
        return CODEC_FRAME_READY;

    default:
        ResetForNextFrame(dec);
        return CODEC_ERROR_SYNC;
    }
}
