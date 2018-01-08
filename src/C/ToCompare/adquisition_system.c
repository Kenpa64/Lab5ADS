/*
 * adquisition_system.c
 *
 *  Created on: 13/06/2017
 *      Author: joan.marimon
 */
#include "adquisition_system.h"

void updateAlarm(unsigned int temp, unsigned int threshold) {
	unsigned int value;
	if(temp >= threshold)
		value = 1;
	else
		value = 0;

	*((unsigned int *) XPAR_AXI_GPIO_ALARM_BASEADDR) = value;
}

void updateThreshold(unsigned int threshold) {
	unsigned int value = threshold << 4; // value = threshold * 1280 / 80 = threshold*16
	if(value >= 1280) value = 1278; // Prevent threshold from disappearing from screen
	*((unsigned int *) (XPAR_AXI_GPIO_TEMP_BASEADDR + 0x00000008)) = value;
}

void updateTemp(unsigned int temp) {
	unsigned int value = temp << 4;
	*((unsigned int *) XPAR_AXI_GPIO_TEMP_BASEADDR) = value;
}

void printLowerCase(char c, int pos) {
	int h,w;

	int x_index = (c - 'a')*48;

	for(h = 0; h < 43; h++) {
	    for(w = 0; w < 6; w++) {
	        int p = (h+86)*2*1584 + w*8+x_index;
	    	int p2 = h*320 + w+pos*8;
	    	unsigned int data;
	    	data = FONTS[p];
	    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
	    }
	}
}
void printNumericChar(int num, int pos) {
    int h,w;
    int x_index = (15 + num)*48;

    for(h = 0; h < 43; h++) {
        for(w = 0; w < 6; w++) {
            int p = h*2*1584 + w*8+x_index;
		    int p2 = h*320 + w+pos*8;
		    unsigned int data;
		    data = FONTS[p];
		    *((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
		}
    }
}
void printUpperCase(char c, int pos) {
	int h,w;

		int x_index = (c - 'A')*48;

		for(h = 0; h < 43; h++) {
		    for(w = 0; w < 6; w++) {
		        int p = (h+43)*2*1584 + w*8 + x_index;
		    	int p2 = h*320 + w+pos*8;
		    	unsigned int data;
		    	data = FONTS[p];
		    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
		    }
		}
}
void printEqual(int pos) {
	int h,w;

		int x_index = (28)*48;

		for(h = 0; h < 43; h++) {
		    for(w = 0; w < 6; w++) {
		        int p = h*2*1584 + w*8+x_index;
		    	int p2 = h*320 + w+pos*8;
		    	unsigned int data;
		    	data = FONTS[p];
		    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
		    }
		}
}
void printDegree(int pos) {
	int h,w;

		int x_index = (32)*48;

		for(h = 0; h < 43; h++) {
		    for(w = 0; w < 6; w++) {
		        int p = h*2*1584 + w*8+x_index;
		    	int p2 = h*320 + w+pos*8;
		    	unsigned int data;
		    	data = FONTS[p];
		    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
		    }
		}
}

void printPoint(int pos) {
	int h,w;

		int x_index = (13)*48;

		for(h = 0; h < 43; h++) {
		    for(w = 0; w < 6; w++) {
		        int p = h*2*1584 + w*8+x_index;
		    	int p2 = h*320 + w+pos*8;
		    	unsigned int data;
		    	data = FONTS[p];
		    	*((volatile unsigned int *) (XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR + p2*4)) = data;
		    }
		}
}

void printString(char *c, int pos, int size) {
	int i;
	int p = pos;
	for(i = 0; i < size; i++) {
		if(c[i] >= 'A' && c[i] <= 'Z') {
			printUpperCase(c[i], p);
		}
		else if(c[i] >= 'a' && c[i] <= 'z') {
			printLowerCase(c[i], p);
		}
		else if(c[i] == '=') {
			printEqual(p);
		}
		else if (c[i] == '.') {
			printPoint(p);
		}
		p++;
	}
}

int XAdcPolledRead(u16 XAdcDeviceId, u32 *temp, float *tfloat)
{
	int Status;
	XAdcPs_Config *ConfigPtr;
	u32 TempRawData;

	float TempData;
	//float MaxData;
	//float MinData;
	XAdcPs *XAdcInstPtr = &XAdcInst;

	/*
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XAdcPs_LookupConfig(XAdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);

	/*
	 * Self Test the XADC/ADC device
	 */
	Status = XAdcPs_SelfTest(XAdcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);
	/*
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	TempRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
	TempData = XAdcPs_RawToTemperature(TempRawData);


	//TempRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MAX_TEMP);
	//MaxData = XAdcPs_RawToTemperature(TempRawData);

	//TempRawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MIN_TEMP);
	//MinData = XAdcPs_RawToTemperature(TempRawData & 0xFFF0);

	*temp = (unsigned int) TempData;
	*tfloat = TempData;

	return XST_SUCCESS;
}
