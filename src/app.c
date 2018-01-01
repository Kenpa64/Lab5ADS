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
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define INTR_DEVICE_ID      XPAR_PS7_SCUGIC_0_DEVICE_ID
#define INTL_DEVICE_ID      XPAR_PS7_SCUGIC_1_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define XADC_DEVICE_ID 		XPAR_XADCPS_0_DEVICE_ID
#define BUTTON_CHANNEL		1 // Input channel

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static XAdcPs XAdcInst;      /* XADC driver instance */
XAdcPs *XAdcInstPtr = &XAdcInst;
XScuGic INTCInst;
static XGpio Gpio; /* The Instance of the GPIO Driver */
int *axi_pointer = (int *) XPAR_VGA_CONTROL_0_S00_AXI_BASEADDR;
int InterruptFlag; /* Flag used to indicate that an interrupt has occurred */
int INTR_INT;
int INTL_INT;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void INT_Intr_Handler(void *baseaddr_p);
static void INT_Intl_Handler(void *baseaddr_p);
static int InterruptRSystemSetup(XScuGic *XScuGicInstancePtr);
static int InterruptLSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntlInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);
static int IntrInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);
static void Peripheral_Init		(void);


//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

void INT_Intr_Handler(void *InstancePtr) {
    // Disable GPIO interrupts
    XGpio_InterruptDisable(&GPIOInst, INTR_INT); //SWITCH or Pulsador
    // Ignore additional button presses
    if ((XGpio_InterruptGetStatus(&GPIOInst) & INTR_INT) != INTR_INT) {
        return;
    }

    /* Sets the interrupt flag */
    InterruptFlag = 1;

    (void) XGpio_InterruptClear(&GPIOInst, INTR_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&GPIOInst, INTR_INT);
}

void INT_Intl_Handler(void *InstancePtr) {
    // Disable GPIO interrupts
    XGpio_InterruptDisable(&GPIOInst, INTL_INT); //SWITCH or Pulsador
    // Ignore additional button presses
    if ((XGpio_InterruptGetStatus(&GPIOInst) & INTL_INT) != INTL_INT) {
        return;
    }

    /* Sets the interrupt flag */
    InterruptFlag = 1;

    (void) XGpio_InterruptClear(&GPIOInst, INTL_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&GPIOInst, INTL_INT);
}

int InterruptRSystemSetup(XScuGic *XScuGicInstancePtr) {
    // Enable interrupt
    XGpio_InterruptEnable(&GPIOInst, INTR_INT);
    XGpio_InterruptGlobalEnable(&GPIOInst);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
            (Xil_ExceptionHandler) XScuGic_InterruptHandler,
            XScuGicInstancePtr);
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

int InterruptLSystemSetup(XScuGic *XScuGicInstancePtr) {
    // Enable interrupt
    XGpio_InterruptEnable(&GPIOInst, INTL_INT);
    XGpio_InterruptGlobalEnable(&GPIOInst);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
            (Xil_ExceptionHandler) XScuGic_InterruptHandler,
            XScuGicInstancePtr);
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

int IntlInitFunction(u16 DeviceId, XGpio *GpioInstancePtr) {
    XScuGic_Config *IntlConfig;
    int status;

    // Interrupt controller initialisation
    IntlConfig = XScuGic_LookupConfig(DeviceId);
    status = XScuGic_CfgInitialize(&INTLInst, IntlConfig,
            IntlConfig->CpuBaseAddress);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Call to interrupt setup
    status = InterruptLSystemSetup(&INTLInst);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Connect GPIO interrupt to handler
    status = XScuGic_Connect(&INTLInst, INTL_GPIO_INTERRUPT_ID,
            (Xil_ExceptionHandler) INT_Intl_Handler, (void *) GpioInstancePtr);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Enable GPIO interrupts interrupt
    XGpio_InterruptEnable(GpioInstancePtr, 1);
    XGpio_InterruptGlobalEnable(GpioInstancePtr);

    // Enable GPIO and timer interrupts in the controller
    XScuGic_Enable(&INTLInst, INTL_GPIO_INTERRUPT_ID);

    return XST_SUCCESS;
}

int IntrInitFunction(u16 DeviceId, XGpio *GpioInstancePtr) {
    XScuGic_Config *IntrConfig;
    int status;

    // Interrupt controller initialisation
    IntrConfig = XScuGic_LookupConfig(DeviceId);
    status = XScuGic_CfgInitialize(&INTRInst, IntrConfig,
            IntrConfig->CpuBaseAddress);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Call to interrupt setup
    status = InterrupRtSystemSetup(&INTRInst);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Connect GPIO interrupt to handler
    status = XScuGic_Connect(&INTRInst, INTR_GPIO_INTERRUPT_ID,
            (Xil_ExceptionHandler) INT_Intr_Handler, (void *) GpioInstancePtr);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Enable GPIO interrupts interrupt
    XGpio_InterruptEnable(GpioInstancePtr, 1);
    XGpio_InterruptGlobalEnable(GpioInstancePtr);

    // Enable GPIO and timer interrupts in the controller
    XScuGic_Enable(&INTRInst, INTR_GPIO_INTERRUPT_ID);

    return XST_SUCCESS;
}

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

int main()
{
    unsigned int temperature_raw;
    unsigned int temperature_C;
    unsigned int max_temp = 40;

    int over_under = 0;

    int new_dataR;
    int old_dataR;
    int new_dataL;
    int old_dataL;

    /* Variable initialisation */
    InterruptFlag = 0;

    init_platform();

    // Initialise GPIO
    status = XGpio_Initialize(&GPIOInst, INTR_DEVICE_ID);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    status = XGpio_Initialize(&GPIOInst, INTL_DEVICE_ID);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Config GPIO channel 1 as input
    XGpio_SetDataDirection(&GPIOInst, SWITCH_CHANNEL, 0xFF); //El channel 2 estaba en 0x00

    // Initialize interrupt controller
    status = IntrInitFunction(INTR_DEVICE_ID, &GPIOInst);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    // Initialize interrupt controller
    status = IntlInitFunction(INTL_DEVICE_ID, &GPIOInst);
    if (status != XST_SUCCESS)
        return XST_FAILURE;

    *axi_pointer = 1; // Change the value, aqui deberia ir algo como "11 primeros bits temp_C, 11 siguientes bits max_temp, 1 bit over_under"

    // Reads initial value of the Interruptor
    new_dataR = XGpio_DiscreteRead(&GPIOInst, INTR_CHANNEL);
    old_dataR = new_dataR;

    new_dataL = XGpio_DiscreteRead(&GPIOInst, INTL_CHANNEL);
    old_dataL = new_dataL;

    while (1) {

        temperature_raw = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
        temperature_C = (int) XAdcPs_RawToTemperature(temperature_raw);

        /* Test for an interrupt produced by GPIO*/
        if (InterruptFlag == 1)
        {
            InterruptFlag = 0; // resets the interrupt flag
            new_dataR = XGpio_DiscreteRead(&GPIOInst, INTR_CHANNEL);
            new_dataL = XGpio_DiscreteRead(&GPIOInst, INTL_CHANNEL);

            /* Rising edge detection*/
            if ((new_dataR == 1) && (old_dataR == 0))
            {
                if(max_temp < 80)
                    max_temp ++;
            }

            if ((new_dataL == 1) && (old_dataL == 0))
            {
                if(max_temp > 0)
                    max_temp --;
            }
            old_dataL = new_dataL;
            old_dataR = new_dataR;
        }
        /* Displays the value of the counter on LED7 and LED6*/
        //XGpio_DiscreteWrite(&GPIOInst, LED_CHANNEL, counter);
        /* Enables or disables the 28-bit counter depending on internal counter variable*/
        /*if (counter == 0)
            *count28_pointer = 1;
        else
            *count28_pointer = 0;
            */
    }

    cleanup_platform();
    return 0;
}
