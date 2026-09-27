/*
 * motor_protocol.c
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */
#include "motor_protocol.h"
#include "frame_codec.h"
#include <string.h>
#include "stm32f4xx_hal.h"
/* ===================== COMMAND ===================== */

size_t Motor_PackCommand(const MotorCommand_t *cmd, uint8_t *buf, size_t cap)
{
    if (cmd == NULL || buf == NULL || cap < sizeof(MotorCommand_t)) {
        return 0;
    }
    memcpy(buf, cmd, sizeof(MotorCommand_t));
    return sizeof(MotorCommand_t);
}

bool Motor_UnpackCommand(const uint8_t *buf, uint16_t len, MotorCommand_t *out)
{
    if (buf == NULL || out == NULL || len == 0) {
        return false;
    }

    memset(out, 0, sizeof(*out));

    size_t copyLen = (len < sizeof(MotorCommand_t)) ? len : sizeof(MotorCommand_t);
    memcpy(out, buf, copyLen);
    return true;
}

size_t Motor_PackTelemetry(const TelemetryPayload_t *tel, uint8_t *buf, size_t cap)
{
    if (tel == NULL || buf == NULL || cap < sizeof(TelemetryPayload_t)) {
        return 0;
    }
    memcpy(buf, tel, sizeof(TelemetryPayload_t));
    return sizeof(TelemetryPayload_t);
}

bool Motor_UnpackTelemetry(const uint8_t *buf, uint16_t len, TelemetryPayload_t *out)
{
    if (buf == NULL || out == NULL || len == 0) {
        return false;
    }
    memset(out, 0, sizeof(*out));

    size_t copyLen = (len < sizeof(TelemetryPayload_t)) ? len : sizeof(TelemetryPayload_t);
    memcpy(out, buf, copyLen);
    return true;
}

/* ===================== SEND WRAPPER ===================== */

extern UART_HandleTypeDef huart2;
extern volatile uint8_t   g_uart_tx_busy;

bool Motor_SendTelemetry(const TelemetryPayload_t *tel)
{
    if (g_uart_tx_busy) {
        return false;
    }

    static uint8_t payloadBuf[sizeof(TelemetryPayload_t)];
    static uint8_t txFrameBuf[FRAME_MAX_ENCODED];

    size_t payloadLen = Motor_PackTelemetry(tel, payloadBuf, sizeof(payloadBuf));
    if (payloadLen == 0) {
        return false;
    }

    Frame_t frame;
    frame.type = MSG_TELEMETRY;
    frame.len  = (uint16_t)payloadLen;
    memcpy(frame.payload, payloadBuf, payloadLen);

    int encodedLen = Codec_EncodeFrame(&frame, txFrameBuf, sizeof(txFrameBuf));
    if (encodedLen <= 0) {
        return false;
    }

    g_uart_tx_busy = 1;
    if (HAL_UART_Transmit_DMA(&huart2, txFrameBuf, (uint16_t)encodedLen) != HAL_OK) {
        g_uart_tx_busy = 0;
        return false;
    }
    return true;
}
