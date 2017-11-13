# nRF52 Peripheral Tutorial

### Brief:

Several small tutorials/exercises that shows you how to: 
 - create an Application Timer to toogle a GPIO pin
 - configure buttons to toggle a GPIO pin
 - generate a PWM pulse that is used to control a analog servo
 - send serial data to and from a terminal window
 - measure the die temperature
 - automate  peripheral  using the tasks and event system 
 -  configure a timer to toggle a gpio pin  to interact autonomously with each
other using tasks and events independent of the CPU

### Requirements
- nRF52 DK
- SDK v14.1.0
- Template Project found in nRF5_SDK_14.1.0_04a0bfd\examples\peripheral\template_project

## Tasks

In all the tasks we'll be using the SDK drivers or libraries for the peripherals, i.e. nrf_drv_xxx.c, which can be found in nRF5_SDK_14.1.0_1dda907\components\drivers_nrf\ and nRF5_SDK_14.1.0_1dda907\components\libraries respectively.

### Warm-up 

The template project includes all the peripheral libraries and drivers from the SDK, but we're only going to use a few, so to reduce the compile time and size of our project we'll temporarily remove them from the project. 

1. Find the *template_project* folder in nRF5_SDK_14.1.0_dda907\examples\peripheral\

2. Create a copy of the  template_project folder and rename it to *nRF52_peripherals_tutorial*

3. Open the *template_pca10040.emProject* Segger Embedded Studio project found in *nRF52_peripherals_tutorial\pca10040\blank\ses*


### 1. Blink a LED using a busy-wait loop

Goal: Blink a LED by keeping the CPU in a busy-wait loop.

1. Include the following headers in main.c

```c
#include "nrf_delay.h"
#include "nrf_gpio.h"
```

2. Use the nrf_gpio_cfg_output() function to configure one of the pins connected to one of the LEDs of the NRF52 DK as an output.
    Hint: See the back of the nRF52 DK for the pin assignments.

3. Use nrf_delay_ms() and nrf_gpio_pin_toggle() to blink a LED within the . 

### 2. Application Timer 

 Blinking a LED with a busy-wait loop is not a very efficient as you'll keep the CPU running without actually doing anything useful. A much better approach would be to set up a timer to toggle the LED at a given interval so that the CPU can do meaningful tasks or sleep in between the timer interrupts.

The Application Timer library provides a user friendly way of using the Real Time Counter 1 (RTC1) peripheral to create multiple timer instances. The RTC uses the Low Frequency Clock (LFCLK). Most applications keep the LFCLK active at all times and when using one of the Nordic SoftDevices the LFCLK is always active. Therefore, there is normally very little extra power consumption associated with using the application timer. As the clock is 32.768 kHz and the RTC is 24 bit, the time/tick resolution is limited, but it takes a substantial amount of time before the counter wrap around (from 0xFFFFFF to 0). By using the 12 bit (1/x) prescaler the frequency of the RTC can be lowered.  

The Application Timer library API is documented [here](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__timer.html)

1. As mentioned in the introduction of this task, the application timer uses the RTC peripheral, which in turn uses the 32kHz LFCLK. Hence, we need to start the LFCLK for the application timer to fucntion properly. Create a function called lfclk_init() where you add the following snippet.

```c
NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
NRF_CLOCK->TASKS_LFCLKSTART    = 1;

while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
{
    // Do nothing.
}
```

2. Include the app_timer.h file in your main.c file.

3. Next, you'll need to create a application timer instance using the APP_TIMER_DEF macro.

4. Create the function application_timer_init(), in which you initialize the application timer library, create and start the application timer. 
        Hints:
            - You will need to use the functions app_timer_init(), app_timer_create() and app_timer_start()
            - You want to create a repeating timer, i.e. the mode of the applicaiton timer should be set to APP_TIMER_MODE_REPEATED.
            - The APP_TIMER_TICKS macro is very useful when setting the timeout interval.
            - Make sure to call the application_timer_init() function in main().

5. Call nrf_gpio_toogle() function to toggle one of the nRF52 DKs LEDs in the timeout handler that you specified when you initialized the application timer.


### 3. Buttons - Button Handler Library

The button handler uses the GPIOTE Handler to detect that a button has been pushed. To handle debouncing, it will start a timer in the GPIOTE event handler. The button will only be reported as pushed if the corresponding pin is still active when the timer expires. If there is a new GPIOTE event while the timer is running, the timer is restarted.

The Button Handler Library API is documented [here](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__button.html?resultof=%22%62%75%74%74%6f%6e%22%20%22%68%61%6e%64%6c%65%72%22%20). 

1. Normally we would need to add the Button Handler library, app_button.c, under nRF_Libraries in our project, but it is already been added in the template project. However,  we do need to include the app_button.h header at the top of main.c.

2. Next, create a static void function called buttons_init(), where you initalize the Button Handler library using app_button_init().

    Hints:
    - You will need to create a app_button_cfg_t struct for each button you configure. Make sure to declare it as *static*.
    - It is possible to configure a separate event handler for each individual button, but in this exercise we will use one event handler for all the buttons.
    - The button pin number as well as the active state of the buttons can be seen on the backside of the nRF52 DK.
    - After initializing the Button Handler library with button configuration you will need to enable it, there should be a appropriate function in the API.


3. In the event handler that you set in the button configuration structure you will have to check which pin as well as which action that generated the event. Add code to the event handler so that one of the LEDs of the nRF52 DK is toggled when you push one of the buttons on the nRF52 DK.
    Hint: 
    - There are two button action types, APP_BUTTON_PUSH and APP_BUTTON_RELEASE.
    - You can see which pins that are connected to the different buttons on the back of the nRF52 DK.

```c
void button_handler(uint8_t pin_no, uint8_t button_action)
{
    // Check which pin that generated the event as well as which type of button action that caused the event.
}
```


### 4. Servo - Controlling a servo using the PWM library

In this task we will use [Pulse-Width Modulation](https://learn.sparkfun.com/tutorials/pulse-width-modulation) to control a analig servo. The PWM library uses one of the nRF52s TIMER peripherals in addition to the PPI and GPIOTE peripherals. The app_pwm library is documented on [this](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__pwm.html?resultof=%22%61%70%70%22%20%22%70%77%6d%22%20) Infocenter page

Connecting the Servo to your nRF52 DK:

The three wires coming from the SG92R Servo are:

Brown: Ground - Should be connected to one of the pins marked GND on your nRF52 DK.

Red: 5V - Should be connected to the pin marked 5V on your nRF52 DK.

Orange: PWM Control Signal - Should be connected to one of the unused GPIO pins of the nRF52 DK (for example P0.04, pin number 4).

1. The first thing we have to do is to include the header to the PWM library, `app_pwm.h` and create a PWM instance with the `APP_PWM_INSTANCE` macro that uses the TIMER2 peripheral. 
<!---
This is done as shown below 

```c
#include "app_pwm.h"

// PMW instance
APP_PWM_INSTANCE(PWM2, 2);  // Setup a PWM instance with TIMER 2
```
--->

2. The second thing we have to do is creating the function `pwm_init()` where we configure, initialize and enable the PWM peripheral. You configure the pwm library by creating a [app_pwm_config_t](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/structapp__pwm__config__t.html) and pass it as a parameter to where the following parameters must be specified:

    - **pins**: Array of two unsigned integers that indicate which physical pins will be used for the PWM output. In one-channel mode, the second element is ignored.
    - **pin_polarity**: 2-element array of app_pwm_polarity_t that indicates the output signal polarity. In one-channel mode, the second element is ignored.
    - **num_of_channels**: Number of PWM channels (1 or 2).
    - **period_us**: Signal period (in microseconds).

Hints:
    - The polarity can be set to either `APP_PWM_POLARITY_ACTIVE_HIGH` or `APP_PWM_POLARITY_ACTIVE_LOW`.
    - We only need one PWM channel.  
    - The second element of the pins array should be set to `APP_PWM_NOPIN`.
    - The period of the PWM pulse should be 20ms


<!---
```c
    app_pwm_config_t pwm_config = {
        .pins               = {4, APP_PWM_NOPIN},
        .pin_polarity       = {APP_PWM_POLARITY_ACTIVE_HIGH, APP_PWM_POLARITY_ACTIVE_LOW}, 
        .num_of_channels    = 1,                                                          
        .period_us          = 20000L                                                
    };
```
--->

3. The struct must be passed as an input to the [app_pwm_init](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__pwm.html#gae3b3e1d5404fd776bbf7bf22224b4b0d) function which initializes the PWM library. After initializng the PWM library you have to enable the PWM instance by calling [app_pwm_enable](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__pwm.html#ga94f5d824afec86aff163f7cccedaa436).

Hints:
    - We do not need to provide an event handler function, i.e. you can pass NULL instead of a function pointer.
    - Make sure that you add `pwm_init()` to the `main()` function before the while-loop.

<!---

```c
    uint32_t err_code;
    err_code = app_pwm_init(&PWM2,&pwm_config,NULL);
    APP_ERROR_CHECK(err_code);
```

You can initialize the PWM library with a callback function that is called when duty cycle change process is finished, but this is not necessary for this example so we'll just pass NULL as an argument. After initializng the PWM library you have to enable the PWM instance by calling [app_pwm_enable](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__pwm.html#ga94f5d824afec86aff163f7cccedaa436).

<!---
```c
    app_pwm_enable(&PWM2);
```

The `pwm_init()` function is now finished and can add it to the `main()` function before the infinite for-loop.
--->
4. Now that we have initialized the PWM library you can set to set the duty cycle of the PWM signal to the servo using the  [app_pwm_channel_duty_set](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v14.1.0/group__app__pwm.html#ga071ee86851d8c0845f297df5d23a240d) function. This will set the duty cycle of the PWM signal, i.e. the percentage of the total time the signal is high or low depending on the polarity that has been chosen. If we want to set the PWM signal to be high 50% of the time, then we call `app_pwm_channel_duty_set` with the following parameters.

```c
    while (app_pwm_channel_duty_set(&PWM1, 0, 50) == NRF_ERROR_BUSY);
```

Make the servo sweep from its maximum angle to its minimum angle. This can be done by calling `app_pwm_channel_duty_set()` twice with a delay between the two calls in the main while-loop, as shown below

```c
    while (true)
    {
        while (app_pwm_channel_duty_set(&PWM2, 0, 0) == NRF_ERROR_BUSY);
        nrf_delay_ms(1000);
        while (app_pwm_channel_duty_set(&PWM2, 0, 0) == NRF_ERROR_BUSY);
        nrf_delay_ms(1000);
    }
    
```
The code snippet above sets the duty cycle to 0, you have to figure out the correct duty cycle values for the min and max angle. 

5. Modify the button handler from Task 3 so that you can set servo to its minimum and maximum angle by pressing the buttons on the nRF52 DK. 


### 5. UART

Use the nRF52s UART peripheral and the UART library (app_uart) to echo data sent from a terminal. If you do not already have a favorite terminal application, then I recommend using [Termite] (http://www.compuphase.com/software_termite.htm). The UART library is documented on this Infocenter page.

1. Create the function `uart_init` where you use the `APP_UART_FIFO_INIT` macro to initialize the UART module. The baudrate should be set to 115200, Flow Control should be disabled, no parity bits are used and the RX and TX buffers should be set to 256 in size. The UART pins of the nRF52 DK are listed on the backside of the board. See the UART example in the `\examples\peripheral\uart\pca10040\blank\ses` folder

2. Create the function uart_event_handler as shown below, we will modify it later in order to receive data from the terminal. 

```c
    void uart_event_handler(app_uart_evt_t * p_event)
    {
        /*
        You're not allowed to decleare variables inside switch-cases, 
        so any variables used in the switch-case must be declared here.
        */
        switch (p_event->evt_type)
        {
            case APP_UART_DATA_READY:
                /*  
                The received data is stored in a receive buffer and can be retrieved using app_uart_get.
                Data to be sent can be placed in the transmit buffer using app_uart_put.
                */
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
```

3. Create a function called `uart_print` which takes a uint8_t array as input and sends this array to the terminal using the `app_uart_put()` function. 
    *Hints:*
        - `app_uart_put()` places one character at the time in the uart transmit buffer, hence it should be called in a loop.
        - strings sent to the terminal should be terminated by `\r\n`.
        - The strlen() function is very useful to find the length of a string terminated by `\n`.

```c
static void uart_print(uint8_t data_string[])
{

}
```

4. Call the `uart_print` function in `main()` or in the button handler and verify that the message is shown in the terminal.



5. The APP_UART_DATA_READY event will be generated for each single byte that is received by the nRF52, which means that [app_uart_get](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v12.2.0/group__app__uart.html#gacddb5b7b711ef104f9eb181a13bc4503) must be called everytime the event is received. 

```c
    case APP_UART_DATA_READY:
        app_uart_get(&data_array[index]);
        index++;
        
        break;              
```

Since the [app_uart_get](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v12.2.0/group__app__uart.html#gacddb5b7b711ef104f9eb181a13bc4503) function takes the pointer to a uint8_t, we need an array to store the received bytes and and index variable to keep track of how many bytes we have received, i.e.

```c
    static uint8_t data_array[32];
    static uint8_t index = 0;              
```

Most terminals append the `\n` character, also known as the Line Feed character, to the end of the string that is sent. The `\n`  indicates that the next character should be printed on a newline. Therefore it makes sense to receive bytes until we see the `\n` character and then send the entire string back to the terminal using [app_uart_put](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v12.2.0/group__app__uart.html#ga2e4c8407274a151e72ed5a226529dc36). 

```c
  if (data_array[index - 1] == '\n') 
  {
    // Call app_uart_put to sent the bytes stored in data_array back to the terminal.
  }
```

The function app_uart_put used to place data in the UART's transmit buffer must be called in a for-loop if more that one byte is to be sent, i.e. 
```c
    for (uint32_t i = 0; i < strlen((const char *)data_array); i++)
    {
        while (app_uart_put(data_array[i]) != NRF_SUCCESS);
    }
```
After adding the array to hold the data and the index to keep track of hom many bytes we have received, adding the if statment and the for loop that calls app_uart_put(), the uart_event_handler function should look something like this:

```c
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
```
The memset function is used to clear the data_array since it is decleared as static, i.e. it will not erase the content in between the calls to `uart_event_handler`. If we do not set data_array to 0 and receive a string that is shorter than the last string we received, then some of the old data will still be stored in the array.

6. Send a text string from the terminal to the nRF52 DK and verify that it is echoed back to the terminal.

## 5.1: Temperature Sensor 
Use the die temperature sensor on the nRF52 to measure the temperature in the room. 

1. Create the function read_temperature() that returns the die temperature as a int32_t. 
    **Hint:** Take a look at the temperature example in the SDK before you start modifying your template example. You'll find it in examples\peripheral\temperature. 

2. Send the temperature data to your terminal application using the UART. 

Hint 1: Use [sprintf](https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm) to copy the content of a string into an array.

### 6. GPIOTE - GPIO Tasks and Events

Up on till now we've been controlling the GPIO pins of the nRF52832 by using the GPIO peripheral directly. This approach is works well for simple use cases, but it is also possible to control them using the Task and Event system, which allows you to take advantage of the PPI system.

The GPIO tasks and events (GPIOTE) module provides functionality for accessing GPIO pins using tasks and events, where each GPIOTE channel can be assigned to one pin. The GPIOTE block enables GPIOs to generate events on pin state change which can be used to carry out tasks through the PPI system.

The GPIOTE driver API (nrf_drv_gpiote.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_gpiote.html?cp=4_0_0_2_3).

#### Steps

1.  Include the *nrf_drv_gpiote.h* header in the *main.c* file and create the function gpiote_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize the GPIOTE driver.

3. Configure pin connected to LED1 on the nRF52 DK so that it is set as an an output and that the OUT task will toogle the pin. 
    (Hint 1: See the backside of the nRF52 Dk for the pinout).
    (Hint 2: Use the *GPIOTE_CONFIG_OUT_TASK_TOGGLE(init_high)* macro when configuring the pin). 

4. Enable the OUT task for the pin connected to LED1.

5. Add gpiote_init() in main() before the infinite while-loop. 

6. Comment out the references to nrf_gpio_out_cfg() and replace all calls to nrf_gpio_toogle() with nrf_drv_gpiote_out_task_trigger();

7. Compile the project and verify that the application timer and the buttons still toggles the LEDs.

### 7. TIMER - Timer/counter

In addtion to the RTCs, the nRF52832 also has several TIMER peripherals that are more accurate and have 32 bit COMPARE registers. 

 The TIMER can operate in two modes: timer and counter. Both run on the high-frequency clock source (HFCLK) and includes a four-bit (1/2X) prescaler that can divide the TIMER input clock from the HFCLK controller. The PPI system allows a TIMER event to trigger a task of any other system peripheral of the device.The PPI system also enables the TIMER task/event features to generate periodic output and PWM signals to any GPIO. The number of input/outputs used at the same time is limited by the number of GPIOTE channels.

The TIMER driver API (nrf_drv_timer.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_timer.html?cp=4_0_0_2_15).

#### Steps

1.  Include the *nrf_drv_timer.h* header in the *main.c* file and create the function timer_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Configure the timer to use the default config(NRF_DRV_TIMER_DEFAULT_CONFIG) that is given in *sdk_config.h*.

3. Open *sdk_config.h* in the *Configuration Wizard* view and check TIMER_ENABLED under nRF_Drivers. Change the settings so that the Timer frequency is set to 1MHz, the bit width is set to 32 and TIMER0 is the only enabled instance. 

4. Initialize the *nrf_drv_timer* driver. 

5. Use the *nrf_drv_timer_extended_compare()* function to set the compare value so that a Compare Event is generated every 0.5 seconds, enable the COMPARE0_CLEAR shortcut so that the timer is cleared for every Compare Event(creating a repeating timer) and enable interrupts on the COMPARE0 event.
    - (Hint 1: Use the *NRF_TIMER_CC_CHANNEL0* macro for the CC channel number).
    - (Hint 2: Use the *nrf_drv_timer_ms_to_ticks()* function to set the CC value).   
    - (Hint 3: Use *NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK* as the timer short mask) 

6. Turn on the timer by enabling it.

7. In the Timer callback function, that was passed as an argument during the initialization, you should call the *nrf_drv_gpiote_out_task_trigger()* function to trigger the OUT task of the LED1 pin manually. 

8. Add timer_init() in main() before the infinite while-loop.

9. Compile the template project and download it to the nRF52 DK. LED_1 shold now blink with a frequency of 0.5 Hz.

### 8. PPI - Programmable Peripheral Interconnect 

**Module description:** The Programmable peripheral interconnect (PPI) enables peripherals to interact autonomously with each
other using tasks and events independent of the CPU. The PPI allows precise synchronization between
peripherals when real-time application constraints exist and eliminates the need for CPU activity to
implement behavior which can be predefined using PPI.


The PPI driver API (nrf_drv_ppi.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_ppi.html?cp=4_0_0_2_7).

#### Steps

1.  Include the *nrf_drv_ppi.h* header in the *main.c* file and create the function ppi_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize the nrf_drv_ppi driver in the newley created ppi_init().

3. Allocate the first available PPI channel.

4. Assign the task and event endpoints of the allocated PPI channel so that a TIMER0 COMPARE[0] event triggers the OUT task of the LED_1 GPIO pin.
    - (Hint 1: Use the *nrf_drv_timer_event_address_get()* function to get the EEP(**E**vent **E**nd**P**oint).
    - (Hint 2: Use the *nrf_drv_gpiote_out_task_addr_get()* function to get the TEP(**T**ask **E**nd**P**oint). 

5. Enable the PPI channel. 

6. Disable the interrupt on the COMPARE0 event by setting the last parameter of *nrf_drv_timer_extended_compare()* to false in *timer_init()*.

7. Add *ppi_init()* in *main()* before the infinite while-loop, but after *gpiote_init()*.

8. Compile the template project and download it to the nRF52 DK. LED1 should now blink with the same frequency as before.

9. Start a debug session, press *Run(F5)* and then *Stop* to halt the CPU of the nRF52832. LED1 should continue to blink with the same frequency since we're bypassing the CPU using the PPI peripheral. 

10. Use the FORK feature of the PPI peripheral to assign a second Task Endpoint to the COMPARE0 event, specifically the OUT task of the pin connected to LED2 on the nRF52 DK.
    - (Hint 1: Use the *nrf_drv_ppi_channel_fork_assign()* to assign the second TEP).
    - (Hint 2: Remember to configure the pin connected to LED2 as an output and set the OUT task to toogle).   

11. Compile the template project and download it to the nRF52 DK. LED1 and LED2 should now blink with the same frequency as before.

12. Start a debug session, press *Run(F5)* and then *Stop* to halt the CPU of the nRF52832. LED1 and LED2 should continue to blink with the same frequency with the CPU halted. 

<!---
### 9. NFC -  Trigger a task when a NFC field is sensed. (Stretch goal)

Task: Use the sense functionality of the NFCT peripheral to detect the prescence of a NFC field and trigger a certain event e.g. toogle led or move the servo ( NFC controlled door lock).

### 10. Logging - Adding the logging module (Optional)

**Module description:** The logger module provides logging capability for your application. It is used by SDK modules and can be also utilized in application code.

The Logging module API (nrf_log.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/nrf_log.html?cp=4_0_0_3_20).

#### Steps

1. Include *nrf_log_ctrl.h* and *nrf_log.h*  Initialize the logging module by calling *NRF_LOG_INIT(NULL)* in *main()*;

2. Include the .c files in the *nRF_Log* and *nRF_Segger_RTT* in the Keil Project explorer. You also need to add *#define NRF_LOG_MODULE_NAME "APP"* to the top of *main()*, but it must be defined before the *nrf_log_ctrl.h* and *nrf_log.h* include-statements.

3. Open *sdk_config.h* in the *Configuration Wizard* view and check *NRF_LOG_ENABLED*, expand to display the logging settings and uncheck *NRF_LOG_DEFFERED*. Under *nrf_log_backend* uncheck *NRF_LOG_BACKEND_SERIAL_USES_UART* and check *NRF_LOG_BACKEND_SERIAL_USES_RTT*. 

4. Add *NRF_LOG_INFO("GPIOTE/TIMER/PPI/TWI Handson \r\n");* after calling *NRF_LOG_INIT()*, compile the project and flash the project to your nRF52 DK. 

5. Open J-Link RTT Viewer, use the configuration shown below and press *OK*. The string passed with the *NRF_LOG_INFO()* macro should show up in the RTT terminal.   

J-Link RTT Viewer Configuration  | 
------------ |
<img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/rtt_viewer_config.JPG" width="250"> |
--->