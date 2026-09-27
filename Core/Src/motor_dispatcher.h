/*
 * motor_dispatcher.h
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */

#ifndef MOTOR_DISPATCHER_H_
#define MOTOR_DISPATCHER_H_

#include "FreeRTOS.h"
#include "queue.h"

#include "frame_codec.h"
#include "motor_protocol.h"

typedef void (*MsgHandler_t)(const Frame_t *frame);

typedef struct {
    uint8_t       msgId;
    MsgHandler_t  handler;
} DispatchEntry_t;


void Motor_Dispatcher_Init(QueueHandle_t cmdQueue);

void Dispatch_Frame(const Frame_t *frame);

#endif /* MOTOR_DISPATCHER_H_ */
