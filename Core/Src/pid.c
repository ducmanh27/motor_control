/*
 * pid.c
 *
 *  Created on: Sep 12, 2026
 *      Author: HLC
 */

#include "pid.h"


void PID_Init(PIDController *pid, float Kp, float Ki, float Kd, float Ts, float min, float max) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->Ts = Ts;
    pid->limMin = min;
    pid->limMax = max;
    pid->integrator = 0.0f;
    pid->prevError = 0.0f;
}

void PID_Reset(PIDController *pid)
{
    pid->integrator      = 0.0f;
    pid->prevError       = 0.0f;
}


float PID_Compute(PIDController *pid, float setpoint, float feedback) {
    // 1. Tính sai số hiện tại
    float error = setpoint - feedback;

    // 2. Thành phần P
    float proportional = pid->Kp * error;

    // 3. Thành phần I tạm thời
    float new_integrator = pid->integrator + 0.5f * pid->Ki * pid->Ts * (error + pid->prevError);

    // 4. Thành phần D
    float derivative = pid->Kd * (error - pid->prevError) / pid->Ts;

    // 5. Tính tổng đầu ra dự kiến
    float output = proportional + new_integrator + derivative;

    // 6. Xử lý Anti-windup (Clamping) & Giới hạn Output
    if (output > pid->limMax) {
        output = pid->limMax;
        // Nếu output bị tràn dương, chỉ cho phép xả tích phân (không tích lũy thêm)
        if (error < 0) {
            pid->integrator = new_integrator;
        }
    } else if (output < pid->limMin) {
        output = pid->limMin;
        // Nếu output bị tràn âm, chỉ cho phép xả tích phân
        if (error > 0) {
            pid->integrator = new_integrator;
        }
    } else {
        // Nếu nằm trong dải hoạt động an toàn, cập nhật tích phân bình thường
        pid->integrator = new_integrator;
    }

    // 7. Lưu lại sai số cho chu kỳ sau
    pid->prevError = error;

    return output;
}
