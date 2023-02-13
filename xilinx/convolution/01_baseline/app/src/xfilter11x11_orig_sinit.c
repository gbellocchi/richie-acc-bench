// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2_AR72614 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xfilter11x11_orig.h"

extern XFilter11x11_orig_Config XFilter11x11_orig_ConfigTable[];

XFilter11x11_orig_Config *XFilter11x11_orig_LookupConfig(u16 DeviceId) {
	XFilter11x11_orig_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XFILTER11X11_ORIG_NUM_INSTANCES; Index++) {
		if (XFilter11x11_orig_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XFilter11x11_orig_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XFilter11x11_orig_Initialize(XFilter11x11_orig *InstancePtr, u16 DeviceId) {
	XFilter11x11_orig_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XFilter11x11_orig_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XFilter11x11_orig_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

