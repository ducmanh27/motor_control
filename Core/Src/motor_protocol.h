/*
 * motor_protocol.h
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */

#ifndef MOTOR_PROTOCOL_H_
#define MOTOR_PROTOCOL_H_
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

typedef enum {
    MSG_CMD_START              = 0x01,
    MSG_CMD_STOP               = 0x02,
    MSG_CMD_SET_SPEED          = 0x03,
    MSG_CMD_GET_STATUS         = 0x04,
    MSG_CMD_ESTOP              = 0x05,  /* dừng khẩn, bypass ramp */
    MSG_CMD_CLEAR_FAULT        = 0x06,  /* fault thường latch, cần lệnh xoá tường minh */
    MSG_CMD_SET_TELEMETRY_RATE = 0x07,  /* tuỳ chọn, giảm tải UART khi không cần vẽ */

    MSG_TELEMETRY              = 0x10,  /* mcu -> host, định kỳ */
    MSG_RESP_STATUS            = 0x12,  /* trả lời GET_STATUS, dùng khi app vừa connect */
    MSG_EVENT_FAULT            = 0x1A,  /* bắn ngay khi lỗi xảy ra, không chờ tick telemetry kế tiếp */
    MSG_RESP_ACK               = 0x11,
    MSG_RESP_ERROR             = 0x1F,
} MotorMsgId_t;

typedef enum {
    MOTOR_STATE_IDLE,      /* PWM = 0, PID không chạy */
    MOTOR_STATE_RUNNING,   /* bám target_speed */
    MOTOR_STATE_STOPPING,  /* setpoint đang ramp về 0 */
} MotorState_t;

typedef enum {
    FAULT_NONE          = 0x00,
    FAULT_OVERCURRENT   = (1u << 0),
    FAULT_OVERSPEED     = (1u << 1),
    FAULT_ENCODER_LOST  = (1u << 2),
    FAULT_STALL         = (1u << 3),
} MotorFault_t;

typedef struct {
	MotorMsgId_t id;
    union {
        float target_vel;   /* dùng khi id == MOTOR_CMD_SET_SPEED */
    } data;
} __attribute__((packed)) MotorCommand_t;

typedef struct {
    uint32_t timestamp_ms;
    uint16_t seq;              /* tăng dần mỗi frame, để app phát hiện mất gói */
    float    setpoint_rpm;     /* active_setpoint SAU ramp - để chồng lên current_rpm trên đồ thị */
    float    current_rpm;
    float    current_amp;
    float    pwm_duty;         /* control effort u (-1..1), để thấy PID có bão hòa/dao động không */
    uint8_t  state;            /* MotorState_t: IDLE/RUNNING/STOPPING/FAULT */
    uint8_t  fault_flags;      /* bitmask, hỗ trợ nhiều lỗi cùng lúc */
} __attribute__((packed)) TelemetryPayload_t;


/* Command pack/unpack */
size_t Motor_PackCommand(const MotorCommand_t *cmd, uint8_t *buf, size_t cap);
bool   Motor_UnpackCommand(const uint8_t *buf, uint16_t len, MotorCommand_t *out);

/* Telemetry pack/unpack */
size_t Motor_PackTelemetry(const TelemetryPayload_t *tel, uint8_t *buf, size_t cap);
bool   Motor_UnpackTelemetry(const uint8_t *buf, uint16_t len, TelemetryPayload_t *out);

/* Wrapper gửi telemetry */
bool   Motor_SendTelemetry(UART_HandleTypeDef* huart, const TelemetryPayload_t *tel);

#endif /* MOTOR_PROTOCOL_H_ */
