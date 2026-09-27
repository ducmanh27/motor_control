/*
 * pid.h
 *
 *  Created on: Sep 12, 2026
 *      Author: HLC
 */

#ifndef SRC_PID_H_
#define SRC_PID_H_

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float Ts;          // Thời gian lấy mẫu (giây), VD: 0.01s (10ms)

    float limMin;      // Giới hạn PWM nhỏ nhất (VD: -8399)
    float limMax;      // Giới hạn PWM lớn nhất (VD: 8399)

    float integrator;  // Biến lưu tổng tích phân
    float prevError;   // Sai số chu kỳ trước
} PIDController;
void PID_Init(PIDController *pid, float Kp, float Ki, float Kd, float Ts, float min, float max);
void PID_Reset(PIDController *pid);

float PID_Compute(PIDController *pid, float setpoint, float feedback);

#endif /* SRC_PID_H_ */
