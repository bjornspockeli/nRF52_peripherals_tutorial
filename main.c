/**
 * Copyright (c) 2009 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
* @brief Example template project.
* @defgroup nrf_templates_example Example Template
*
*/

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

// Header files that needs to be included for PPI/GPIOTE/TIMER/TWI example.

// Peripheral driver header files
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_clock.h"

#include "nrf_drv_uart.h"

// Library header files
#include "app_timer.h"
#include "app_button.h"
#include "app_pwm.h"

#include "app_uart.h"

// Headers and defines needed by the logging interface
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"

#define UART_TX_BUF_SIZE                256     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256     /**< UART RX buffer size. */

// Timer instance
const nrf_drv_timer_t timer0 = NRF_DRV_TIMER_INSTANCE(0);

// PPI channel
nrf_ppi_channel_t ppi_channel;

// PMW instance
APP_PWM_INSTANCE(PWM2, 2);  // Setup a PWM instance with TIMER 2

static volatile int ready_flag = true;

//Application timer instance
APP_TIMER_DEF(m_led_timer_id);

// Dummy timer event handler that will be used for step 2 and 4, but not step 3.
void timer_event_handler(nrf_timer_event_t event_type, void * p_context)
{
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            nrf_drv_gpiote_out_task_trigger(LED_3);
            NRF_LOG_INFO("Toogle LED3 \r\n");
            break;
        default:
            // Do nothing.
            break;
    }
    
}

/** @brief Function for initializing the Timer peripheral.
*/
static void timer_init(void) {
    
    ret_code_t err_code;
    
    // Configure the timer to use the default configuration set in sdk_config.h
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    
    //Initializing the Timer driver
    err_code = nrf_drv_timer_init(&timer0, &timer_cfg, timer_event_handler);
    APP_ERROR_CHECK(err_code);
    
    /*Configure the timer to generate the COMPARE event after 200*1000UL ticks and enable the shortcut that triggers the CLEAR task on every COMPARE event.
        This will */
    nrf_drv_timer_extended_compare(&timer0, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&timer0, 1000), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true); // Set last argument to false for Task 3
    
    // Turning on the Timer. 
    nrf_drv_timer_enable(&timer0);
}


/** @brief Function for initializing the GPIOTE peripheral.
*/
static void gpiote_init(void) {
    
    ret_code_t err_code;
    
    // Check wether the GPIOTE driver has been initialized before by any other module, e.g. app_button_init().
    if (!nrf_drv_gpiote_is_init())
    {
        // Initialize the GPIOTE driver
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    // Configure the GPIO pin so that its toggled every time the OUT task is triggerd
    nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false); 
    
    // Apply the configuration above to the LED_1 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED_1, &config);
    APP_ERROR_CHECK(err_code);
    
    // Apply the configuration above to the LED_2 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED_2, &config);
    APP_ERROR_CHECK(err_code);
    
       // Apply the configuration above to the LED_2 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED_3, &config);
    APP_ERROR_CHECK(err_code);
   
    // Enabling the OUT task for the LED_1 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED_1);
    
    // Enabling the OUT task for the LED_2 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED_2);
    
    // Enabling the OUT task for the LED_3 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED_3);
}

/** @brief Function for initializing the PPI peripheral.
*/
static void ppi_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    //Initialize the PPI driver
    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    
   // Allocate the first unused PPI channel 
    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);
    
    
    // Assigning task and event endpoints so that the PPI channel triggers the OUT task of the LED_1 GPIO pin on TIMER0 COMPARE[0] match.
    err_code = nrf_drv_ppi_channel_assign(ppi_channel,
                                          nrf_drv_timer_event_address_get(&timer0, NRF_TIMER_EVENT_COMPARE0),
                                          nrf_drv_gpiote_out_task_addr_get(LED_1)); 
    APP_ERROR_CHECK(err_code);
      
    // Fork the PPI channel, so that OUT task of the LED_2 GPIO pin is also triggered by the TIMER compare event. 
    err_code = nrf_drv_ppi_channel_fork_assign(ppi_channel, nrf_drv_gpiote_out_task_addr_get(LED_2));
    APP_ERROR_CHECK(err_code);
    
    // Enable the PPI channel 
    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);
   
}   

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void led_timer_timeout_handler(void)
{
    nrf_gpio_pin_toggle(LED_1);
}

static void application_timer_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_led_timer_id,APP_TIMER_MODE_REPEATED, led_timer_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_led_timer_id,APP_TIMER_TICKS(1000),NULL);
    APP_ERROR_CHECK(err_code);

}

void button_handler(uint8_t pin_no, uint8_t button_action)
{
    NRF_LOG_INFO("Button press detected \r\n");
    if(pin_no == BUTTON_1 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 1 pressed \r\n");
        nrf_drv_gpiote_out_task_trigger(LED_1);
        APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 10));
        ready_flag = false;
         nrf_delay_ms(1000);
        while(!ready_flag){APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 0));}
    }
    if(pin_no == BUTTON_2 && button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button 2 pressed \r\n");
        nrf_drv_gpiote_out_task_trigger(LED_2);
        APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 5));
        ready_flag = false;
        nrf_delay_ms(1000);
        while(!ready_flag){APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM2, 0, 0));}
    }

}


static void buttons_init()
{
    ret_code_t err_code;
/*
    static app_button_cfg_t button_cfg = {
      .pin_no = BUTTON_1,
      .active_state = APP_BUTTON_ACTIVE_LOW,
      .pull_cfg = NRF_GPIO_PIN_PULLUP,
      .button_handler = button_handler 
    };
*/
    static app_button_cfg_t button_cfg[2] = {
        {BUTTON_1,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,button_handler},
        {BUTTON_2,APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,button_handler}
    };

    err_code = app_button_init(button_cfg,2,5);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

static void pwm_init()
{
    ret_code_t err_code;
    
    app_pwm_config_t pwm2_cfg = APP_PWM_DEFAULT_CONFIG_1CH(20000L, 4);
    pwm2_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    err_code = app_pwm_init(&PWM2,&pwm2_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);

    app_pwm_enable(&PWM2);
}

void clock_event_handler(nrf_drv_clock_evt_type_t event)
{
  if(event == NRF_DRV_CLOCK_EVT_LFCLK_STARTED )
  {
     NRF_LOG_INFO("LFCLK started \r\n");
  }

}


static void lfclk_init()
{
/*
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_handler_item_t clock_handler_item;

    clock_handler_item.event_handler = clock_event_handler;

    nrf_drv_clock_lfclk_request(NULL);
  */  
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }

}

void uart_event_handler(app_uart_evt_t * p_event)
{
    static uint8_t data_array[32];
    static uint8_t index = 0;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            app_uart_get(&data_array[index]);
            index++;

            if (data_array[index - 1] == '\n') 
            {
                for (uint32_t i = 0; i < strlen((const char *)data_array); i++)
                {
                    while (app_uart_put(data_array[i]) != NRF_SUCCESS);
                }
                memset(data_array,0,sizeof(data_array));
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

static void uart_init()
{

    uint32_t err_code;
    const app_uart_comm_params_t uart_comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT( &uart_comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handler,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);

}

static void uart_print(uint8_t data_string[])
{
  static uint8_t id[] = "[nRF52 DK]: ";
  static uint8_t array[256];
  
  memcpy(array,id, sizeof(id));
  memcpy(array+sizeof(id)-1,data_string,strlen(data_string));
  
  for (uint32_t i = 0; i < strlen(array); i++)
  {
      while (app_uart_put(array[i]) != NRF_SUCCESS);
  }
  
}

static void power_manage()
{ 
  /* WFE - If the Event Register is not set, WFE suspends execution until one of the following events occurs:
            * an IRQ interrupt, unless masked by the CPSR I-bit
            * an FIQ interrupt, unless masked by the CPSR F-bit
            * an Imprecise Data abort, unless masked by the CPSR A-bit
            * a Debug Entry request, if Debug is enabled
            * an Event signaled by another processor using the SEV instruction. 
    
    If the Event Register is set, WFE clears it and returns immediately. If WFE is implemented, SEV must also be implemented.    
    
    SEV - SEV causes an event to be signaled to all cores within a multiprocessor system. If SEV is implemented, WFE must also be implemented.
   */

    // Wait for an event.
    __WFE();
    // Clear any pending events.
    __SEV();
    __WFE();
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{ 
    //log_init();
    
    //NRF_LOG_INFO("nRF52 Peripheral Tutorial \r\n");

    lfclk_init();
    
    // Must be called before buttons_init as the Button Handler library(app_button.c) is used.
    application_timer_init();
    
    // Initialize the buttons
    buttons_init();
    
    // The GPIOTE peripheral must be initialized first, so that the correct Task Endpoint addresses are returned by nrf_drv_gpiote_xxx_task_addr_get()
    //gpiote_init();
    //ppi_init();
    //timer_init();

    pwm_init();
    
    uart_init();

    uart_print("Nordic Semiconductor ASA\r\n");

    nrf_gpio_cfg_output(LED_1);

    while (true)
    {
        
        //power_manage();
        // Do nothing.
        //nrf_delay_ms(1000);
        //nrf_gpio_pin_toggle(LED_1);
    }
}
/** @} */
