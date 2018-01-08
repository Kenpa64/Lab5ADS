/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2015; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          SETUP INSTRUCTIONS
*
*   This demonstration project illustrate a basic uC/OS-III project with simple "hello world" output.
*
*   By default some configuration steps are required to compile this example :
*
*   1. Include the require Micrium software components
*       In the BSP setting dialog in the "overview" section of the left pane the following libraries
*       should be added to the BSP :
*
*           ucos_common
*           ucos_osiii
*           ucos_standalone
*
*   2. Kernel tick source - (Not required on the Zynq-7000 PS)
*       If a suitable timer is available in your FPGA design it can be used as the kernel tick source.
*       To do so, in the "ucos" section select a timer for the "kernel_tick_src" configuration option.
*
*   3. STDOUT configuration
*       Output from the print() and UCOS_Print() functions can be redirected to a supported UART. In
*       the "ucos" section the stdout configuration will list the available UARTs.
*
*   Troubleshooting :
*       By default the Xilinx SDK may not have selected the Micrium drivers for the timer and UART.
*       If that is the case they must be manually selected in the drivers configuration section.
*
*       Finally make sure the FPGA is programmed before debugging.
*
*
*   Remember that this example is provided for evaluation purposes only. Commercial development requires
*   a valid license from Micrium.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <Source/os.h>
#include  <ucos_bsp.h>

#include <includes.h>
#include "xadcps.h"
#include "xstatus.h"
#include "stdio.h"
#include "xparameters.h"
#include "xgpio.h"
#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/*
*********************************************************************************************************
*                                            DEFINES
*********************************************************************************************************
*/

#define	APP_TASK_START_STK_SIZE	512u
#define	APP_TASK1_STK_SIZE		512u
#define APP_TASK2_STK_SIZE		512u
#define APP_TASK_START_PRIO		8u
#define APP_TASK1_PRIO			2u
#define APP_TASK2_PRIO			3u


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;							// Task Control Block (TCB).
static  OS_TCB       AppTask1TCB;
static  OS_TCB       AppTask2TCB;

static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE]; 	// Startup Task Stack
static  CPU_STK      AppTask1Stk[APP_TASK1_STK_SIZE];			// Task #1      Stack
static  CPU_STK      AppTask2Stk[APP_TASK2_STK_SIZE];			// Task #2      Stack

static  OS_MUTEX     AppMutexPrint, AppMutexAlarm, AppMutexTemp;									// App Mutex

static unsigned int temp, thresh, alarm, temp_raw;
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate      (void);
static  void  AppTaskStart       (void *p_arg);
static  void  AppTask1           (void *p_arg);
static  void  AppTask2           (void *p_arg);
static  void  AppPrintWelcomeMsg (void);
static  void  AppPrint           (char *str);
static  void  AppUpdateTemp (void);
static  void  AppUpdateAlarm (void);
static  void  AppPrintWelcomeMsg (void);
void  MainTask (void *p_arg);


/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
*********************************************************************************************************
*/

int main(void)
{

	UCOSStartup(MainTask);

	return 0;
}



/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define XADC_DEVICE_ID      XPAR_XADCPS_0_DEVICE_ID
#define BUTTON_CHANNEL      1 // Input channel

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static XAdcPs XAdcInst;      /* XADC driver instance */
XAdcPs *XAdcInstPtr = &XAdcInst;
static XGpio Gpio; /* The Instance of the GPIO Driver */

/*
*********************************************************************************************************
*                                       INIT TEMPERATURE PERIPHERAL
*********************************************************************************************************
*/

void Peripheral_Init()
{
    int Status;
    XAdcPs_Config *ConfigPtr;

    /* Initialize the GPIO driver. If an error occurs then exit */
        Status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
        if (Status != XST_SUCCESS) {
            return XST_FAILURE;
        }

        /*
         * Perform a self-test on the GPIO.  This is a minimal test and only
         * verifies that there is not any bus error when reading the data
         * register
         */
        XGpio_SelfTest(&Gpio);

        /*
         * Setup direction register so the switch is an input and the LED is
         * an output of the GPIO
         */
        XGpio_SetDataDirection(&Gpio, BUTTON_CHANNEL, 1);

        /*
         * Initialize the XAdc driver.
         */
        ConfigPtr = XAdcPs_LookupConfig(XADC_DEVICE_ID);


        XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr,
                    ConfigPtr->BaseAddress);

        /*
         * Self Test the XADC/ADC device
         */
        Status = XAdcPs_SelfTest(XAdcInstPtr);


        /*
         * Disable the Channel Sequencer before configuring the Sequence
         * registers.
         */
        XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 
*********************************************************************************************************
*/


void  MainTask (void *p_arg)
{
    OS_ERR       err;

    AppPrintWelcomeMsg();

	OSInit(&err);		/* Initialize uC/OS-III.                                */

    //init values

    //Init temp sensor
    
    Peripheral_Init();

    temp_raw = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
    temp = (int) XAdcPs_RawToTemperature(temperature_raw);
    thresh = 40;
    //updateTemp(temperature);
    *((unsigned int *) XPAR_AXI_GPIO_TEMP_BASEADDR) = temp << 4;
    //updateThreshold(threshold);
    *((unsigned int *) (XPAR_AXI_GPIO_TEMP_BASEADDR + 0x00000008)) = thresh << 4;
   // decide alarm value 
    if(temp >= thresh)
        alarm = 1;
    else
        alarm = 0;
    //updateAlarm(temperature, threshold);
    *((unsigned int *) XPAR_AXI_GPIO_ALARM_BASEADDR) = alarm;
    
   

	OSTaskCreate	((OS_TCB	*)&AppTaskStartTCB,
					(CPU_CHAR	*)"App Task Start",
					(OS_TASK_PTR )AppTaskStart,
					(void		*)0,
					(OS_PRIO	 )APP_TASK_START_PRIO,
					(CPU_STK 	*)&AppTaskStartStk[0],
					(CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10,
					(CPU_STK_SIZE)APP_TASK_START_STK_SIZE,
					(OS_MSG_QTY	 )0,
					(OS_TICK	 )0,
					(void 		*)0,
					(OS_OPT )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
					(OS_ERR *)&err);

	OSStart(&err);			/* Start multitasking (i.e. give control to uC/OS-II).  */

	while (1) {
	        ;
	    }
}

/*
*********************************************************************************************************
*                                        PRINT WELCOME THROUGH UART
*
* Description : Prints a welcome message through the UART.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : application functions.
*
* Note(s)     : Because the welcome message gets displayed before
*               the multi-tasking has started, it is safe to access
*               the shared resource directly without any mutexes.
*********************************************************************************************************
*/

static  void  AppPrintWelcomeMsg (void)
{
    UCOS_Print("\f\f\r\n");
    UCOS_Print("Micrium\r\n");
    UCOS_Print("uCOS-III\r\n\r\n");
    UCOS_Print("This application runs three different tasks:\r\n\r\n");
    UCOS_Print("1. Task Start: Initializes the OS and creates tasks and\r\n");
    UCOS_Print("               other kernel objects such as semaphores.\r\n");
    UCOS_Print("               This task remains running and printing a\r\n");
    UCOS_Print("               dot '.' every 100 milliseconds.\r\n");
    UCOS_Print("2. Task #1   : Prints '1' every 1-second.\r\n");
    UCOS_Print("3. Task #2   : Prints '2' every 2-seconds.\r\n\r\n");
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    OS_ERR    err;

	UCOS_Print("Task Start Created\r\n");

    OS_CSP_TickInit();                                          /* Initialize the Tick interrupt                        */

    Mem_Init();                                                 /* Initialize memory management module                  */
    Math_Init();                                                /* Initialize mathematical module                       */
    
    AppTaskCreate();                                            /* Create Application tasks                             */

    //Define three mutexes for three different processes (print and send new info)
    OSMutexCreate((OS_MUTEX *)&AppMutexPrint, (CPU_CHAR *)"My App. Mutex", (OS_ERR *)&err);
    OSMutexCreate((OS_MUTEX *)&AppMutexAlarm, (CPU_CHAR *)"My App. Update Alarm", (OS_ERR *)&err);
    OSMutexCreate((OS_MUTEX *)&AppMutexTemp, (CPU_CHAR *)"My App. Update Temp", (OS_ERR *)&err);

    while (1) {                                            /* Task body, always written as an infinite loop.       */

        OSTimeDlyHMSM(0, 0, 0, 100,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits 100 milliseconds.                              */

    	//AppPrint(".");                                          /* Prints a dot every 100 milliseconds.                 */
    }
}

/*
*********************************************************************************************************
*                                       CREATE APPLICATION TASKS
*
* Description : Creates the application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
	OS_ERR  err;


    OSTaskCreate((OS_TCB     *)&AppTask1TCB,                    /* Create the Task #1.                                  */
                 (CPU_CHAR   *)"Task 1",
                 (OS_TASK_PTR ) AppTask1,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK1_PRIO,
                 (CPU_STK    *)&AppTask1Stk[0],
                 (CPU_STK_SIZE) APP_TASK1_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_TASK1_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSTaskCreate((OS_TCB     *)&AppTask2TCB,                    /* Create the Task #2.                                  */
                 (CPU_CHAR   *)"Task 2",
                 (OS_TASK_PTR ) AppTask2,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK2_PRIO,
                 (CPU_STK    *)&AppTask2Stk[0],
                 (CPU_STK_SIZE) APP_TASK2_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_TASK2_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

/*
*********************************************************************************************************
*                                              TASK #1
*
* Description : This is an example of an application task that prints "1" every second to the UART.
*
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTask1 (void *p_arg)
{
	OS_ERR  err;

    unsigned int but, new_dataR = 0, old_dataR = 0, new_dataL = 0, old_dataL = 0;

	(void)p_arg;

    AppPrint("Task #1 Started\r\n");

    while (1) {                                            		/* Task body, always written as an infinite loop.       */
        //Read values from the buttons
        //1st approach
        but = *((volatile unsigned int*) XPAR_AXI_GPIO_BTN_BASEADDR); //XPAR_AXI_GPIO_0_BASEADDR?
        new_dataR = 0x00000001 & but;
        new_dataL= (0x00000002 & but)>>1;

        //second approach is using interruptions so we are not using at this moment
        if(new_dataL ^ old_dataL){
            if(thresh > 0){
                thresh--;
            }
            AppUpdateAlarm();
            old_dataL = new_dataL;
            
        }else if(new_dataR ^ old_dataR){
            if(thresh < 80){
                thresh++;
            }
            AppUpdateAlarm();
            old_dataR = new_dataR;
        }      

        OSTimeDlyHMSM(0, 0, 0, 100,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits for 0.1 second.                                  */

    	//AppPrint("1");                                          /* Prints 1 to the UART.                                */

    }
}


/*
*********************************************************************************************************
*                                               TASK #2
*
* Description : This is an example of an application task that prints "2" every 2 seconds to the UART.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTask2 (void *p_arg)
{
	OS_ERR  err;


	(void)p_arg;

    AppPrint("Task #2 Started\r\n");

    while (1) {                                            		/* Task body, always written as an infinite loop.       */
        
        AppUpdateTemp();
        AppPrint("Temperature updated: %d\n", temp);
        if(alarm = 1) AppPrint("Alarm ON\n"); 
        OSTimeDlyHMSM(0, 0, 0, 500,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits for 0.5 seconds.                                 */

    	                                          /* Prints 2 to the UART.                                */

    }
}


/*
*********************************************************************************************************
*                                            PRINT THROUGH UART
*
* Description : Prints a string through the UART. It makes use of a mutex to
*               access this shared resource.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : application functions.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppPrint (char *str)
{
	OS_ERR  err;
    CPU_TS  ts;


                                                                /* Wait for the shared resource to be released.         */
    OSMutexPend(	(OS_MUTEX *)&AppMutexPrint,
    				(OS_TICK )0u,                                            /* No timeout.                                          */
					(OS_OPT )OS_OPT_PEND_BLOCKING,                          /* Block if not available.                              */
					(CPU_TS *)&ts,                                            /* Timestamp.                                           */
					(OS_ERR *)&err);

    UCOS_Print(str);                                                 /* Access the shared resource.                          */

                                                                /* Releases the shared resource.                        */
    OSMutexPost( 	(OS_MUTEX *)&AppMutexPrint,
    				(OS_OPT )OS_OPT_POST_NONE,                              /* No options.                                          */
					(OS_ERR *)&err);
}


static void AppUpdateAlarm() {
    OS_ERR err;
    CPU_TS ts;

    OSMutexPend(    (OS_MUTEX *)&AppMutexAlarm,
                    (OS_TICK )0u,                                            /* No timeout.                                          */
                    (OS_OPT )OS_OPT_PEND_BLOCKING,                          /* Block if not available.                              */
                    (CPU_TS *)&ts,                                            /* Timestamp.                                           */
                    (OS_ERR *)&err);

    if(temp > thresh) alarm= 1;
    else alarm = 0;
    
    //updateThreshold(threshold);
    *((unsigned int *) (XPAR_AXI_GPIO_TEMP_BASEADDR + 0x00000008)) = thresh << 4;
    //Update alarm
    *((unsigned int *) XPAR_AXI_GPIO_ALARM_BASEADDR) = alarm;
    
    OSMutexPost(    (OS_MUTEX *)&AppMutexAlarm,
                    (OS_OPT )OS_OPT_POST_NONE,                              /* No options.                                          */
                    (OS_ERR *)&err);
}


static void AppUpdateTemp() {
    OS_ERR err;
    CPU_TS ts;

    OSMutexPend(    (OS_MUTEX *)&AppMutexTemp,
                    (OS_TICK )0u,                                            /* No timeout.                                          */
                    (OS_OPT )OS_OPT_PEND_BLOCKING,                          /* Block if not available.                              */
                    (CPU_TS *)&ts,                                            /* Timestamp.                                           */
                    (OS_ERR *)&err);

    //Get the new value 
    temp_raw = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
    temp = (int) XAdcPs_RawToTemperature(temperature_raw);

    if(temp > thresh) alarm= 1;
    else alarm = 0;

    //updateTemp(temperature);
    *((unsigned int *) XPAR_AXI_GPIO_TEMP_BASEADDR) = temp << 4;
     //Update alarm
    *((unsigned int *) XPAR_AXI_GPIO_ALARM_BASEADDR) = alarm;

    OSMutexPost(    (OS_MUTEX *)&AppMutexTemp,
                    (OS_OPT )OS_OPT_POST_NONE,                              /* No options.                                          */
                    (OS_ERR *)&err);
}