// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2_AR72614 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xfilter11x11_orig.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XFilter11x11_orig_CfgInitialize(XFilter11x11_orig *InstancePtr, XFilter11x11_orig_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XFilter11x11_orig_Start(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL) & 0x80;
    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XFilter11x11_orig_IsDone(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XFilter11x11_orig_IsIdle(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XFilter11x11_orig_IsReady(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XFilter11x11_orig_EnableAutoRestart(XFilter11x11_orig *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XFilter11x11_orig_DisableAutoRestart(XFilter11x11_orig *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_AP_CTRL, 0);
}

void XFilter11x11_orig_Set_width(XFilter11x11_orig *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_WIDTH_DATA, Data);
}

u32 XFilter11x11_orig_Get_width(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_WIDTH_DATA);
    return Data;
}

void XFilter11x11_orig_Set_height(XFilter11x11_orig *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_HEIGHT_DATA, Data);
}

u32 XFilter11x11_orig_Get_height(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_HEIGHT_DATA);
    return Data;
}

void XFilter11x11_orig_Set_src(XFilter11x11_orig *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_SRC_DATA, Data);
}

u32 XFilter11x11_orig_Get_src(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_SRC_DATA);
    return Data;
}

void XFilter11x11_orig_Set_dst(XFilter11x11_orig *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_DST_DATA, Data);
}

u32 XFilter11x11_orig_Get_dst(XFilter11x11_orig *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_DST_DATA);
    return Data;
}

void XFilter11x11_orig_InterruptGlobalEnable(XFilter11x11_orig *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_GIE, 1);
}

void XFilter11x11_orig_InterruptGlobalDisable(XFilter11x11_orig *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_GIE, 0);
}

void XFilter11x11_orig_InterruptEnable(XFilter11x11_orig *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_IER);
    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_IER, Register | Mask);
}

void XFilter11x11_orig_InterruptDisable(XFilter11x11_orig *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_IER);
    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_IER, Register & (~Mask));
}

void XFilter11x11_orig_InterruptClear(XFilter11x11_orig *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XFilter11x11_orig_WriteReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_ISR, Mask);
}

u32 XFilter11x11_orig_InterruptGetEnabled(XFilter11x11_orig *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_IER);
}

u32 XFilter11x11_orig_InterruptGetStatus(XFilter11x11_orig *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XFilter11x11_orig_ReadReg(InstancePtr->Control_BaseAddress, XFILTER11X11_ORIG_CONTROL_ADDR_ISR);
}

