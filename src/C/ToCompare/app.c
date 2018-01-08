/*
*********************************************************************************************************
*                                               uC/OS-III
*                                         The Real-Time Kernel
*
*                             (c) Copyright 2012, Micrium, Inc.; Weston, FL
*                                          All Rights Reserved
*
*
* File : APP.C
* By   : JPB
*
* LICENSING TERMS:
* ---------------
*           uC/OS-III is provided in source form for FREE short-term evaluation, for educational use or
*           for peaceful research.  If you plan or intend to use uC/OS-III in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-II for its use in your
*           application/product.   We provide ALL the source code for your convenience and to help you
*           experience uC/OS-III.  The fact that the source is provided does NOT mean that you can use
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can contact us at www.micrium.com, or by phone at +1 (954) 217-2036.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>

#include "xadcps.h"
#include "xstatus.h"
#include "stdio.h"
#include "xadcps.h"
#include "xparameters.h"
#include "xgpio.h"
#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

#include "adquisition_system.h"

#include "fonts.h"

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;                              /* Task Control Block (TCB).                         */
static  OS_TCB       AppTask1TCB;
static  OS_TCB       AppTask2TCB;

static  CPU_STK      AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE]; /* Startup Task Stack                                */
static  CPU_STK      AppTask1Stk[APP_CFG_TASK_1_STK_SIZE];         /* Task #1      Stack                                */
static  CPU_STK      AppTask2Stk[APP_CFG_TASK_2_STK_SIZE];         /* Task #2      Stack                                */

        OS_MUTEX    *AppMutexPrint;                                /* UART mutex.                                       */
        OS_MUTEX	*AppMutexAlarm;

static unsigned int temperature;
static unsigned int threshold;


static float temp_float;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate      (void);

static  void  AppMutexCreate     (void);

static  void  AppTaskStart       (void *p_arg);
static  void  AppTask1           (void *p_arg);
static  void  AppTask2           (void *p_arg);

static  void  AppPrint           (char *str);
static  void  AppPrintWelcomeMsg (void);
        void  print              (char *str);

static void AppUpdateAlarm(void);
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Startup Code.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR   err;
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif


    BSP_Init();

    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)CSP_DEV_NAME,
                (CPU_ERR  *)&cpu_err);
#endif

    CPU_IntDis();

    AppPrintWelcomeMsg();

    // Initialize values
    XAdcPolledRead(XADC_DEVICE_ID, (u32*) &temperature, &temp_float);
    threshold = 40;
    updateTemp(temperature);
    updateThreshold(threshold);
    updateAlarm(temperature, threshold);
    int i;
    int a;
    a = 0;
    for(i = 0; i <1280*20; i++) {
    	unsigned int val = 0x80808080;
    	if(a)
    		val = 0x80808080;
    	else
    		val = 0xFFFFFFFF;
    	if(i%(1280/4) == 0)
    		a = 1 - a;
    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + i*4)) = 0xFFFFFFFF;
    }

    	int w = 0;
    	int h = 0;
    	/*for(h = 0; h < 48; h++) {
    		for(w = 0; w < 24; w++) {
    			int p = h*1584*2 + w*8;
    			int p2 = h*1280 + w;
    			//unsigned int data;
    			//if(h == w)
    				//data = 0x00000000;
    			//else
    				//data = 0xFFFFFFFF;
    			unsigned int data = (((unsigned int)FONTS[p+3])<<24) + (((unsigned int)FONTS[p+2])<<16) + (((unsigned int)FONTS[p+1])<<8) + (unsigned int)FONTS[p];
    			*((volatile unsigned int *) XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2) = data;
    		}
    	}*/

    	for(h = 0; h < 43; h++) {
    	    for(w = 0; w < 6; w++) {
    	    			int p = (h+43)*2*1584 + (w+6)*8;
    	    			int p2 = h*320 + w;
    	    			unsigned int data;
    	    			if(h > 2 && h < 8)
    	    				data = 0x00000000;
    	    			else if(h >= 8 && h < 20 && w > 5 && w < 8)
    	    				data = 0x00000000;
    	    			else
    	    				data = 0xFFFFFFFF;
    	    			data = FONTS[p];
    	    			//data = (((unsigned int)FONTS[p+3])<<24) + (((unsigned int)FONTS[p+2])<<16) + (((unsigned int)FONTS[p+1])<<8) + (unsigned int)FONTS[p];
    	    			*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
    	    		}
    	    	}

    	printUpperCase('T',0);

    	printEqual(1);
    	printDegree(6);
    	printUpperCase('C', 7);

    	char *c = "Ttrig=";
    	printString(c,9,6);

    	unsigned int t = threshold*80/256;
    	printNumericChar(t%10,15);
    	t = t/10;
    	printNumericChar(t%10,16);
    	printDegree(17);
    	printUpperCase('C',18);



    OSInit(&err);                                               /* Initialize uC/OS-III.                                */

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"Startup Task",
                 (OS_TASK_PTR ) AppTaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_CFG_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-II).  */


    while (1) {
        ;
    }
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

	(void)p_arg;

    print("Task Start Created\r\n");

    OS_CSP_TickInit();                                          /* Initialize the Tick interrupt                        */

    Mem_Init();                                                 /* Initialize memory management module                  */
    Math_Init();                                                /* Initialize mathematical module                       */


#if (OS_TASK_STAT_EN > 0u)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    AppTaskCreate();                                            /* Create Application tasks                             */

    AppMutexCreate();                                           /* Create Mutual Exclusion Semaphores                   */

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

        OSTimeDlyHMSM(0, 0, 0, 100,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits 100 milliseconds.                              */

    	if(temp_float < 100) {
    		unsigned int t = (unsigned int)(temp_float*10);
    		int pos = 5;
    		while(t > 0) {
    			int n = t%10;
    			printNumericChar((char)n, pos);
    			if(pos == 5) {
    				printPoint(pos-1);
    				pos = pos-1;
    			}
    			t = t/10;
    			pos--;
    		}
    	}

    	unsigned int t = threshold;
    	    	printNumericChar(t%10,16);
    	    	t = t/10;
    	    	printNumericChar(t%10,15);
    	    	printDegree(17);
    	    	printUpperCase('C',18);

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
                 (OS_PRIO     ) APP_CFG_TASK_1_PRIO,
                 (CPU_STK    *)&AppTask1Stk[0],
                 (CPU_STK_SIZE) APP_CFG_TASK_1_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_CFG_TASK_1_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSTaskCreate((OS_TCB     *)&AppTask2TCB,                    /* Create the Task #2.                                  */
                 (CPU_CHAR   *)"Task 2",
                 (OS_TASK_PTR ) AppTask2,
                 (void       *) 0,
                 (OS_PRIO     ) APP_CFG_TASK_2_PRIO,
                 (CPU_STK    *)&AppTask2Stk[0],
                 (CPU_STK_SIZE) APP_CFG_TASK_2_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_CFG_TASK_2_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}


/*
*********************************************************************************************************
*                                       CREATE APPLICATION MUTEXES
*
* Description : Creates the application mutexes.
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

static  void  AppMutexCreate (void)
{
	OS_ERR  err;


    OSMutexCreate(AppMutexPrint, "Mutex Print UART", &err);     /* Creates the UART mutex.                              */
    OSMutexCreate(AppMutexAlarm, "Mutex Alarm", &err);
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
	unsigned int buttons, inc_btn, dec_btn, inc_prev, dec_prev;

	(void)p_arg;

    AppPrint("Task #1 Started\r\n");

    inc_prev = 0;
    dec_prev = 0;
    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

    	// Read buttons value
    	buttons = *((volatile unsigned int*) XPAR_AXI_GPIO_BTN_BASEADDR);
        dec_btn = 0x00000001 & buttons;
        inc_btn = (0x00000002 & buttons)>>1;

        // Check if buttons have been pressed
        if(inc_btn ^ inc_prev) { // XOR
        	if(inc_btn) {
        		AppPrint("Inc pressed\n");
        		if(threshold < 80)
        		    threshold++;
        		updateThreshold(threshold);
        		AppUpdateAlarm();
        	}
        	inc_prev = inc_btn;
        }
        else if(dec_btn ^ dec_prev) { // XOR
            if(dec_btn) {
            	AppPrint("Dec pressed\n");
            	if(threshold > 0)
            	    threshold--;
            	updateThreshold(threshold);

            	AppUpdateAlarm();
            }
            dec_prev = dec_btn;
        }

    	OSTimeDlyHMSM(0, 0, 0, 100,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits for 100 milliseconds.                            */

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
	int status;
	unsigned int temp_read;

	(void)p_arg;

    AppPrint("Task #2 Started\r\n");

    while (DEF_ON) {                                            /* Task body, always written as an infinite loop.       */

        OSTimeDlyHMSM(0, 0, 0, 500,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);                                     /* Waits for 2 seconds.                                 */

        status = XAdcPolledRead(XADC_DEVICE_ID,(u32*) &temp_read, &temp_float);
        if(status == XST_SUCCESS) {
            updateTemp(temp_read);
            temperature = temp_read;
            AppUpdateAlarm();
        }
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
    OSMutexPend( AppMutexPrint,
                 0u,                                            /* No timeout.                                          */
                 OS_OPT_PEND_BLOCKING,                          /* Block if not available.                              */
                &ts,                                            /* Timestamp.                                           */
                &err);

	print(str);                                                 /* Access the shared resource.                          */

                                                                /* Releases the shared resource.                        */
    OSMutexPost( AppMutexPrint,
                 OS_OPT_POST_NONE,                              /* No options.                                          */
                &err);
}

static void AppUpdateAlarm(void) {
	OS_ERR err;
	CPU_TS ts;

	OSMutexPend(AppMutexAlarm,
			    0u,
			    OS_OPT_PEND_BLOCKING,
			    &ts,
			    &err);

	updateAlarm(temperature, threshold);

	OSMutexPost(AppMutexAlarm,
			    OS_OPT_POST_NONE,
			    &err);

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
    print("\f\f\r\n");
    print("Micrium\r\n");
    print("uCOS-III\r\n\r\n");
    print("This application runs three different tasks:\r\n\r\n");
    print("1. Task Start: Initializes the OS and creates tasks and\r\n");
    print("               other kernel objects such as semaphores.\r\n");
    print("               This task remains running and printing a\r\n");
    print("               dot '.' every 100 milliseconds.\r\n");
    print("2. Task #1   : Prints '1' every 1-second.\r\n");
    print("3. Task #2   : Prints '2' every 2-seconds.\r\n\r\n");
}
