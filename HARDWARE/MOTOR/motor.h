#ifndef __MOTOR_H__
#define __MOTOR_H__
#include "sys.h"

#define MOTOR_IN1 PBout(7)
#define MOTOR_IN2 PAout(4)
#define MOTOR_IN3 PGout(15)
#define MOTOR_IN4 PCout(9)

#define MOTOR_DOUBLE_POS 1
#define MOTOR_DOUBLE_REV 2
#define MOTOR_EGHIT_POS 3
#define MOTOR_EGHIT_REV 4

extern void motor_init(void);
extern void motor_corotation_double_pos(void);
extern void motor_corotation_double_rev(void);
extern void motor_corotation_eghit_pos(void);
extern void motor_corotation_eghit_rev(void);

#endif
