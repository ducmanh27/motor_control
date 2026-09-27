/*
 * motor_dispatcher.c
 *
 *  Created on: Sep 18, 2026
 *      Author: HLC
 */
#include "motor_dispatcher.h"

static QueueHandle_t s_cmdQueue = NULL;

static void OnCmdSetSpeed(const Frame_t *f)
{
	MotorCommand_t cmd;
    if (!Motor_UnpackCommand(f->payload, f->len, &cmd)) {
        return;
    }

    if (s_cmdQueue != NULL) {
        xQueueSend(s_cmdQueue, &cmd, 0);
    }
}

static void OnCmdStart(const Frame_t *f)
{
    (void)f;
    MotorCommand_t cmd = { .id = MSG_CMD_START };
    if (s_cmdQueue != NULL) xQueueSend(s_cmdQueue, &cmd, 0);
}

static void OnCmdStop(const Frame_t *f)
{
    (void)f;
    MotorCommand_t cmd = { .id = MSG_CMD_STOP };
    if (s_cmdQueue != NULL) xQueueSend(s_cmdQueue, &cmd, 0);
}




static const DispatchEntry_t table[] = {
    { MSG_CMD_SET_SPEED, OnCmdSetSpeed },
    { MSG_CMD_START,     OnCmdStart    },
    { MSG_CMD_STOP,      OnCmdStop     },
};

void Motor_Dispatcher_Init(QueueHandle_t cmdQueue)
{
    s_cmdQueue = cmdQueue;
}

void Dispatch_Frame(const Frame_t *frame)
{
    for (size_t i = 0; i < sizeof(table)/sizeof(table[0]); i++) {
        if (table[i].msgId == frame->type) {
            table[i].handler(frame);
            return;
        }
    }
}

