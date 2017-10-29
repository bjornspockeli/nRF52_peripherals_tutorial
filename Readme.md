# Simple PPI/Timer/GPIOTE Example

### Brief:

Short tutorial that shows how to configure a Timer to toogle a GPIO pin, in software and by using PPI and GPIOTE to bypass the CPU.


### Requirements
- nRF52 DK
- SDK v14.1.0
- Template Project found in nRF5_SDK_14.1.0_04a0bfd\examples\peripheral\template_project

## Tasks

In all the tasks we'll be using the SDK driver for the peripherals, i.e. nrf_drv_xxx.c, which can be found in nRF5_SDK_14.1.0_1dda907\components\drivers_nrf\. It is possible to use the peripherals at the register level using the Hardware Access Layer(HAL) for said peripheral, found in nRF5_SDK_13.0.0_04a0bfd\components\drivers_nrf\hal. THis will, however, not be covered in this tutorial. 

### Warm-up (consider removing this section as this is very tedious and after the first build only the changed source files will be compiled )

The template project includes all the peripheral libraries and drivers from the SDK, but we're only going to use a few, so to reduce the compile time and size of our project we'll temporarily remove them from the project. 

1. Find the *template_project* folder in nRF5_SDK_13.0.0_04a0bfd\examples\peripheral\

2. Create a copy of the  template_project folder and rename it to *gpiote_timer_ppi_handson*

3. Open the *template_pca10040.uvprojx* Keil project found in *gpiote_timer_ppi_handson\pca10040\blank\arm5_no_packs*

4. Select the folders *Board Support*, *nRF_Drivers*, *nRF_Libaries*, *nRF_Log* and *nRF_Segger_RTT* one by one in the Keil Project Explorer, left-click the selected folder, select *Options for Group xxxx* and uncheck *Include in Target Build*. 

5. Open the *sdk_config.h* file in the *None* folder in the Keil Project Explorer and click the *Configuration Wizard* tab. Uncheck all *nRF_Drivers*, *nRF_Libraries* and *nRF_Log*.

Remove files from Target Build | Uncheck modules in skd_config.h  | 
------------ |------------ |
<img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/warmup_uninclude_files.JPG" width="400"> | <img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/skd_config_uncheck.JPG" width="400"> |

### -1. Blink a LED

Goal: Blink a LED by keeping the CPU in a busy-wait loop 



### 0. Buttons - Button Handler Library

1. Normally we would need to add the Button Handler library, app_button.c, under nRF_Libraries in our project, but it is already been added in the template project. However,  we do need to include the app_button.h header at the top of main.c.

2. Next, create a static void function called buttons_init(), where you initalize the Button Handler library using [app_button_init](link to infocenter documentation). 

    Hints:
    - You will need to create a app_button_cfg_t struct for each button you configure.
    - The button pin number as well as the active state of the buttons can be seen on the backside of the nRF52 DK.
    - After initializing the Button Handler library with button configuration you will need to enable it, there should be a appropriate function.

3. 


### 1. GPIOTE - GPIO Tasks and Events

**Module description:** The GPIO tasks and events (GPIOTE) module provides functionality for accessing GPIO pins using tasks and events. Each GPIOTE channel can be assigned to one pin. The GPIOTE block enables GPIOs to generate events on pin state change which can be used to carry out
tasks through the PPI system.

The GPIOTE driver API (nrf_drv_gpiote.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_gpiote.html?cp=4_0_0_2_3).

#### Steps

1.  Include the *nrf_drv_gpiote.h* header in the *main.c* file and create the function gpiote_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize the GPIOTE driver.

3. Configure pin connected to LED1 on the nRF52 DK so that it is set as an an output and that the OUT task will toogle the pin. 
    (Hint 1: See the backside of the nRF52 Dk for the pinout).
    (Hint 2: Use the *GPIOTE_CONFIG_OUT_TASK_TOGGLE(init_high)* macro when configuring the pin). 

4. Enable the OUT task for the pin connected to LED1.

5. Add gpiote_init() in main() before the infinite while-loop. 

### 2. TIMER - Timer/counter

**Module description:** The TIMER can operate in two modes: timer and counter. Both run on the high-frequency clock source (HFCLK) and includes a four-bit (1/2X) prescaler that can divide the TIMER input clock from the HFCLK controller. The PPI system allows a TIMER event to trigger a task of any other system peripheral of the device.The PPI system also enables the TIMER task/event features to generate periodic output and PWM signals to any GPIO. The number of input/outputs used at the same time is limited by the number of GPIOTE channels.

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

### 3. PPI - Programmable Peripheral Interconnect

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

### 4. Logging - Adding the logging module (Optional)

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


### 5. Servo - Controlling a servo using the PWM driver(nrf_drv_pwm.c) or PWM library(app_pwm.c)

Task: 


### 6. NFC -  Trigger a task when a NFC field is sensed.

Task: Use the sense functionality of the NFCT peripheral to detect the prescence of a NFC field and trigger a certain event e.g. toogle led or move the servo ( NFC controlled door lock).

