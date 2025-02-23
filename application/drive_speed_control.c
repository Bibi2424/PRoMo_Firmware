#define DEBUG_THIS_FILE DEBUG_DRIVE_SPEED_CONTROL_FILE

#include <math.h>

#include "time.h"
#include "utils.h"
#include "main.h"
#include "drive_speed_control.h"
#include "control_loop.h"
#include "encoder.h"
#include "motor.h"


static void set_speed(unsigned id, float output);


#define DEFAULT_SPEED_PID \
    .proportional_gain = 1.5f, \
    .integral_gain = 0.3f, \
    .derivative_gain = 0.0f

#define DEFAULT_ODOMETRY_PARAMS \
    .wheel_radius = WHEEL_RADIUS, \
    .tick_to_rad = SENSOR_TICK_TO_RAD

#define DEFAULT_CONTROL_LOOP_PARAMS \
    .target = 0.0f, \
    .next_target = 0.0f, \
    .min_output = MIN_SPEED, \
    .max_output = MAX_SPEED, \
    .pid = { DEFAULT_SPEED_PID }, \
    .get_feedback = odometry_get_speed, \
    .set_output = set_speed, \
    .filtered_target = 0.0f, \
    .last_feedback = 0.0f, \
    .last_output = 0.0f

static control_loop_t control_loops[2] = {
    {
        .name = "Left",
        .id = LEFT_SIDE,
        .odo = { 
            .invert = INVERSE_LEFT_ENCODER,
            DEFAULT_ODOMETRY_PARAMS,
        },
        DEFAULT_CONTROL_LOOP_PARAMS,
    },
    {
        .name = "Right",
        .id = RIGHT_SIDE,
        .odo = { 
            .invert = INVERSE_RIGHT_ENCODER,
            DEFAULT_ODOMETRY_PARAMS,
        },
        DEFAULT_CONTROL_LOOP_PARAMS,
    }
};


static void set_speed(unsigned id, float output) {
    if(id == NO_SIDE) { return; }
    int32_t output_percent = output * 100.0f / MAX_SPEED;

    motor_set_dir(id, (output > 0)? MOTOR_DIR_FORWARD: MOTOR_DIR_REVERSE);
    motor_set_speed(id, (output > 0)? output_percent: -output_percent);
}


extern void drive_speed_control_init(void) {
    debugf("&Left,t,target_speed,current_speed,motor_command\n");
    debugf("&Right,t,target_speed,current_speed,motor_command\n");

    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);

    /* TIM11 interrupt Init */
    NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
    NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

    const uint16_t frequency = US_TO_S / MOTOR_CONTROL_INTERVAL_US;
    // Timer at 1MHz overflow, update at `frequency`
    TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000UL);
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, TIM_InitStruct.Prescaler, frequency);
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM11, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM11);
}


extern void drive_speed_control_enable(bool enable) {
    if(enable) {
        LL_TIM_EnableCounter(TIM11);
        LL_TIM_ClearFlag_UPDATE(TIM11);
        LL_TIM_EnableIT_UPDATE(TIM11);
    }
    else {
        LL_TIM_DisableCounter(TIM11);
        LL_TIM_DisableIT_UPDATE(TIM11);
    }
}


static inline void debug_control(control_loop_t* control) {
    debugf("@%s,%.4f,%.4f,%.4f,%.4f,%.4f\n", control->name, (float)get_time_absolute_us() / 1000000.0f,
        control->target, control->last_feedback, control->last_output, control->pid.integral_error);
}


void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    static uint8_t debug_cnt = 0;
    if(LL_TIM_IsActiveFlag_UPDATE(TIM11) == 1) {
        LL_TIM_ClearFlag_UPDATE(TIM11);             // Clear the update interrupt flag

        for(unsigned i = 0; i < ARRAY_SIZE(control_loops); i++) {
            control_loops[i].odo.count = encoder_get_tick_count(control_loops[i].id);
            if(debug_cnt == 0 && i == 0) { debugf("[%u]\n", control_loops[i].odo.count - control_loops[i].odo.last_count); }
            
            control_loop_run(&control_loops[i]);

            if(debug_cnt == 0) {
                debug_control(&control_loops[i]);
            }
        }
        if(debug_cnt == 0) { debug_cnt = 10; }
        debug_cnt--;
    }
}


extern void set_target_speed_percent(int32_t target_percent_left, int32_t target_percent_right) {
    //! NOTE: speed = [-MAX_SPEED..MAX_SPEED]
    if(target_percent_left > 100) { target_percent_left = 100; }
    else if(target_percent_left < -100) { target_percent_left = -100; }
    if(target_percent_right > 100) { target_percent_right = 100; }
    else if(target_percent_right < -100) { target_percent_right = -100; }

    float target_speed_left = (float)target_percent_left * MAX_SPEED / 100.0f;
    float target_speed_right = (float)target_percent_right * MAX_SPEED / 100.0f;
    // debugf("@TS,,%0.3f,%0.3f\n", target_speed_left, target_speed_right);

    for(unsigned i = 0; i < ARRAY_SIZE(control_loops); i++) {
        if(control_loops[i].id == LEFT_SIDE) {
            control_loops[i].next_target = target_speed_left;
        }
        else if(control_loops[i].id == RIGHT_SIDE) {
            control_loops[i].next_target = target_speed_right;
        }
    }
}


extern void update_speed_pid(actuator_t id, float p, float i, float d) {
    if(id == LEFT_SIDE || id == BOTH_SIDE) {
        control_loops[0].pid.proportional_gain = p;
        control_loops[0].pid.integral_gain = i;
        control_loops[0].pid.derivative_gain = d;
        control_loops[0].pid.integral_error = 0;
        debugf("Update Left, p=%f, i=%f, d=%f\n", p, i, d);
    }
    if(id == RIGHT_SIDE || id == BOTH_SIDE) {
        control_loops[1].pid.proportional_gain = p;
        control_loops[1].pid.integral_gain = i;
        control_loops[1].pid.derivative_gain = d;   
        control_loops[1].pid.integral_error = 0;
        debugf("Update Right, p=%f, i=%f, d=%f\n", p, i, d);
    }
}
