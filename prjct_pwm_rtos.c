/*
 *  ======== prjct_pwm_rtos.c ========
 *  Replicates Prjct_PWM_Ex, but using TI-RTOS
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/Timer.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>

/* Board Header file */
#include "Board.h"

#include <pthread.h>

/*
 * Global variables (better approach?)
 */
PWM_Handle redPWM, greenPWM, bluePWM;

/*
 * Prototypes.
 */
void buttonInt(uint_least8_t);
void *timerCallback(void *);

/*
 * Typedefs. (Should this go somewhere else?)
 */
typedef enum RGB_Opt {RED, GREEN, BLUE} RGB_Opt;


/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    PWM_Params red_params, green_params, blue_params;
    pthread_t led_thread;
    pthread_attr_t led_thread_attrs;
    struct sched_param priParam;
    int retc;

    /* Call driver init functions */
    GPIO_init();
    PWM_init();
    //Timer_init();
    // I2C_init();
    // SDSPI_init();
    // SPI_init();
    // UART_init();
    // Watchdog_init();

    GPIO_setCallback(Board_GPIO_BUTTON0, buttonInt);
    GPIO_setCallback(Board_GPIO_BUTTON1, buttonInt);
    GPIO_enableInt(Board_GPIO_BUTTON0);
    GPIO_enableInt(Board_GPIO_BUTTON1);

    GPIO_write(Board_GPIO_REDLED, Board_GPIO_LED_OFF);

    PWM_Params_init(&red_params);
    PWM_Params_init(&green_params);
    PWM_Params_init(&blue_params);

    // Since all clock, following have to be the same
    red_params.idleLevel = green_params.idleLevel = blue_params.idleLevel = PWM_IDLE_LOW;
    red_params.periodUnits = green_params.periodUnits = blue_params.periodUnits = PWM_PERIOD_US;
    red_params.periodValue = green_params.periodValue = blue_params.periodValue = 512;
    red_params.dutyUnits = green_params.dutyUnits = blue_params.dutyUnits = PWM_DUTY_US;

    red_params.dutyValue = 0;
    green_params.dutyValue = 0;
    blue_params.dutyValue = 0;

    redPWM = PWM_open(Board_PWM_RED, &red_params);
    greenPWM = PWM_open(Board_PWM_GREEN, &green_params);
    bluePWM = PWM_open(Board_PWM_BLUE, &blue_params);

    if(!redPWM || !greenPWM || !bluePWM)
    {
        while(1){}
    }

    //Initial PWM
    PWM_setDuty(redPWM, 0);
    PWM_setDuty(greenPWM, 0);
    PWM_setDuty(bluePWM, 0);

    // Set up thread for blinky.

    priParam.sched_priority = 1;
    pthread_attr_init(&led_thread_attrs);
    pthread_attr_setdetachstate(&led_thread_attrs, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedparam(&led_thread_attrs, &priParam);

    retc = pthread_create(&led_thread, &led_thread_attrs, timerCallback, NULL);

    if(retc!=0)
    {
        while(1)
        {
        }
    }

    PWM_start(redPWM);
    PWM_start(bluePWM);
    PWM_start(greenPWM);


    return 0;
}

void* timerCallback(void* arg0)
{
    while(1)
    {
        GPIO_toggle(Board_GPIO_REDLED);
        sleep(1);
    }

}

void buttonInt(uint_least8_t pin)
{
    static RGB_Opt cur_opt = BLUE;
    static uint8_t red_duty, green_duty, blue_duty;

    switch(pin)
    {
    case Board_GPIO_BUTTON0:
        // Change option being modified.
        cur_opt = (cur_opt + 1) % 3;
        break;
    case Board_GPIO_BUTTON1:
        switch(cur_opt)
        {
        case RED:
            red_duty = (red_duty + 20) % 512;\
            PWM_setDuty(redPWM, red_duty);
            break;
        case GREEN:
            green_duty = (green_duty + 20) % 512;
            PWM_setDuty(greenPWM, green_duty);
            break;
        case BLUE:
            blue_duty = (blue_duty + 20) % 512;
            PWM_setDuty(bluePWM, blue_duty);
            break;
        }
        break;
    }
}

