#include "PWM.h"

void mcpwm_gpio_initialize_both_wheels(void)
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, LEFT_WHEEL_R);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, LEFT_WHEEL_B);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, RIGHT_WHEEL_R);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, RIGHT_WHEEL_B);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Rover stops
 */
void rover_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num_left_wheel, mcpwm_timer_t timer_num_right_wheel)
{
    brushed_motor_stop(mcpwm_num, timer_num_left_wheel);
    brushed_motor_stop(mcpwm_num, timer_num_right_wheel);

}

/**
 * @brief Rover moves forward
 */
void rover_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num_left_wheel, float duty_cycle_left_wheel, mcpwm_timer_t timer_num_right_wheel, float duty_cycle_right_wheel)
{
    brushed_motor_forward(mcpwm_num, timer_num_left_wheel, duty_cycle_left_wheel);
    brushed_motor_backward(mcpwm_num, timer_num_right_wheel, duty_cycle_right_wheel);
}

/**
 * @brief Rover moves backward
 */
void rover_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num_left_wheel, float duty_cycle_left_wheel, mcpwm_timer_t timer_num_right_wheel, float duty_cycle_right_wheel)
{
    brushed_motor_backward(mcpwm_num, timer_num_left_wheel, duty_cycle_left_wheel);
    brushed_motor_forward(mcpwm_num, timer_num_right_wheel, duty_cycle_right_wheel);
}

/**
 * @brief TODO : IMPLEMENTATION
 * 
 */
void rover_right_turn(void)
{
    //place holder for turning left TODO
}
/**
 * @brief TODO : IMPLEMENTATION
 * 
 */
void rover_left_turn(void)
{
    //place holder for turning left TODO
}