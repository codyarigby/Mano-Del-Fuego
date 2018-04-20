// TI File $Revision:  $
// Checkin $Date:  $
//###########################################################################
//
// FILE:	MDF_init.c
//
// TITLE:	MDF peripheral initialization functions
//
// DESCRIPTION:
//
//
//###########################################################################
// $TI Release:   $
// $Release Date:   $
//###########################################################################


#include "MDF_init.h"




void init_dma(Uint32 ping_buff_offset, Uint32 pong_buff_offset)
{
  EALLOW;
  DmaRegs.DMACTRL.bit.HARDRESET = 1;
  asm("     NOP");

  DmaRegs.PRIORITYCTRL1.bit.CH1PRIORITY = 0;

  /* DMA Channel 1 - McBSP-A Receive */
  DmaRegs.CH1.BURST_SIZE.all = 1;	// 2 16-bit words/burst (1 32-bit word) - memory address bumped up by 1 internally
  DmaRegs.CH1.SRC_BURST_STEP = 1;	// DRR2 must be read first & then DRR1. Increment by 1. Hence a value of +1. (This is a 2's C #)
  DmaRegs.CH1.DST_BURST_STEP = -1;	// Copy DRR2 data to address N+1 and DRR1 data to N. Hence -1 (32-bit read= read addr N+1 as MSB, then N as LSB)
  DmaRegs.CH1.TRANSFER_SIZE = 0;	// transfer one burst

  DmaRegs.CH1.SRC_TRANSFER_STEP = -1; // Decrement source address by 1 (from DRR1 back to DRR2) after processing a burst of data
  DmaRegs.CH1.DST_TRANSFER_STEP = -1; // After copying 1 32-bit word of L-C data (1 burst), move down to R-C data in a given buffer

  DmaRegs.CH1.SRC_ADDR_SHADOW = (Uint32) &McbspaRegs.DRR2.all;  // First read from DRR2
  DmaRegs.CH1.SRC_BEG_ADDR_SHADOW = (Uint32) &McbspaRegs.DRR2.all;
  DmaRegs.CH1.DST_ADDR_SHADOW = ping_buff_offset;               // First write to ping_buffer[1]
  DmaRegs.CH1.DST_BEG_ADDR_SHADOW = ping_buff_offset;

  DmaRegs.CH1.DST_WRAP_SIZE = 1;	  // After LEFT(1) and then RIGHT(2), go back to LEFT buffer
  DmaRegs.CH1.SRC_WRAP_SIZE = 0xFFFF; // Arbitary large value. We'll never hit this.....
  DmaRegs.CH1.DST_WRAP_STEP = 0;      // From starting address, move down 2 16-bit addresses to write nxt 32-bit word

  DmaRegs.CH1.CONTROL.bit.PERINTCLR = 1;
  DmaRegs.CH1.CONTROL.bit.SYNCCLR = 1;
  DmaRegs.CH1.CONTROL.bit.ERRCLR = 1;

  DmaRegs.CH1.MODE.bit.CHINTE = 1;          // Enable DMA channel interrupts
  DmaRegs.CH1.MODE.bit.CHINTMODE = 1;       // Interrupt at beginning of transfer //turn back to 0
  DmaRegs.CH1.MODE.bit.PERINTSEL = 15;		// McBSP MREVTA
  DmaRegs.CH1.MODE.bit.CONTINUOUS = 1;      // Enable continuous mode (continuously receives)// Enable interrupts from peripheral (to trigger DMA)
  DmaRegs.CH1.MODE.bit.PERINTE = 1;         // Enable interrupts from peripheral (to trigger DMA)
}


void init_mcbspa()
{
    EALLOW;
    McbspaRegs.SPCR2.all=0x0000;		// Reset FS generator, sample rate generator & transmitter
	McbspaRegs.SPCR1.all=0x0000;		// Reset Receiver, Right justify word

    McbspaRegs.SPCR1.bit.RJUST = 2;		// left-justify word in DRR and zero-fill LSBs

 	McbspaRegs.MFFINT.all=0x0;			// Disable all interrupts

    McbspaRegs.SPCR1.bit.RINTM = 0;		// McBSP interrupt flag to DMA - RRDY
	McbspaRegs.SPCR2.bit.XINTM = 0;     // McBSP interrupt flag to DMA - XRDY

    McbspaRegs.RCR2.all=0x0;			// Clear Receive Control Registers
    McbspaRegs.RCR1.all=0x0;

    McbspaRegs.XCR2.all=0x0;			// Clear Transmit Control Registers
    McbspaRegs.XCR1.all=0x0;

    McbspaRegs.RCR2.bit.RWDLEN2 = 5;	// 32-BIT OPERATION
    McbspaRegs.RCR1.bit.RWDLEN1 = 5;
    McbspaRegs.XCR2.bit.XWDLEN2 = 5;
    McbspaRegs.XCR1.bit.XWDLEN1 = 5;

    McbspaRegs.RCR2.bit.RPHASE = 0;		// single-phase frame
	McbspaRegs.RCR2.bit.RFRLEN2 = 0;	// Recv frame length = 1 word in phase2
	McbspaRegs.RCR1.bit.RFRLEN1 = 0;	// Recv frame length = 1 word in phase1

	McbspaRegs.XCR2.bit.XPHASE = 1;		// Dual-phase frame
	McbspaRegs.XCR2.bit.XFRLEN2 = 0;	// Xmit frame length = 1 word in phase2
	McbspaRegs.XCR1.bit.XFRLEN1 = 0;	// Xmit frame length = 1 word in phase1

	McbspaRegs.RCR2.bit.RDATDLY = 1;	// n = n-bit data delay, in DSP mode, X/RDATDLY=0
	McbspaRegs.XCR2.bit.XDATDLY = 1;    // DSP mode: If LRP (AIC23) = 0, X/RDATDLY=0, if LRP=1, X/RDATDLY=1
	                                    // I2S mode: R/XDATDLY = 1 always

    McbspaRegs.SRGR1.all=0x0001;		// Frame Width = 1 CLKG period, CLKGDV must be 1 as slave
                                        // SRG clocked by LSPCLK - SRG clock MUST be at least 2x external data shift clk

    McbspaRegs.PCR.all=0x0000;			// Frame sync generated externally, CLKX/CLKR driven
    McbspaRegs.PCR.bit.FSXM = 0;		// FSX is always an i/p signal
	McbspaRegs.PCR.bit.FSRM = 0;		// FSR is always an i/p signal
	McbspaRegs.PCR.bit.SCLKME = 0;

                                 // In normal DSP McBSP mode:
    McbspaRegs.PCR.bit.FSRP = 0;		// 0-FSRP is active high (data rx'd from rising edge)
	McbspaRegs.PCR.bit.FSXP = 0 ;       // 0-FSXP is active high (data tx'd from rising edge)

    McbspaRegs.PCR.bit.CLKRP  = 1;		// 1-Rcvd data sampled on rising edge of CLKR
	McbspaRegs.PCR.bit.CLKXP  = 0;      // 0- Tx data sampled on falling edge of CLKX
	McbspaRegs.SRGR2.bit.CLKSM = 1;		// LSPCLK is clock source for SRG

	McbspaRegs.PCR.bit.CLKXM = 0;		// 0-MCLKXA is an i/p driven by an external clock
    McbspaRegs.PCR.bit.CLKRM = 0;		// MCLKRA is an i/p signal

    McbspaRegs.SPCR2.all |=0x00C0;     	// Frame sync & sample rate generators pulled out of reset
    delay_loop();
	McbspaRegs.SPCR2.bit.XRST=1;       	// Enable Transmitter
    McbspaRegs.SPCR1.bit.RRST=1;		// Enable Receiver

    EDIS;
}



void Init_timer1(void)
{
	 // initialize timer1

	 EALLOW;
	 SysCtrlRegs.PCLKCR3.bit.CPUTIMER1ENCLK = 1;	// enable clock for timer peripheral (receives sysclkout)
	 InitCpuTimers();								// initialize cpu timer
	 // 22 is 44.1kHz
	 //Flanger = 800
	 //wah = 200
	 //ConfigCpuTimer(&CpuTimer1, 150, 200);			// 150Mhz, 100uS
	 // 5 is an octave
	 // 28 is minor 3rd
	 // 10.5 is 5th
	 ConfigCpuTimer(&CpuTimer1, 150, 10.5);			// 150Mhz, 100uS
	 CpuTimer1.RegsAddr->TCR.bit.TSS = 0;			// start the timer
	 EDIS;

}

void Init_gpioUI(void){

	EALLOW;

	// set Xint1 and Xint2 to alternate between GPIO0/GPIO1 and GPIO2/GPIO3 every DMA Audio transfer
	// xintSel == 0;  (GPIO0/GPIO1)    Xint1 -> effectsel = WAH,       		Xint2 -> effectsel = FLANGER,
	// xintSel == 1;  (GPIO2/GPIO3)    Xint1 -> effectsel = VOLSWELL,       Xint2 -> effectsel = BYPASS,


	// *** These GPIO's are assigned to buttons that make up the user interface *** //

	// *** Button Up *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO0 	= 0;   		// GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO0 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO0 	= 1;        // Enable pull-up

	// *** Button Down *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO1 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO1 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO1 	= 1;        // Enable pull-up

	// *** Button 1 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO2 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO2 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO2 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO2 	= 1;        // Enable pull-up

	// *** Button 2 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO3 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO3 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO3 	= 1;        // Enable pull-up

	// *** Button 3 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO4 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO4 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO4 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO4 	= 1;        // Enable pull-up

	// *** Button 4 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO6 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO6 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO6 	= 1;        // Enable pull-up

	// *** Button Interrupt (Xint1) *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO9 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO9 	= 0;        // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;        // Qual using 3 samples
	GpioCtrlRegs.GPAPUD.bit.GPIO9 	= 1;        // Enable pull-up


	// *** These GPIO's go to the LED decoder on the User Interface *** //

	// *** LED Ctrl0 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO7 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO7 	= 1;        // output
    GpioCtrlRegs.GPAPUD.bit.GPIO7   = 0;        // Disable pull-up

	// *** LED Ctrl1 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO8 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO8 	= 1;        // output
    GpioCtrlRegs.GPAPUD.bit.GPIO8   = 0;        // Disable pull-up


	// *** These GPIO's go directly to LEDs on the User Interface *** //

	// *** LED0 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO10 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO10 		= 1;        // output

	// *** LED11 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO11 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO11	 	= 1;        // output

	// *** LED11 *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 	= 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO12	 	= 1;        // output



	// *** Measurement GPIO *** //
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;        // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO12 	= 1;        // output


	// *** These GPIOS go to the stomp switches and have their own unique external interrupt *** //

	// *** Stomp0 (SDAA Xint3) *** //
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 	= 0;   		// GPIO
	GpioCtrlRegs.GPBDIR.bit.GPIO32 		= 0;        // input
	GpioCtrlRegs.GPBQSEL1.bit.GPIO32 	= 0;        // Qual using 3 samples
	GpioCtrlRegs.GPBPUD.bit.GPIO32 		= 0;        // Enable pull-up

	// *** Stomp1 (SCLA Xint4) *** //
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 	= 0;   		// GPIO
	GpioCtrlRegs.GPBDIR.bit.GPIO33 		= 0;        // input
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 	= 0;        // Qual using 3 samples
	GpioCtrlRegs.GPBPUD.bit.GPIO33 		= 0;        // Enable pull-up

	// *** Stomp2 (XRWn Xint5) *** //
	GpioCtrlRegs.GPBMUX1.bit.GPIO35 	= 0;   		// GPIO
	GpioCtrlRegs.GPBDIR.bit.GPIO35 		= 0;        // input
	GpioCtrlRegs.GPBQSEL1.bit.GPIO35 	= 0;        // Qual using 3 samples
	GpioCtrlRegs.GPBPUD.bit.GPIO35 		= 0;        // Enable pull-up

	// *** Stomp3 (XCS7 Xint6) *** //
	GpioCtrlRegs.GPBMUX1.bit.GPIO37 	= 0;   		// GPIO
	GpioCtrlRegs.GPBDIR.bit.GPIO37 		= 0;        // input
	GpioCtrlRegs.GPBQSEL1.bit.GPIO37 	= 0;        // Qual using 3 samples
	GpioCtrlRegs.GPBPUD.bit.GPIO37 		= 0;        // Enable pull-up



	GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 0xFF;   // Each sampling window is 510*SYSCLKOUT
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 0xFF;   // Each sampling window is 510*SYSCLKOUT



	// *** UI Button Interrupt *** //
	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL 	= 9;   // Xint1 is GPIO9
	XIntruptRegs.XINT1CR.bit.POLARITY 		= 0;   // Falling edge interrupt


	// *** UI Stomp Interrupts *** //
	GpioIntRegs.GPIOXINT3SEL.bit.GPIOSEL 	= 0;   // Xint3 is GPIO32
	XIntruptRegs.XINT3CR.bit.POLARITY 		= 3;   // interrupt on either edge

	GpioIntRegs.GPIOXINT4SEL.bit.GPIOSEL 	= 1;   // Xint4 is GPIO33
	XIntruptRegs.XINT4CR.bit.POLARITY 		= 3;   // interrupt on either edge

	GpioIntRegs.GPIOXINT5SEL.bit.GPIOSEL 	= 3;   // Xint5 is GPIO35
	XIntruptRegs.XINT5CR.bit.POLARITY 		= 3;   // interrupt on either edge

	GpioIntRegs.GPIOXINT6SEL.bit.GPIOSEL 	= 5;   // Xint6 is GPIO37
	XIntruptRegs.XINT6CR.bit.POLARITY 		= 3;   // interrupt on either edge





	// *** Enable External Interrupts *** //
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;        	// Enable Xint1
	XIntruptRegs.XINT3CR.bit.ENABLE = 1;        	// Enable Xint3
	XIntruptRegs.XINT4CR.bit.ENABLE = 1;        	// Enable Xint3
	XIntruptRegs.XINT5CR.bit.ENABLE = 1;        	// Enable Xint3
	XIntruptRegs.XINT6CR.bit.ENABLE = 1;        	// Enable Xint3

	EDIS;
}

// initialize the external interface
void init_Xintf(void)
{
	 EALLOW;
	 SysCtrlRegs.PCLKCR3.bit.XINTFENCLK = 1;
	 GpioCtrlRegs.GPBMUX1.all = 0xFFFFF000;
	 GpioCtrlRegs.GPCMUX1.all = 0xFFFFFFFF;
	 GpioCtrlRegs.GPCMUX2.all = 0xFFFF;
	 //GpioCtrlRegs.GPAMUX1.all  = 0x00000000;
	 GpioCtrlRegs.GPAMUX2.all |= 0xFF000000;
	 EDIS;

}


void init_uart(void)
{
	// 115200 baud rate in BRR
	EALLOW;

	GpioCtrlRegs.GPAPUD.bit.GPIO15   = 0;	   	// Enable pull-up for GPIO15 (SCIRXDB)
	GpioCtrlRegs.GPAPUD.bit.GPIO14   = 0;	   	// Enable pull-up for GPIO18 (SCITXDB)
	GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;		// Asynch input GPIO15
	GpioCtrlRegs.GPAMUX1.bit.GPIO15  = 2;   	// UART input GPIO15 (SCIRXDB) refer to page 95 of spru439 pdf
	GpioCtrlRegs.GPAMUX1.bit.GPIO14  = 2;   	// UART output GPIO15 (SCITXDB) refer to page 95 of spru439 pdf


	SysCtrlRegs.PCLKCR0.bit.SCIBENCLK = 1;

	ScibRegs.SCIHBAUD = 0x00;
	//ScibRegs.SCILBAUD = 80; //115200
	ScibRegs.SCILBAUD = 40;   //230400

	ScibRegs.SCICCR.bit.STOPBITS = 0; 			// 1 stop bit
	ScibRegs.SCICCR.bit.PARITY = 0; 			// odd parity, next line disables parity though
	ScibRegs.SCICCR.bit.PARITYENA = 0; 			// parity disabled! no parity
	ScibRegs.SCICCR.bit.LOOPBKENA = 0; 			// no loopback, idk what that is good for anyways
	ScibRegs.SCICCR.bit.ADDRIDLE_MODE = 0; 		// no
	ScibRegs.SCICCR.all = 0x07; 				// enables 8 bit mode async mode

	//In idle-line mode: write a 1 to TXWAKE, then write data to register
	//SCITXBUF to generate an idle period of 11 data bits

	//SCI sleep. The TXWAKE bit controls selection of the data-transmit feature, depending on which transmit
	//mode (idle-line or address-bit) is specified at the ADDR/IDLE MODE bit (SCICCR, bit 3). In a multiprocessor
	//configuration, this bit controls the receiver sleep function. Clearing this bit brings the SCI out of the sleep
	//mode.

	ScibRegs.SCICTL1.all 		= 0x23;
	ScibRegs.SCICTL1.bit.RXENA 		= 1;
	ScibRegs.SCICTL1.bit.TXENA 		= 1;
	//ScibRegs.SCICTL1.bit.SWRESET 	= 1;
	ScibRegs.SCICTL2.bit.RXBKINTENA = 1; // enable interrupts
	ScibRegs.SCICTL1.all 		= 0x23;
	//ScibRegs.SCICTL1.bit.RXENA = 0x01;  // enable receiver and transmitter, relinquish sci from reset


	//ScibRegs.SCICTL2.bit.TXEMPTY
	/*
	Transmitter empty flag. This flag’s value indicates the contents of the transmitter’s buffer register
	(SCITXBUF) and shift register (TXSHF). An active SW RESET (SCICTL1.5), or a system reset,
	sets this bit. This bit does not cause an interrupt request.
	0 Transmitter buffer or shift register or both are loaded with data
	1 Transmitter buffer and shift registers are both empty
	*/

	//ScibRegs.SCICTL2.bit.TXRDY
	/*
	Transmitter buffer register ready flag. When set, this bit indicates that the transmit data buffer
	register, SCITXBUF, is ready to receive another character. Writing data to the SCITXBUF
	automatically clears this bit. When set, this flag asserts a transmitter interrupt request if the
	interrupt-enable bit, TX INT ENA (SCICTL2.0), is also set. TXRDY is set to 1 by enabling the SW
	RESET bit (SCICTL1.5) or by a system reset.
	0 SCITXBUF is full
	1 SCITXBUF is ready to receive the next character
	*/

	//ScibRegs.SCIRXST.bit.RXRDY
	/*
	SCI receiver-ready flag. When a new character is ready to be read from the SCIRXBUF register,
	the receiver sets this bit, and a receiver interrupt is generated if the RX/BK INT ENA bit
	(SCICTL2.1) is a 1. RXRDY is cleared by a reading of the SCIRXBUF register, by an active SW
	RESET, or by a system reset.
	0 No new character in SCIRXBUF
	1 Character ready to be read from SCIRXBUF
	 */
}

// Transmit a character from the SCI
void scib_xmit(int b)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    ScibRegs.SCITXBUF=b;
}

// sci_receive function
// not used
int sci_receive(void)
{
	int a;
	while (ScibRegs.SCIFFRX.bit.RXFFST !=0);
	a = ScibRegs.SCIRXBUF.all;
	return a;

}






