// TI File $Revision:  $
// Checkin $Date:  $
//###########################################################################
//
// FILE:	MDF_init.h
//
// TITLE:	Mano Del Fuego Peripheral initializations
//
//###########################################################################
// $TI Release:   $
// $Release Date:   $
//###########################################################################

#ifndef MDF_INIT_H
#define MDF_INIT_H

#include "DSP2833x_Device.h"
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File


extern void init_uart(void);	// uart initialization
extern void scib_xmit(int b);  // uart transmission function
extern int sci_receive(void);  // uart receive function
extern void init_dma(Uint32 ping_buff_offset, Uint32 pong_buff_offset);
extern void init_mcbspa(void);
extern void init_Xintf(void);
extern void init_uart(void);
extern void scib_xmit(int b);
extern int sci_receive(void);

#endif



