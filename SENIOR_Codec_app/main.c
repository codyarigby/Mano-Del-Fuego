   // TI File $Revision:  $
// Checkin $Date:  $
//###########################################################################
//
// FILE:	Senior_Main.c
//
// TITLE:	DSP2833x McBSP to AIC23 Audio Codec Interface via DMA
//
// ASSUMPTIONS:
//
//    This program requires the DSP2833x header files.
//    As supplied, this project is configured for "boot to SARAM"
//    operation.  The 2833x Boot Mode table is shown below.
//
//       $Boot_Table:
//
//         GPIO87   GPIO86     GPIO85   GPIO84
//          XA15     XA14       XA13     XA12
//           PU       PU         PU       PU
//        ==========================================
//            1        1          1        1    Jump to Flash
//            1        1          1        0    SCI-A boot
//            1        1          0        1    SPI-A boot
//            1        1          0        0    I2C-A boot
//            1        0          1        1    eCAN-A boot
//            1        0          1        0    McBSP-A boot
//            1        0          0        1    Jump to XINTF x16
//            1        0          0        0    Jump to XINTF x32
//            0        1          1        1    Jump to OTP
//            0        1          1        0    Parallel GPIO I/O boot
//            0        1          0        1    Parallel XINTF boot
//            0        1          0        0    Jump to SARAM	    <- "boot to SARAM"
//            0        0          1        1    Branch to check boot mode
//            0        0          1        0    Boot to flash, bypass ADC cal
//            0        0          0        1    Boot to SARAM, bypass ADC cal
//            0        0          0        0    Boot to SCI-A, bypass ADC cal
//                                              Boot_Table_End$
//
// DESCRIPTION:
//
// This program will transmit and receive audio data between the DSP2833x McBSP and
// the TLV320AIC23B Stereo Audio Codec configured for DSP or I2S mode.  The following connections
// are requred -
//
// EXTERNAL CONNECTIONS:
//
// Audio Connections
//
//  AIC23               McBSP-A
//----------------------------------------------
//  DIN         ---      MDXA
//  LRCIN       ---      MFSXA
//  BCLK        ---      MCLKXA/MCLKRA (short)
//  DOUT        ---      MDRA
//  LRCOUT      ---      MFSRA
//  Feed Line In from CD/DVD/MP3 headphone jack or audio out
//  Feed HPOUT or Line Out to headphones or speakers respectively
//
// Control Signals
//
//  AIC23               McBSP-B
//----------------------------------
//  SDIN        ---     MDXB
//  SCLK        ---     MCLKXB
//  CSn         ---     MFSXB
//
//
// -----------------------------------------------------------------------------------------
//
//
// * Prior to building and loading code, first select microphone and digital audio interface
// (DSP mode or I2S mode) options using #define MIC and I2S_SEL directives.
//
// Data is transferred as follows:
//
// 1. RRDY signal triggers DMA interrupt as soon as McBSP-A receiver is enabled.
//    - This first interrupt is serviced and ignored because no data has been
//      received yet.
// 2. 32-bits of L-channel data is received in DRR2->DRR1, which is moved by
//    DMA to buffer (ping or pong - starts with ping_buffer[1]).
//    (Remember 32-bit reads read larger address first, then
//    smaller address second - i.e. if buffer is at 0xD000, a 32-bit read of
//    ping_buffer[1] would read MSB in 0xD001, then LSB in 0xD000).
// 3. Then 32-bits of R-channel data is received in DRR2->DRR1, which is moved
//    by DMA to ping(or pong)_buffer[513] (Again, MSB in ping/pong_buffer[513],
//    LSB in ping/pong_buffer[512])
// 4. DMA wraps around - receives L-channel data again, and places it in next 32-bit
//    buffer address. Likewise R-channel is received and data is written to next
//    buffer address (same as Step 3)
// 5. After 512 sets of 32-bit L and R-channel data have been received (1024 sets of
//    16-bit data), DMA channel 1 interrupts indicating buffer is filled.
// 6. Process data in buffer. Then kick off DMA channel 2 here to transmit data
//    from the same buffer from which data was received.
// 7. Change DMA destination address to pong_buff_offset if ping_buff_offset was
//    previous destination address and ping_buff_offset if pong_buff_offset was
//    previous destination address. As soon as interrupt is acknowledged, DMA
//    channel 1 will start receiving from the other buffer while DMA channel 2 is
//    transmitting the first buffer.
// 8. On DMA Channel 2, the transmit is opposite of receive (i.e. source = ping/
//    pong buffer offset destination = McBSP DXR2/DXR1 registers).
//    L/R-channel data moves from buffer to DXR2/DXR1 registers to be tx'ed to
//    AIC23. Then when complete, switch to other ping/pong buffer.
// 9. Steps 2-8 are repeated continuously via interrupts.
//
// Watch Variables:
//        ping_buffer (buffer of 1024 Uint32 values - audio data)
//        pong_buffer (buffer of 1024 Uint32 values - audio data)
//
//
//###########################################################################
// Original Author: Cody Rigby
//###########################################################################

#include "stdlib.h"
#include "math.h"
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
//#include "AIC23.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "MDF_init.h"
#include "UI.h"
#include <string.h>



// *** AIC23_DSP_SPI_control.c External Function Prototypes *** //
extern void init_mcbsp_spi();
extern void mcbsp_xmit (int a);
extern void aic23_init(int mic, int i2s_mode);

void mano_del_fuego(void);
void init_structs(void);
Uint16 absU16(Uint16 val);

// *** Interrupt Definitions *** //
interrupt void local_D_INTCH1_ISR(void); 	// Channel 1 Rx ISR
interrupt void local_D_INTCH2_ISR(void); 	// Channel 2 Tx ISR
interrupt void local_SCIRXINTB_ISR(void);   // Scib Rx ISR aleady defined in pie.h
interrupt void local_timer_ISR(void);		// timer isr
interrupt void local_XINT1_ISR(void);
interrupt void local_XINT3_ISR(void);
interrupt void local_XINT4_ISR(void);
interrupt void local_XINT5_ISR(void);
interrupt void local_XINT6_ISR(void);

//===============================================================
// SELECT AUDIO INPUT AND DIGITAL AUDIO INTERFACE OPTIONS HERE:
//===============================================================
#define MIC 		0      	// 0 = line input, 1 = microphone input
#define I2S_SEL 	0  		// 0 = normal DSP McBSP dig. interface, 1 = I2S interface


// *** Data Allocations, All effects structures go into the EFFECTRAM *** //
#pragma DATA_SECTION (ping_buffer, "DMARAML5"); // Place ping and pong in DMA RAM L5
#pragma DATA_SECTION (pong_buffer, "DMARAML5");
#pragma DATA_SECTION(ext_Buffer, "extSRAM6");
#pragma DATA_SECTION(echo_Buffer, "extSRAM6");
#pragma DATA_SECTION(pitch_shift_up, "EFFECTRAM6");
#pragma DATA_SECTION(pitch_shift_down, "EFFECTRAM6");
#pragma DATA_SECTION(sample, "EFFECTRAM6");
#pragma DATA_SECTION(sample_delay, "EFFECTRAM6");
#pragma DATA_SECTION(zcross_neg, "EFFECTRAM6");
#pragma DATA_SECTION(imu_dat, "EFFECTRAM6");
#pragma DATA_SECTION(flang, "EFFECTRAM6");
#pragma DATA_SECTION(dig_delay, "EFFECTRAM6");
#pragma DATA_SECTION(tremolo, "EFFECTRAM6");
#pragma DATA_SECTION(wah_svf, "EFFECTRAM6");
#pragma DATA_SECTION(bass_svf, "EFFECTRAM6");
#pragma DATA_SECTION(treble_svf, "EFFECTRAM6");
#pragma DATA_SECTION(zcross_return, "EFFECTRAM6");
#pragma DATA_SECTION(zcross_pitch, "EFFECTRAM6");


interrupt void local_D_INTCH1_ISR(void); 	// Channel 1 Rx ISR
interrupt void local_D_INTCH2_ISR(void); 	// Channel 2 Tx ISR
interrupt void local_SCIRXINTB_ISR(void);   // Scib Rx ISR aleady defined in pie.h
interrupt void local_timer_ISR(void);		// timer isr
interrupt void local_XINT1_ISR(void);
interrupt void local_XINT3_ISR(void);
interrupt void local_XINT4_ISR(void);
interrupt void local_XINT5_ISR(void);
interrupt void local_XINT6_ISR(void);

#pragma CODE_SECTION(local_D_INTCH1_ISR, "ramfuncs");
#pragma CODE_SECTION(local_D_INTCH2_ISR, "ramfuncs");
#pragma CODE_SECTION(local_SCIRXINTB_ISR, "ramfuncs");
#pragma CODE_SECTION(local_timer_ISR, "ramfuncs");
#pragma CODE_SECTION(local_XINT1_ISR, "ramfuncs");
#pragma CODE_SECTION(local_XINT3_ISR, "ramfuncs");
#pragma CODE_SECTION(local_XINT4_ISR, "ramfuncs");
#pragma CODE_SECTION(local_XINT5_ISR, "ramfuncs");
#pragma CODE_SECTION(local_XINT6_ISR, "ramfuncs");
#pragma CODE_SECTION(mano_del_fuego, "ramfuncs");
#pragma CODE_SECTION(init_structs, "ramfuncs");

// *** These are defined by the linker (see F28335.cmd) *** //
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;

//extern void Init_gpioUI(void);
//#pragma CODE_SECTION(Init_gpioUI, "ramfuncs");


Uint16 prevcountstart;

// *** main external audio buffer *** //
int16 ext_Buffer[32767];
Uint16 ext_Buffer_size = 32767; // and these pointers with 0x7FFF
int16 echo_Buffer[131071ULL];
unsigned long long int echo_Buffer_size = 32767; // and these pointers with 0x7FFF
// *******************************************************************************************************
// 							Pitch Shifting variables
// *******************************************************************************************************
struct PITCH_SHIFT{
	Uint16 freeze;
	Uint16 return_flag;     // flag goes off when a zero crossing is detected in the search window and its time to jump to a return zero crossing
	Uint16 zcross_i;		// index for the zero crossing fifo
	Uint16 zcross_detect_search;
	Uint16 zcross_detect;   // for detecting a positive zero crossing
	Uint16 Direction;       // pitch shifting up or down
	float PerUpper;      	// upper period limit
	float PerLower[10];     // lower period limit
	Uint16 pitch_prev;	    // previous pitch
	Uint16 pitch;
	float Per;           	// current period
	Uint16 count_delay; 	// delay counter (tracks current delay behind the sample pointer)
	Uint16 pitch_index; 	// index of the pitch shifted sample
	int16  pitch_samp_prev; // previous sample of pitch shifting sample
	Uint16 PerReset;        // reset the period for MCM control
	Uint16 delay_Ovfl;      // delay overflow value (anded with the counte delay +- 1) absolute max delay the pitch pointer can travel
	Uint16 delay_return;    // for both pitch shifting up and down. for up, this is the delay we approximately want to return to
	Uint16 search_window;   // the threshold for when we need to start looking for a pair of zero crossings
	float  Arpeggios[10][4];// the settings for arpeggios/ notes
} pitch_shift_down;

struct PITCH_SHIFT pitch_shift_up; // struct for pitch shifting up

float C;
// *** shared by both pitch shifter and delay based effects *** //
int16 ext_prev;
Uint16 ext_index;

Uint16 connect_count = 0;

// *** variables used in zcross calculation *** //
int16 closest_d;
int16 temp_d;
int16 closest_d;
Uint16 temp_index;
Uint16 temp_count_offset;

struct SAMPLE {
	   int16 samp[5];
	   Uint16 index[5];
	   Uint16 count;
};
struct SAMPLE sample;
struct SAMPLE sample_delay; // This structure is used to record the samples between a zero crossing in the
// return range.  When a zero crossing is detected for positive or negative, a function finds the pair of
// samples where the actual crossing occurs and gets the positive side index for pos zero crossing
// and the negative index for negative zero crossing

// *** this is the structure that is used for the zero crossing detector for the pitch shifter *** //
struct ZCROSS_NEG{
	   Uint16  pos_zcross_i; 		// index of the positive side of the positive zero crossing
	   Uint16  neg_zcross_i;        // index of the negative side of the negative zero crossing
	   Uint16  distance;			// distance in samples between the positive and the negative zero crossings
	   Uint16  peak;				// the peak value between the positive and negative zero crossing
}zcross_neg;

// *** Zcross FIFO *** //
Uint16 FIFO_size = 4;
struct ZCROSS_NEG zcross_return[4]; // Array of zero crossing to return to
struct ZCROSS_NEG zcross_pitch;      // the zero crossing that is captured when look for a return spot


// *** interpolation tools *** //
int16 ext_prev_inter;
int16 interpol_inc;

// *******************************************************************************************************
// 							Audio sample related variables
// *******************************************************************************************************

// ping pong buffers for right and left channels
Uint16 ping_buffer[2];  							// Note that Uint16 is used, not Uint32
Uint16 pong_buffer[2];								//
Uint16 p_buff_size = 2;
Uint16 * L_channel = &ping_buffer[0];				// This pointer points to the beginning of the L-C data in either of the buffers
Uint16 * R_channel = &ping_buffer[1];				// This pointer points to the beginning of the R-C data in either of the buffers
Uint32 ping_buff_offset = (Uint32) &ping_buffer[0]; // start address of ping buffer
Uint32 pong_buff_offset = (Uint32) &pong_buffer[0]; // start address of pong buffer
int16 * ch1_ptr; 									// ptr that points to ping/pong buffer that just received data

// *******************************************************************************************************
// 							Bluetooth / IMU sample related variables
// *******************************************************************************************************
// Address used to connect bluetooth modules
char			fastDataMode[4] = {'C', 'F', 'R' ,'\n' };
char 			address[15] = {'C', ',', '0', '0', '0', '6', '6', '6' ,'D', 'A', '0', '9', 'B', '7', '\n'};
static Uint16   wah_timer;
Uint16  		buffer_uart[128];					// Buffer to view samples from IMU over time
int 			uart_i = 0;								// uart buffer index
Uint16  		uart_step = 0; 						// if the interrupt receives a 0xA when uart_step is 0 then a transfer has begun!
int  			scib_rx;								// variable that holds the received BT data
int16			scib_gyro;
float			gyro_vol = 1.0;
float accel_mag;

struct IMU {
   int16  XaccelPrev;
   int16  Xaccel;
   int16  XgyroPrev;
   int16  Xgyro;
   float  Xa_velocity;
   float  Xa_velocity_prev;
   float  Xa_pos;
   float  Xg_velocity;
   float  Xg_velocity_prev;
   float  Xg_pos;
   float  Xaccel_real;
   float  Xaccel_real_prev;
} imu_dat;

// *******************************************************************************************************
// 							Delay Effects and Sine Buffer related variables
// *******************************************************************************************************
int16 gyro_pos;
int16 gyro_pos_prev;
int16 delay_const = 96;			// center of delay for the flanger/chorus effect
int16 delay = 512;  			// delay value for effect
Uint16 fb = 0;					// forward backward for delay related effects
Uint16 timer_delay 		= 100;
Uint16 timer_reset 		= 0;
Uint16 zcross_period 	= 0;

struct FLANG {
	Uint16 upper_delay;
	Uint16 lower_delay;
	float  speed[10];
	Uint16 delay;
	Uint16 delay_range[10];
	int16  delay_index;
} flang;

struct DELAY {
	Uint16 upper_delay;
	Uint16 lower_delay;
	Uint16 speed[10];
	Uint16 delay;
	int16  delay_index;
} dig_delay;

struct TREMOLO {
	float amplitude[10];
	float periodHigh;
	float periodLow;
	float period;
	float periodrange[10];
} tremolo;


// *******************************************************************************************************
// 							interrupt flag variables
// *******************************************************************************************************
Uint16 t1_flag = 0;				// timer 1 flag set in timer interrupt
Uint16 r_flag1 = 0; 			// flag set in interrupt to que data process
Uint16 r_flag2 = 0; 			// flag set in interrupt to que data process
Uint16 d_flag = 0;					// flag for uart (I think)
Uint16 first_interrupt = 1;   	// 1 indicates first interrupt
Uint16 activateEffect = 0;
Uint16 activateCounter = 0;
Uint16 deactivateCounter = 0;
Uint16 nextCounter = 0;
Uint16 nextCounter2 = 0;
Uint16 nextHold = 0;


// *******************************************************************************************************
// 							UI Variables
// *******************************************************************************************************
// Dummy Counter Variables
Uint16 Xint_count   	= 0;
Uint16 Button1_count 	= 0;
Uint16 Button2_count 	= 0;
Uint16 Button3_count 	= 0;
Uint16 Button4_count 	= 0;
Uint16 ButtonUp_count	= 0;
Uint16 ButtonDown_count = 0;

Uint16 zcrosstrig;

// *******************************************************************************************************
// 							State Variable Filter Variables
// *******************************************************************************************************
// State Variable Filter Stucture
// damp = 0.05, Q = 2*damp
// F controller by timer 0
struct SVF {
   float Fupper;		    // upper boundary of F
   float Flower;            // lower boundary of F
   float F;	     			// Tuning Coefficient  F1 = 2sin(pi*fc/fs)
   float Q[10];				// Tuning Coefficient  Q1 = 2sigma (10 different Q values to select from)
   float F_range[10];       // 10 different wah ranges
   float HPF;
   float BPF[2];
   float LPF[2];
};
struct SVF wah_svf;
struct SVF bass_svf;
struct SVF treble_svf;


// *** Dummy Variables used for visualization in debugging *** //
//float fIn;
//float dummy_buffer[1024];
//Uint16 dummy_index = 0;
//float *dptr = &dummy_buffer[0];


// *******************************************************************************************************
// 							Misc
// *******************************************************************************************************
Uint16 toggler				= 0;
Uint16 ii 					= 0;
Uint16 k 					= 0;
unsigned long long int kk	= 0;
Uint16 effectsel			= BYPASS;

//effect UI Control
bool   state_change_flag    = false;
Uint16 p1					= 0;
Uint16 p2					= 0;

Uint16 most_recent_peak     = 0;

void main(void)

{

   memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize);

   // Call Flash Initialization to setup flash waitstates
   // This function must reside in RAM
   InitFlash();
   InitSysCtrl();
   InitFlash();


   EALLOW;

   // Initalize GPIO:
   // For this example, enable the GPIO PINS for McBSP operation.
   InitMcbspGpio();

   init_Xintf();

   // Fill the buffers with dummy data
   for(k=0; k<p_buff_size; k++) { ping_buffer[k] = 0x0000; }
   for(k=0; k<p_buff_size; k++) { pong_buffer[k] = 0x0000; }
   for(k=0; k<ext_Buffer_size; k++) { ext_Buffer[k] = 0x0000; }
   for(kk=0; kk<echo_Buffer_size; kk++) { echo_Buffer[kk] = 0x0000; }
   ping_buff_offset++;    		// Start at location 1
   pong_buff_offset++;    		// Start at location 1

   // Clear all interrupts and initialize PIE vector table:
   // Disable CPU interrupts
   DINT;

   // Initialize PIE control registers to their default state.
   // The default state is all PIE interrupts disabled and flags
   // are cleared.
   // This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

   EALLOW;
   DINT;						// Disable interrupts again (for now)



   // Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

    init_mcbsp_spi();      		// Initialize McBSP-B as SPI Control
    Init_gpioUI();
    DELAY_US(900000L);

    aic23_init(MIC, I2S_SEL);  	// Set up AIC23 w/ McBSP-B
    DELAY_US(90L);
    DELAY_US(900000L);

	init_dma(ping_buff_offset, pong_buff_offset);					// Initialize the DMA before McBSP, so that DMA is ready to transfer the McBSP data
   	init_mcbspa();      		// Initalize McBSP-A for audio data transfers with audio codec
    delay_loop();				// Delay loop
    Init_timer1();				// Initialize timer 1
    EALLOW;

    //DmaRegs.CH1.CONTROL.bit.RUN = 1; // Start rx on Channel 1

    // Assign ISRS
    PieVectTable.SCIRXINTB 	= &local_SCIRXINTB_ISR; // ISR Triggered by RXRDY flag
	PieVectTable.DINTCH1 	= &local_D_INTCH1_ISR;	// Triggered by DMA Channel 1 Transfer
    //PieVectTable.DINTCH2 	= &local_D_INTCH2_ISR;  // Triggered by DMA Channel 2 Transfer
    PieVectTable.XINT13     = &local_timer_ISR;		// Triggered by Timer 1

    PieVectTable.XINT1 		= &local_XINT1_ISR;		// Triggered by GPIO9
    PieVectTable.XINT3 		= &local_XINT3_ISR;		// Triggered by GPIO32 (stomp0 SDAA)
    PieVectTable.XINT4 		= &local_XINT4_ISR;		// Triggered by GPIO33 (stomp1 SCLA)
    PieVectTable.XINT5 		= &local_XINT5_ISR;		// Triggered by GPIO35 (stomp2 XRWn )
    PieVectTable.XINT6 		= &local_XINT6_ISR;		// Triggered by GPIO34 (stomp3 XREADY)

    // Configure PIE interrupts
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable vector fetching from PIE block
	PieCtrlRegs.PIECTRL.all  |= 1;  	// Enable vector fetching from PIE block

	// Enable PIE to drive pulse from different PIE groups into the CPU
	//PieCtrlRegs.PIEACK.all = 0xFFFF;    // Enables PIE to drive a pulse into the CPU
	PieCtrlRegs.PIEACK.bit.ACK12 = 1;      // Enables PIE to drive a pulse into the CPU
	PieCtrlRegs.PIEACK.bit.ACK1  = 1;      // Enables PIE to drive a pulse into the CPU
	PieCtrlRegs.PIEACK.bit.ACK7  = 1;      // Enables PIE to drive a pulse into the CPU
	PieCtrlRegs.PIEACK.bit.ACK9  = 1;      // Enables PIE to drive a pulse into the CPU
	PieCtrlRegs.PIEACK.bit.ACK3 = 1;    // Enables PIE to drive a pulse into the CPU

	// The interrupt can be asserted in the following interrupt lines
	PieCtrlRegs.PIEIER12.bit.INTx1 = 1;  // Enable PIE Group12 INT1 (XINT3)
	PieCtrlRegs.PIEIER12.bit.INTx2 = 1;  // Enable PIE Group12 INT2 (XINT4)
	PieCtrlRegs.PIEIER12.bit.INTx3 = 1;  // Enable PIE Group12 INT3 (XINT5)
	PieCtrlRegs.PIEIER12.bit.INTx4 = 1;  // Enable PIE Group12 INT4 (XINT6)
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;  // Enable PIE Group 1 INT4 (XINT1)
	PieCtrlRegs.PIEIER7.bit.INTx1 = 1;	// Enable INTx.1 of INT7 (DMA CH1)
	PieCtrlRegs.PIEIER7.bit.INTx2 = 1;  // Enable INTx.2 of INT7 (DMA CH2)
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;  // Enable INTx.3 of INT9 (SCIB RXRDY Flag)
	//DmaRegs.CH1.CONTROL.bit.RUN = 1; // Start rx on Channel 1


// *******************************************************************************************************
// 							Initialize the Bluetooth Module
// *******************************************************************************************************
/*
init_uart();
scib_xmit('$');
DELAY_US(10000L);
scib_xmit('$');
DELAY_US(10000L);
scib_xmit('$');
DELAY_US(10000L);
*/


// set to fast data mode
init_uart();
for(ii = 0; ii < 4; ii++)
{
	scib_xmit(fastDataMode[ii]);
	DELAY_US(2000L);
}
/*
// send out the BT module address
for(ii = 0; ii < 15; ii++)
{
	scib_xmit(address[ii]);
	DELAY_US(2000L);
}
*/
// the BT  module will now connect to the MCM's bluetooth module
DELAY_US(10000L);


// *** Initialize the UI Display *** //
//Initialize_Board();


// *** Enable all interrupt channels *** //
// *** UI is disabled for this application
IER |= M_INT12;								// Enable Xint for Stomp Buttons
IER |= M_INT1;								// Enable Xint for UI Buttons
IER |= M_INT13;								// Enable Interrupt for timer 1
IER |= PIEACK_GROUP7;					    // Enable  INT7
IER |= PIEACK_GROUP9;						// Enable  INT9
EINT;      					        		// Global enable of interrupts

rx_flag = false;
Initialize_Board();

//CpuTimer1.RegsAddr->TCR.bit.TSS = 0; // start the timer
DmaRegs.CH1.CONTROL.bit.RUN = 1; // Start rx on Channel 1
EDIS;
delay = 0;
k = 0;
// **************** NEED TO WRITE A ROUTINE THAT INITIALIZES THE EFFECTS STRUCTURES ************************************//
init_structs();

imu_dat.XaccelPrev 	= 0;
imu_dat.Xaccel		= 0;
imu_dat.XgyroPrev	= 0;
imu_dat.Xgyro		= 0;
imu_dat.Xa_velocity = 0.0;
imu_dat.Xa_velocity_prev = 0.0;
imu_dat.Xa_pos = 0.0;
imu_dat.Xg_velocity = 0.0;
imu_dat.Xg_velocity_prev = 0.0;
imu_dat.Xg_pos = 0.0;
imu_dat.Xaccel_real = 0.0;
imu_dat.Xaccel_real_prev = 0.0;

//effectsel = absU16(0xfffe);

//effectsel = PITCHUP;
  	while(1) {
	// Code loops here all the time
  	mano_del_fuego();
}
}

//===========================================================================
// End of main()
//===========================================================================


interrupt void local_timer_ISR()
{


  //if(t1_flag == 1){
  //  t1_flag=1;
  //}
  //if(t1_flag==0){
  //  t1_flag=1;
  //}
  t1_flag = 1;						// set the flag for timer processing
  CpuTimer1Regs.TCR.bit.TIF = 1;	// acknowledge interrupt
  CpuTimer1Regs.TCR.bit.TSS = 0;	//
}



interrupt void local_SCIRXINTB_ISR(void) // SCI-B
{

	EALLOW;
	if(ScibRegs.SCIRXBUF.all == 0x0A && uart_step == 0)
	{
		uart_step++;
		//uart_i++;
	}
	else if(uart_step == 1)
	{
		scib_rx = ((ScibRegs.SCIRXBUF.all << 8) & 0xFF00);
		if((scib_rx & 0xFF00) == 0x0A00)
		{
			uart_step = 0;
		}
		else
		{
			uart_step++;
		}
	}
	else if(uart_step == 2)
	{
		scib_rx |= ScibRegs.SCIRXBUF.all;

		// if we were just getting accel data, uncomment the next two lines
		//uart_step = 0;
		//d_flag = 1;

		uart_step++;
	}
	else if(uart_step == 3)
	{
		scib_gyro = ((ScibRegs.SCIRXBUF.all << 8) & 0xFF00);
		if((scib_gyro & 0xFF00) == 0x0A00)
		{
			uart_step = 0;
		}
		else
		{
			uart_step++;
		}
	}
	else if(uart_step == 4)
	{
		scib_gyro |= ScibRegs.SCIRXBUF.all;
		uart_step = 0;
		d_flag = 1;

	}
	else
	{
		uart_step = 0;
	}

	//Uint16 activateEffect = 0;
	//Uint16 activateCounter = 0;
	//Uint16 deactivateCounter = 0;
	//rx_flag = true;
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ack

	//connect_count = 0x7fff;
	//GpioDataRegs.GPADAT.bit.GPIO11 = 1;

}




// INT7.1 -
interrupt void local_D_INTCH1_ISR(void)		// DMA Ch1 - McBSP-A Rx
{
    EALLOW;

    // this is to check if the DMA makes a transfer during the audio processing
    if(r_flag1)
    {
    	r_flag1 = 1;
    }

    //	When DMA first starts working on ping buffer, set the shadow registers
    //	to start at pong buffer next time and vice versa
    if(DmaRegs.CH1.DST_ADDR_SHADOW == ping_buff_offset)
	{
		DmaRegs.CH1.DST_ADDR_SHADOW = pong_buff_offset;
  		DmaRegs.CH1.DST_BEG_ADDR_SHADOW = pong_buff_offset;
  		ch1_ptr = ping_buff_offset; // set channel pointer to the previous ping/pong buffer because that is where the data was just recorded
	}
	else
	{
		DmaRegs.CH1.DST_ADDR_SHADOW = ping_buff_offset;
  		DmaRegs.CH1.DST_BEG_ADDR_SHADOW = ping_buff_offset;
  		ch1_ptr = pong_buff_offset; // set channel pointer to the previous ping/pong buffer because that is where the data was just recorded
	}
    // To receive more interrupts from this PIE group, acknowledge this interrupt
    r_flag1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;
    EDIS;
}



interrupt void local_XINT1_ISR(void)
{
	Xint_count++;
	DELAY_US(50000);
	if(GpioDataRegs.GPADAT.bit.GPIO0 == 0)
	{
		// "Button Up" has been pressed
		ButtonUp = true;
		ButtonUp_count++;
	}
	if(GpioDataRegs.GPADAT.bit.GPIO1 == 0)
	{
		// "Button Down" has been pressed
		ButtonDown = true;
		ButtonDown_count++;
	}
	if(GpioDataRegs.GPADAT.bit.GPIO2 == 0)
	{
		// "Button 1" has been pressed
		Button1 = true;
		Button1_count++;
	}
	if(GpioDataRegs.GPADAT.bit.GPIO3 == 0)
	{
		// "Button 2" has been pressed
		Button2 = true;
		Button2_count++;
	}
	if(GpioDataRegs.GPADAT.bit.GPIO4 == 0)
	{
		// "Button 3" has been pressed
		Button3 = true;
		Button3_count++;
	}
	if(GpioDataRegs.GPADAT.bit.GPIO6 == 0)
	{
		// "Button 4" has been pressed
		Button4 = true;
		Button4_count++;
	}

	State_Change();
	// *** ENTER NICKS FUNCTION *** //
	// Acknowledge this interrupt to get more from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void local_XINT3_ISR(void)
{

	// Stomp0 has been pressed
	Switch1 = true;
	// send 0x0 to the decoder
	State_Change();
	// Acknowledge this interrupt to get more from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
interrupt void local_XINT4_ISR(void)
{

	// Stomp1 has been pressed
	Switch2 = true;
	// send 0x1 to the decoder
	State_Change();
	// Acknowledge this interrupt to get more from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
interrupt void local_XINT5_ISR(void)
{

	// Stomp2 has been pressed
	Switch3 = true;
	// send 0x2 to the decoder
	State_Change();
	// Acknowledge this interrupt to get more from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
interrupt void local_XINT6_ISR(void)
{

	// Stomp3 has been pressed
	Switch4 = true;
	// send 0x3 to the decoder
	State_Change();
	// Acknowledge this interrupt to get more from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}



// ****************************** //
// *** THIS IS MANO DEL FUEGO *** //
// ****************************** //
void mano_del_fuego(void)
{

		if((*ch1_ptr) == 0x8000 || (*ch1_ptr) == 0x7FFF)
	    {
			GpioDataRegs.GPADAT.bit.GPIO12 = 1;
	    }
	    else
	    {
	    	GpioDataRegs.GPADAT.bit.GPIO12 = 0;
	    }



	    //activateEffect = 1;
		if(state_change_flag)
  		{
  			state_change_flag = false;
  			effectsel = Global_Board_State.FX[Global_Board_State.currentEffect].FX_index;
  			p1 = Global_Board_State.FX[Global_Board_State.currentEffect].value1;     // parameter 1
  			p2 = Global_Board_State.FX[Global_Board_State.currentEffect].value2;     // parameter 2
  			timer_reset = 1;
  		}

  		if(d_flag == 1)
  		{
  			// Take care of IMU calculations
  			imu_dat.XgyroPrev 			= imu_dat.Xgyro;
  			imu_dat.Xgyro 				= scib_gyro;
  			imu_dat.XaccelPrev 			= imu_dat.Xaccel;
  			imu_dat.Xaccel 				= scib_rx;

  			//imu_dat.Xaccel_real_prev 	= imu_dat.Xaccel_real;
  			//imu_dat.Xaccel_real 		+= (float)(imu_dat.Xaccel - imu_dat.XaccelPrev)*0.0000305185;
  			//imu_dat.Xa_velocity    		+= imu_dat.Xaccel_real;
  			//imu_dat.Xa_pos              += imu_dat.Xa_velocity;
  			//imu_dat.Xg_pos              += (float)(imu_dat.Xgyro)*0.0000305185;

  			//FOR VIEWING IMU DATA
  			//buffer_uart[uart_i] = scib_rx;
  			//buffer_uart[uart_i] = scib_gyro;
  			//buffer_uart[uart_i] = gyro_pos;

  			// logic that handles selecting between two different effects

  			if(scib_gyro & 0x0001)
			{
  			    int next = (Global_Board_State.currentEffect + 1) & 0x0003;
  			    if(next == 0) Switch1 = 1;
  			    else if (next == 1) Switch2 = 1;
  			    else if (next == 2) Switch3 = 1;
  			    else if (next == 3) Switch4 = 1;
  			    State_Change();

			}

  			//logic that handles activating an effect

			if((scib_rx & 0x0001) && activateEffect == 0)
			{
				activateCounter++;
				if(activateCounter > 6)
				{
					activateEffect = 1;
					activateCounter = 0;
				}
			}
			else
			{
				if(activateCounter > 0)
				{
				activateCounter--;
				}
				else{
					activateCounter = 0;
				}
			}
			if(!(scib_rx & 0x0001) && activateEffect == 1)
			{
				deactivateCounter++;
				if(deactivateCounter > 6)
				{
					activateEffect = 0;
					deactivateCounter = 0;
				}
			}
			else
			{
				if(deactivateCounter > 0)
				{
				deactivateCounter--;
				}
				else{
					deactivateCounter = 0;
				}
			}

			if(Global_Board_State.Flex_State == HOLD_MODE)
			{
			    if(activateEffect == 1)
			    {
			        effectsel = effectsel = Global_Board_State.FX[Global_Board_State.currentEffect].FX_index;
			    }
			    else if (activateEffect == 0)
			    {
			        effectsel = BYPASS;
			    }
			}

			else if(Global_Board_State.Flex_State == MOD_MODE)
			{
                effectsel = effectsel = Global_Board_State.FX[Global_Board_State.currentEffect].FX_index;
			}

            else if(Global_Board_State.Flex_State == PASS_MODE)
            {
                activateEffect = 1;
            }


  			if(effectsel == WAH)
  			{
  				// *** new way of doing wah, change it back if it sounds bad *** //

				if(imu_dat.Xaccel > (imu_dat.XaccelPrev+280) | imu_dat.Xaccel < (imu_dat.XaccelPrev-280)) // set threshold
				{
					//wah_svf.F += 0.0050;
					//imu_dat.Xaccel_real_prev = imu_dat.Xaccel_real;
					//imu_dat.Xaccel_real = imu_dat.Xaccel_real + (float)imu_dat.Xaccel -  (float)imu_dat.XaccelPrev;  //*0.0000305185;
					wah_svf.F += (wah_svf.Fupper)*((float)imu_dat.Xaccel -  (float)imu_dat.XaccelPrev)*0.0000305185*0.35;
					if(wah_svf.F > wah_svf.Fupper)
					{
						wah_svf.F = wah_svf.Fupper;
					}
					else if (wah_svf.F < wah_svf.Flower)
					{
						wah_svf.F = wah_svf.Flower;
					}
					//fb = 0;
				}
  			}
  			else if(effectsel == FLANGER)
  			{
  				if(imu_dat.Xgyro > 500)
  				{
  					fb = 0;
  				}
  				else if (imu_dat.Xgyro < -500)
  				{
  					fb = 1;
  				}
			timer_reset = 1;
  			}

  			else if(effectsel == VOLSWELL || effectsel == BASSBOOST || effectsel == TREMOLOO || effectsel == FUZZ || effectsel == DISTORTION || effectsel == CHORUS || effectsel == ECHO || effectsel == GYROWAH)
  			{

  				//imu_dat.XgyroPrev 	= imu_dat.Xgyro;
  			    //activateEffect = 1;
  			  if(imu_dat.Xgyro > 250 || imu_dat.Xgyro < -250)
  			  {
  				if(activateEffect != 0)
  				{
  					//gyro_vol += ((float)(imu_dat.Xgyro ))*0.0000031*0.07;
  					imu_dat.Xg_pos += ((float)(imu_dat.Xgyro ))*0.0000031*0.065;
  					//gyro_vol = imu_dat.Xg_pos;
  				}
  				if(imu_dat.Xg_pos < 0.0)
				{
					imu_dat.Xg_pos = 0.0;
				}
				else if (imu_dat.Xg_pos > 1.0)
				{
					imu_dat.Xg_pos = 1.0;
				}
  				gyro_vol = imu_dat.Xg_pos;
  				if(gyro_vol < 0.0)
  				{
  					gyro_vol = 0.0;
  				}
  				else if (gyro_vol > 1.0)
  				{
  					gyro_vol = 1.0;
  				}
  			  }
  			}

  			else if(effectsel == PITCHUP || effectsel == ARPEGG || effectsel == PITCHDOWN)
			{
				pitch_shift_up.pitch_prev = pitch_shift_up.pitch;

				if(imu_dat.Xgyro > 250 || imu_dat.Xgyro < -250)
				{
					if(activateEffect !=0 && effectsel == ARPEGG)
					{
						imu_dat.Xg_pos += ((float)(imu_dat.Xgyro ))*0.0000031*0.075;
					}
					else if ((activateEffect !=0 && effectsel == PITCHUP) || (activateEffect !=0 && effectsel == PITCHDOWN))
					{
						imu_dat.Xg_pos += ((float)(imu_dat.Xgyro ))*0.0000031*0.0185;
					}

					// *** Set limits to the gyro position *** //
					if(imu_dat.Xg_pos < 0.0)
					{
						imu_dat.Xg_pos = 0.0;
					}
					else if (imu_dat.Xg_pos > 1.0)
					{
						imu_dat.Xg_pos = 1.0;
					}

					if(effectsel == ARPEGG)
					{
						// *** choose index for pithc in arpeggiator *** //
						// *** deactivate effect so there is no pitch shifting for arpeggiator *** //
						if(activateEffect == 0)
						{
							imu_dat.Xg_pos = 0.0;
						}
						if(imu_dat.Xg_pos < 0.30)
						{
							pitch_shift_up.pitch = 0;
						}
						else if(imu_dat.Xg_pos < 0.60)
						{
							pitch_shift_up.pitch = 1;
						}
						else if(imu_dat.Xg_pos < 0.90)
						{
							pitch_shift_up.pitch = 2;
						}
						else if(imu_dat.Xg_pos < 1.00)
						{
							pitch_shift_up.pitch = 3;
						}
					}
					else if(effectsel == PITCHUP || effectsel == PITCHDOWN)
					{
						//C = 1.6*expf(-1.2*imu_dat.Xg_pos)+0.7;
						C = 3.9*expf(-1.65*imu_dat.Xg_pos)+0.295;
						pitch_shift_up.Per = pitch_shift_up.PerUpper - C*imu_dat.Xg_pos*(pitch_shift_up.PerUpper - pitch_shift_up.PerLower[p1]);

						if (pitch_shift_up.Per < pitch_shift_up.PerLower[p1])
						{
							pitch_shift_up.Per = pitch_shift_up.PerLower[p1];
						}
						if (pitch_shift_up.Per > pitch_shift_up.PerUpper)
						{
							pitch_shift_up.Per = pitch_shift_up.PerUpper;
						}
						pitch_shift_up.PerReset = 1;
					}

					// *** if the gyro position changes to a new section, reset the timer *** //
					if(pitch_shift_up.pitch != pitch_shift_up.pitch_prev)
					{
						pitch_shift_up.PerReset = 1;
					}
				}
			}
  			// *** reset the d_flag to wait for next imu data packet from MCM *** //
  			d_flag = 0;
  			// *** circular buffer uart_i *** //
  	  	  	uart_i = 0x03ff & (uart_i + 1);

  	  	    connect_count = 0x7fff;
  	  		GpioDataRegs.GPADAT.bit.GPIO11 = 1;
  	  		rx_flag = 1;


  		}

  		// Timer 1 interrupt prompts the modulation of effects
  		if(t1_flag == 1)
  		{
  			// *** Uncomment to measure timer *** //
  			//toggler = 0x0001 & ~(toggler);
  			//GpioDataRegs.GPADAT.bit.GPIO6 = toggler;

			if(effectsel == FLANGER)
			{
				if(fb == 1)
				{
					flang.delay++;
					if(flang.delay > 1024 + flang.delay_range[p1])
					{
						flang.delay = 1024 + flang.delay_range[p1];
					}
				}
				else if(fb == 0)
					{
						flang.delay--;
						if(delay < 1024 - flang.delay_range[p1])
						{
							flang.delay = 1024 - flang.delay_range[p1];
						}
					}
				if(timer_reset == 1)
				{

					CpuTimer1.RegsAddr->TCR.bit.TSS = 1;					// start the timer
					ConfigCpuTimer(&CpuTimer1, 150, flang.speed[p2]);			// 150Mhz, period defined by flanger structure
					CpuTimer1.RegsAddr->TCR.bit.TSS = 0;					// start the timer
					//flang.delay = flang.lower_delay + ((flang.upper_delay - flang.lower_delay) >> 1);
					timer_reset = 0;
				}
			}



			else if(effectsel == ARPEGG || effectsel == PITCHUP || effectsel == PITCHDOWN)
			{
				if(effectsel == PITCHDOWN)
				{
					if (pitch_shift_up.freeze == 0)
					{
						pitch_shift_up.count_delay = pitch_shift_up.delay_Ovfl & (pitch_shift_up.count_delay + 1); // 0x0000 - 0x0fff
					}
				}
				else
				{
					if (pitch_shift_up.freeze == 0)
					{
						pitch_shift_up.count_delay = pitch_shift_up.delay_Ovfl & (pitch_shift_up.count_delay - 1); // 0x0000 - 0x0fff
					}
				}

				if(activateEffect != 0)
				{
					if(pitch_shift_up.PerReset == 1 && effectsel == ARPEGG)
					{
						CpuTimer1.RegsAddr->TCR.bit.TSS = 1;				// start the timer
						ConfigCpuTimer(&CpuTimer1, 150, pitch_shift_up.Arpeggios[p1][pitch_shift_up.pitch]);	// 150Mhz, period = 100uS
						CpuTimer1.RegsAddr->TCR.bit.TSS = 0;				// start the timer
						pitch_shift_up.PerReset = 0;
					}
					else if(pitch_shift_up.PerReset == 1)
					{
						CpuTimer1.RegsAddr->TCR.bit.TSS = 1;				// start the timer
						ConfigCpuTimer(&CpuTimer1, 150, pitch_shift_up.Per);	// 150Mhz, period = 100uS
						CpuTimer1.RegsAddr->TCR.bit.TSS = 0;				// start the timer
						pitch_shift_up.PerReset = 0;
					}
				}
			}

			if(connect_count != 0)
			{
				connect_count--;
				if(connect_count == 0)
				{
					GpioDataRegs.GPADAT.bit.GPIO11 = 0;
					rx_flag = 0;
				}
			}
			t1_flag=0;
  		}

  		// Interrupt for the audio transfer
  		if(r_flag1==1)
  		{

  			// *** if effect_current == effect_sel && effectChange_flag == 1
  			// Change the corresponding effect structure in accordance with the signal chain effect parameter
  			// effectChange_flag = 0

  			// *** Audio In Audio Out with lots of interpolation *** //
  			ext_prev_inter = ext_prev;
			interpol_inc = ((*ch1_ptr - ext_prev)>>2);

			ext_prev_inter += interpol_inc;
			ext_Buffer[ext_index] = ext_prev_inter;
			sample.samp[1]  = ext_prev_inter;
			sample.index[1] = ext_index;
			ext_index = 0x7fff & (ext_index+1);

			ext_prev_inter += interpol_inc;
			ext_Buffer[ext_index] = ext_prev_inter;
			sample.samp[2] = ext_prev_inter;
			sample.index[2] = ext_index;
			ext_index = 0x7fff & (ext_index+1);

			ext_prev_inter += interpol_inc;
			ext_Buffer[ext_index] = ext_prev_inter;
			sample.samp[3] = ext_prev_inter;
			sample.index[3] = ext_index;
			ext_index = 0x7fff & (ext_index+1);

			ext_Buffer[ext_index] = *ch1_ptr;
			sample.samp[4] = *ch1_ptr;
			sample.index[4] = ext_index;



  			// *** State Variable Filter output *** //
  			// *** Used for the 'wah' effect *** //
  			if(effectsel == WAH)
			{
				wah_svf.HPF    = ((float)(*ch1_ptr))*0.000031 - wah_svf.LPF[0] - (wah_svf.Q[p1])*(wah_svf.BPF[0]); // index Q using Nick's structure
				wah_svf.BPF[1] = (wah_svf.F)*(wah_svf.HPF) + wah_svf.BPF[0];
				wah_svf.LPF[1] = (wah_svf.F)*(wah_svf.BPF[1]) + wah_svf.LPF[0];

				// *** Current values become past values *** //
				wah_svf.BPF[0] = wah_svf.BPF[1];
				wah_svf.LPF[0] = wah_svf.LPF[1];


				 McbspaRegs.DXR2.all = (int16)((wah_svf.BPF[1])*32767.0);
				 McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;

				 // *** dummy buffer *** //
				 //dummy_buffer[dummy_index] = wah_svf.F;
				 //dummy_index = 0x03ff & (dummy_index + 1);
			}
  			else if (effectsel == FLANGER)
  			{
  				// *** Flanger Effect *** //
  				flang.delay_index = 0x7fff & (ext_index - flang.delay);
				McbspaRegs.DXR2.all = (ext_Buffer[flang.delay_index] >> 1) + (ext_Buffer[ext_index] >> 1);
				McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
  			}

  			else if (effectsel == VOLSWELL)
  			{
  				float fIn = ((float)(*ch1_ptr))*gyro_vol;
  				McbspaRegs.DXR2.all = (int16)(fIn);
  				McbspaRegs.DXR1.all = (int16)(fIn);

  				// *** dummy buffer *** //
  				//dummy_buffer[dummy_index] = gyro_vol;
  				//dummy_index = 0x03ff & (dummy_index + 1);
  			}

  			else if (effectsel == GYROWAH)
  			{
  			//////   Gyro Wah ///////

			wah_svf.F = wah_svf.Flower + gyro_vol*(wah_svf.Fupper - wah_svf.Flower);
			wah_svf.Q[0] = 0.70 - gyro_vol*(0.70 - 0.10);
			if(wah_svf.F > wah_svf.Fupper)
			{
				wah_svf.F = wah_svf.Fupper;
			}
			else if (wah_svf.F < wah_svf.Flower)
			{
				wah_svf.F = wah_svf.Flower;
			}

			wah_svf.HPF    = ((float)(*ch1_ptr))*0.000031 - wah_svf.LPF[0] - (wah_svf.Q[0])*(wah_svf.BPF[0]); // index Q using Nick's structure
			wah_svf.BPF[1] = (wah_svf.F)*(wah_svf.HPF) + wah_svf.BPF[0];
			wah_svf.LPF[1] = (wah_svf.F)*(wah_svf.BPF[1]) + wah_svf.LPF[0];

			// *** Current values become past values *** //
			wah_svf.BPF[0] = wah_svf.BPF[1];
			wah_svf.LPF[0] = wah_svf.LPF[1];


			 McbspaRegs.DXR2.all = (int16)((wah_svf.BPF[1])*32767.0);
			 McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;

  			}
  			else if (effectsel == BYPASS)
  			{




  				// *** View the IMU Data real time from the DAC *** //
  				//McbspaRegs.DXR2.all = imu_dat.Xaccel;
  				//McbspaRegs.DXR2.all = imu_dat.Xgyro;
  				//McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
  				McbspaRegs.DXR2.all = *ch1_ptr;
  				McbspaRegs.DXR1.all = *ch1_ptr;
			}
  		    else if (effectsel == DISTORTION)
            {
                float sgn = ((*ch1_ptr) < 0) ? -1.0 : 1.0;
                float x =  (float)*ch1_ptr;
                float dist;
                //dist = x;
                //dist = sgn*(1.0 - exp(0.004*x*sgn));
                //dist = 16000 * tanh(x/2000);
                //dist = 16000 * atan(x/3000);
                //dist = 16000 * (x/8000)/(1 + exp(-x/8000));
                //dist = 16000 * (x/4000)/sqrt(1 + (x/4000)*(x/4000));
                //dist = 32000 * ((x/3000)/(1 + abs(x/3000)));
                //dist = 0.001*dist + 0.999*x;
                //dist = x + x*x/10000 + x*x*x/1000000000;
                int j;
                //int levels = gyro_vol * 20;
                dist = x;
                for(j = 0; j < (1 + p1); j++)
                {
                    dist = 16000.0 * atan(x/(16000.0 - (500.0 * p2)));
                    x = dist;
                }
                dist = dist * (1.0 + gyro_vol);
                //dist = 32000*(3*(x/32000)/2 * (1 - (x/32000)*(x/32000)/3));
                //dist = 32000*(abs(2*x/32000) - (x/32000)*(x/32000)) * sgn;
                if(dist > 32767.0) dist = 32767.0;
                if(dist < -32767.0) dist = -32767.0;



                McbspaRegs.DXR2.all = (int16)(dist);
                McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == BASSBOOST)
            {
                static float x = 0;
                static float x_1 = 0;
                static float x_2 = 0;
                static float y1 = 0;
                static float y1_1 = 0;
                static float y1_2 = 0;
                float y;

                x_2 = x_1;
                x_1 = x;
                x = (float)*ch1_ptr;

                y1_2 = y1_1;
                y1_1 = y1;

                float fc = 25.0;
                float fb = 2.5*(p1+1);
                float fs = 48000.0;
                float G = (38.0 + (p2+1))*gyro_vol;
                float d = -1*cos(2*3.14*fc/fs);
                float V0 = pow(10, G/20);
                float H0 = V0-1.0;
                float ab = (tan(3.14*fb/fs) - 1)/(tan(3.14*fb/fs) + 1);


                y1 = -1.0*ab*x + d*(1.0 - ab)*x_1 + x_2 - d*(1.0-ab)*y1_1 + ab*y1_2;
                y = H0/2 * (x - y1) + x;

                McbspaRegs.DXR2.all = (int16)(y);
                McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == ECHO)
            {
                static unsigned long long int echo_index = 0;
                unsigned long long int phase_delay[4] = {16383/4*(p1+1), 32767/4*(p1+1), 65535/4*(p1+1), 131071/4*(p1+1)};
                int16 phase_out = ((*ch1_ptr)/15)*5;
                int j;
                for(j = 0; j < 4; j++)
                {
                    long long int phase_index = (echo_index - phase_delay[j]) & 0x1FFFF;
                    if(phase_index < 0) phase_index += 131072;
                    phase_out += (echo_Buffer[phase_index]/15)*(4-j)*gyro_vol*(1.0 + 0.1*p2);
                }

                echo_Buffer[echo_index] = phase_out;
                echo_index++;
                echo_index &= 0x1FFFF;
                McbspaRegs.DXR2.all = phase_out;
                McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == CHORUS)
            {
                int phase_delay[4] = {1600*(p2+1)/2, 3200*(p2+1)/2, 4800*(p2+1)/2, 6400*(p2+1)/2};
                int16 phase_out = 0;
                int j;
                for(j = 0; j < 4; j++)
                {
                    int phase_index = ext_index - phase_delay[j];
                    if(phase_index < 0) phase_index += 32768;
                    phase_out += (ext_Buffer[phase_index]/15*(4-j));
                }
                phase_out = ((2 + 0.2*p1)*gyro_vol)*phase_out + (*ch1_ptr)/15*(15 - 10*gyro_vol);
                McbspaRegs.DXR2.all = phase_out;
                McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == TREMOLOO)
            {
                  static float trem_index = 0.0;
                  float trem_len = (p1/3.0 + 1.0)*(48000.0 - 40000.0 * gyro_vol);
                  float trem_out = (float)(*ch1_ptr);
                  trem_out *= ((0.1 * (p2 + 1))*cos(2*3.14*trem_index/trem_len) + (1.0 - (0.1 * (p2 + 1))));
                  trem_index += 1.0;
                  if(trem_index > trem_len) trem_index = 0.0;
                  McbspaRegs.DXR2.all = (int16)trem_out;
                  McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == FUZZ)
            {
                  float gain = 16.0 - 15.0*(gyro_vol*gyro_vol*gyro_vol*gyro_vol);
                  int16 fuzz_out = (int16)(gain * (*ch1_ptr));
                  int16 thresh_high = (10000 - p1*1000) + (int16)(15000.0 * gyro_vol);
                  int16 thresh_low = -1 * ((10000 - p2*1000) + (int16)(15000.0 * gyro_vol));
                  if(fuzz_out > thresh_high) fuzz_out = thresh_high;
                  else if(fuzz_out < thresh_low) fuzz_out = thresh_low;
                  McbspaRegs.DXR2.all = (int16)fuzz_out;
                  McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }
            else if (effectsel == PHASER)
            {
                  static float dmin = 440.0/(48000.0/2);
                  static float dmax = 16000.0/(48000.0/2);
                  static float fb = 0.7;
                  static float lfoPhase = 0;
                  static float depth = 1.0;
                  static float zm1 = 0.0;
                  static float lfoInc = 2.0 * 3.14 * 0.5 / 48000.0;
                  fb = 0.5 + (0.05) * (p2);
                  //lfoInc = 2.0 * 3.14 * (0.5 * (p1 + 1)) / 48000.0;
                  float d  = dmin + (dmax-dmin) * ((sin( lfoPhase ) + 1.0)/2.0);
                  lfoPhase += lfoInc;
                  if( lfoPhase >= 3.14 * 2.0 ) lfoPhase -= 3.14 * 2.0;
                  static float alps_delay[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
                  static float alps_zm1[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
                  //update filter coeffs
                  int j;
                  for( j = 0; j < 6; j++ )
                    alps_delay[j] = (1.0 - d)/(1.0 + d);
                  float in_samp = (float)(*ch1_ptr);
                  float y = in_samp + zm1 * fb;
                  for( j = 0; j < 6; j++ )
                  {
                      in_samp = y;
                      y = y * -1.0*alps_delay[j] + alps_zm1[j];
                      alps_zm1[j] = y * alps_delay[j] + in_samp;
                  }
                  zm1 = y;
                  lfoInc = 2.0 * 3.14 * (2.0 + 0.5 * (p1)) / 48000.0 * imu_dat.Xaccel/32000.0;
                  McbspaRegs.DXR2.all = (int16)(in_samp + depth*y);
                  McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;
            }


  			// *******************************************************************************************************
  			// 							Pitch Shifting variables
  			// *******************************************************************************************************


            else if (effectsel == PITCHUP || effectsel == ARPEGG || effectsel == PITCHDOWN)
			{
				pitch_shift_up.pitch_index = 0x7fff & (ext_index - pitch_shift_up.count_delay);

				// *** delay samples for pitch shiftng upward *** //
				// *** this is how we detect our zero crossings ***//
				sample_delay.samp[0] = sample_delay.samp[4];
				sample_delay.index[0]= sample_delay.index[4];
				if(effectsel == PITCHDOWN)
				{
					sample_delay.index[4] = ext_index;
				}
				else
				{
					sample_delay.index[4] = 0x7fff & (ext_index - pitch_shift_up.delay_return);
				}
				sample_delay.samp[4]  = ext_Buffer[sample_delay.index[4]];

				// *** Positive Zero Crossing Occured at return location *** //
				if(sample_delay.samp[4] > 0 && sample_delay.samp[0] < 0)
				{
					// for debugging using oscilliscope
					//GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;

					pitch_shift_up.zcross_detect = 1;  // this is what is used to detect a zero crossing
					// it will stay one until the corresponding negative zero crossing occurs


					pitch_shift_up.zcross_i = (FIFO_size - 1) & (pitch_shift_up.zcross_i + 1); // index to the next place in the
					// zross FIFO

					// *** trigger the peak detection algorithm *** //
					// *** find the exact index of the positive side of the zero crossing *** //
					zcross_return[pitch_shift_up.zcross_i].pos_zcross_i = sample_delay.index[4];  // store the index of the positive zero crossing in
					// its corresponding place in the zcross_neg buffer
					zcross_return[pitch_shift_up.zcross_i].peak = 0;

				}
				if(pitch_shift_up.zcross_detect == 1)
				{
					// time to find the peak
					// The signal sample that is ahead determines the peak
					if((sample_delay.samp[4] & 0x8000) == 0)
					{
						if(sample_delay.samp[4] > zcross_return[pitch_shift_up.zcross_i].peak)
						{
							zcross_return[pitch_shift_up.zcross_i].peak = sample_delay.samp[4];
						}
					}
				}

				// *** Negative Zero Crossing Occured at return location *** //
				if(sample_delay.samp[4] < 0 && sample_delay.samp[0] > 0 && pitch_shift_up.zcross_detect == 1)
				{

					//GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;

					// we are now done with the window for zer crossings time to pack up and find the distance
					// record the negative zero crossing INDEX
					zcross_return[pitch_shift_up.zcross_i].neg_zcross_i = sample_delay.index[4];

					// solve for the distances between the negative zcross and positivie zcross in raw samples
					if(zcross_return[pitch_shift_up.zcross_i].pos_zcross_i < zcross_return[pitch_shift_up.zcross_i].neg_zcross_i)
					{
						// in the case where the pos is behind the neg in the buffer without looparound
						// the equation is pretty simple
						zcross_return[pitch_shift_up.zcross_i].distance =  zcross_return[pitch_shift_up.zcross_i].neg_zcross_i - zcross_return[pitch_shift_up.zcross_i].pos_zcross_i;
					}
					else
					{
						zcross_return[pitch_shift_up.zcross_i].distance = 0x7fff - zcross_return[pitch_shift_up.zcross_i].pos_zcross_i + zcross_return[pitch_shift_up.zcross_i].neg_zcross_i + 1;
					}
					most_recent_peak = zcross_return[pitch_shift_up.zcross_i].peak;
					// reset the zcross_detect thing
					pitch_shift_up.zcross_detect = 0;
				}

				// pitch_shift_up.search_window
				if((pitch_shift_up.count_delay < pitch_shift_up.search_window && (effectsel == PITCHUP || effectsel == ARPEGG)) || ((pitch_shift_up.count_delay > (pitch_shift_up.delay_Ovfl>>1)) && effectsel == PITCHDOWN))
				{
					//GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;
					// *** The pitch pointer is in the search window, find a zero crossing! *** //
					// *** Positive Zero Crossing Occured at return location *** //
					if(ext_Buffer[pitch_shift_up.pitch_index] > 0 && pitch_shift_up.pitch_samp_prev < 0)
					{
						pitch_shift_up.zcross_detect_search = 1;
						zcross_pitch.pos_zcross_i = pitch_shift_up.pitch_index;
						zcross_pitch.peak = 0;

					}
					if(pitch_shift_up.zcross_detect_search == 1)
					{
						// time to find the peak
						// only searchs in between positive and negative zero crossing
						if((ext_Buffer[pitch_shift_up.pitch_index] & 0x8000) == 0)
						{
							if(ext_Buffer[pitch_shift_up.pitch_index] > zcross_pitch.peak)
							{
								zcross_pitch.peak = ext_Buffer[pitch_shift_up.pitch_index];
							}
						}
					}
					// *** Negative Zero Crossing Occured at return location *** //
					if(ext_Buffer[pitch_shift_up.pitch_index] < 0 && pitch_shift_up.pitch_samp_prev > 0 && pitch_shift_up.zcross_detect_search == 1)
					{
						//GpioDataRegs.GPADAT.bit.GPIO11 = 1;
						// we are now done with the window for zero crossings time to pack up and find the distance
						// record the negative zero crossing INDEX
						zcross_pitch.neg_zcross_i = pitch_shift_up.pitch_index;

						// solve for the distances between the negative zcross and positive zcross in raw samples
						if(zcross_pitch.pos_zcross_i < zcross_pitch.neg_zcross_i)
						{
							// in the case where the pos is behind the neg in the buffer without looparound
							// the equation is pretty simple
							zcross_pitch.distance =  zcross_pitch.neg_zcross_i - zcross_pitch.pos_zcross_i;
						}
						else
						{
							// size of the external buffer is 0x7fff and need for when
							// the samples are accross the buffer break
							zcross_pitch.distance = 0x7fff - zcross_pitch.pos_zcross_i + zcross_pitch.neg_zcross_i;
						}

						// reset the zcross_detect thing
						pitch_shift_up.zcross_detect_search = 0;

						// trigger the jump/return of the pitch pointer to a return spot!
						// if froze do not set the return flag
						if (pitch_shift_up.freeze == 0)
						{
							pitch_shift_up.return_flag = 1;
						}
					}
				}

				if(pitch_shift_up.return_flag == 1)
				{
					// find the manhatten distance between the pitch_zcross and the return_zcrosses
					// to find the most likely splicing point
					Uint16 manhattens[4];
					Uint16 min_index;
					Uint16 min_dist;
					//Uint16 manhattens;

					// calculate all manhatten distances
					// get the first distance for immediate comparison
					manhattens[0] = abs(zcross_pitch.distance - zcross_return[0].distance) + abs(zcross_pitch.peak - zcross_return[0].peak);
					min_index = 0;
					min_dist = manhattens[0];
					for(k = 1; k < FIFO_size; k++)
					{
						//abs(zcross_pitch.peak - zcross_return[k].peak);
						//absU16(zcross_pitch.distance - zcross_return[k].distance)
						manhattens[k] = abs(zcross_pitch.distance - zcross_return[k].distance) + abs(zcross_pitch.peak - zcross_return[k].peak);
						if(manhattens[k] < min_dist)
						{
							min_index = k;
							min_dist = manhattens[k];
						}
					}

					// after calculating the minimum distance, jump to the negative cross of the zcross
					// structure with the most similarity to the pitch zcross by making
					// pitch_shift_up.pitch_index equal to the neg_zcross_i value of the chosen return zcross

					pitch_shift_up.pitch_index = zcross_return[min_index].neg_zcross_i;

					// now reset the count_delay value
					// the count delay value is the distance from the sample pointer (ext_index) to the new pitch pointer

					if(pitch_shift_up.pitch_index < ext_index)
					{
						pitch_shift_up.count_delay =  ext_index - pitch_shift_up.pitch_index;
					}
					else
					{
						pitch_shift_up.count_delay = 0x7fff - pitch_shift_up.pitch_index + ext_index + 1;
					}


					if(pitch_shift_up.count_delay < pitch_shift_up.delay_return)
					{
						pitch_shift_up.return_flag = 0;
					}

					pitch_shift_up.return_flag = 0;
					zcrosstrig = 0x0001 & (zcrosstrig + 1);
				}


				//if(pitch_shift_up.count_delay == 1)
				//{
				//	pitch_shift_up.count_delay = 0;
				//}

				//if((pitch_shift_up.pitch == 0 || activateEffect == 0) && effectsel == ARPEGG)
				//{
				//	McbspaRegs.DXR2.all = *ch1_ptr;
				//}
				if((imu_dat.Xg_pos < 0.025 && effectsel == PITCHUP) || (imu_dat.Xg_pos < 0.025 && effectsel == PITCHDOWN))
				{
					pitch_shift_up.freeze = 1;
					McbspaRegs.DXR2.all = (ext_Buffer[pitch_shift_up.pitch_index]);
				}
				else
				{
					McbspaRegs.DXR2.all = (ext_Buffer[pitch_shift_up.pitch_index]);
					//McbspaRegs.DXR2.all = sample_delay.samp[4];
					//McbspaRegs.DXR1.all = 0x8000 & (zcrosstrig - 1);
					pitch_shift_up.freeze = 0;
				}
				McbspaRegs.DXR1.all = McbspaRegs.DXR2.all;

				// previous value of the sample pointer sample
				pitch_shift_up.pitch_samp_prev= ext_Buffer[pitch_shift_up.pitch_index];
			}

  			// *** finish audio buffer *** //
  			ext_prev = ext_Buffer[ext_index];
			sample.samp[0] = ext_prev;
			sample.index[0] = ext_index;
			ext_index = 0x7fff & (ext_index+1);

			// *** reset flag for sampler *** //
  			r_flag1=0;
  		}
}




// *** initialize all structures *** //
void init_structs(void)
{
	// *** initialize flanger structure *** //
	flang.speed[0] = 1000.0;    		// 100us default
	flang.speed[1] = 1600.0;    		// 100us default
	flang.speed[2] = 2200.0;    		// 100us default
	flang.speed[3] = 2800.0;    		// 100us default
	flang.speed[4] = 3400.0;    		// 100us default
	flang.speed[5] = 4000.0;    		// 100us default
	flang.speed[6] = 5000.0;    		// 100us default
	flang.speed[7] = 6000.0;    		// 100us default
	flang.speed[8] = 7000.0;    		// 100us default
	flang.speed[9] = 8000.0;    		// 100us default
	flang.delay_range[0] = 32;
	flang.delay_range[1] = 64;
	flang.delay_range[2] = 128;
	flang.delay_range[4] = 156;
	flang.delay_range[5] = 256;
	flang.delay_range[6] = 356;
	flang.delay_range[7] = 412;
	flang.delay_range[8] = 512;
	flang.delay_range[8] = 712;
	flang.delay 		 = 1024;

	// *** Initialize the Wah Structure *** //
	wah_svf.Q[0] = 0.05; // Q = damp*2
	wah_svf.Q[1] = 0.10;
	wah_svf.Q[2] = 0.12;
	wah_svf.Q[3] = 0.15; // Q = damp*2
	wah_svf.Q[4] = 0.17;
	wah_svf.Q[5] = 0.20;
	wah_svf.Q[6] = 0.22;
	wah_svf.Q[7] = 0.24;
	wah_svf.Q[8] = 0.30;
	wah_svf.Q[9] = 0.40;
	wah_svf.F_range[0] = 0.1;
	wah_svf.F_range[0] = 0.15;
	wah_svf.F_range[0] = 0.20;
	wah_svf.F_range[0] = 0.25;
	wah_svf.F_range[0] = 0.30;
	wah_svf.F_range[0] = 0.35;
	wah_svf.F_range[0] = 0.40;
	wah_svf.F_range[0] = 0.45;
	wah_svf.F_range[0] = 0.50;
	wah_svf.F = 0.085403;
	wah_svf.Fupper = 0.21;
	wah_svf.Flower = 0.08690;
	wah_svf.BPF[0]=0.0;
	wah_svf.BPF[1]=0.0;
	wah_svf.LPF[0]=0.0;
	wah_svf.LPF[1]=0.0;
	wah_svf.HPF=0.0;

// *** Pitch Shift Up Initializations *** //
pitch_shift_up.pitch            = 0;
pitch_shift_up.return_flag      = 0;
pitch_shift_up.zcross_i         = 0;
pitch_shift_up.zcross_detect_search    = 0;
pitch_shift_up.zcross_detect    = 0;
pitch_shift_up.Direction 		= 1;
pitch_shift_up.PerUpper 		= 300;
pitch_shift_up.Per 				= 50;
pitch_shift_up.count_delay  	= 0;
pitch_shift_up.pitch_index  	= 0;
pitch_shift_up.pitch_samp_prev  = 0;
pitch_shift_up.PerReset			= 1;
pitch_shift_up.delay_Ovfl       = 0xfff;  // 2047 is the max alllowable delay for pitch up, only happens when overflow occurs
pitch_shift_up.delay_return     = 0x7ff;  // 1023 is what we aim for when returning back in time to a zero crossing
pitch_shift_up.search_window    = 1024;   // start seahc for a zero crossing to jump when the pitch ptr delay is between 512 and 0

// *** Initialization of Arpeggio Arrays *** //

// Minor arpeggio
pitch_shift_up.Arpeggios[0][0]  = 2000.0;
pitch_shift_up.Arpeggios[0][1]  = 28.0;
pitch_shift_up.Arpeggios[0][2]  = 10.5;
pitch_shift_up.Arpeggios[0][3]  = 5;
// Major Arpeggio
pitch_shift_up.Arpeggios[1][0]  = 2000.0;
pitch_shift_up.Arpeggios[1][1]  = 20.0;
pitch_shift_up.Arpeggios[1][2]  = 10.5;
pitch_shift_up.Arpeggios[1][3]  = 5;
// Locrian Arpeggio
pitch_shift_up.Arpeggios[2][0]  = 2000.0;
pitch_shift_up.Arpeggios[2][1]  = 28.0;
pitch_shift_up.Arpeggios[2][2]  = 12.5;
pitch_shift_up.Arpeggios[2][3]  = 5;
// Minor Sweep Arpeggio
pitch_shift_up.Arpeggios[3][0]  = 2000.0;
pitch_shift_up.Arpeggios[3][1]  = 16.0;
pitch_shift_up.Arpeggios[3][2]  = 8.5;
pitch_shift_up.Arpeggios[3][3]  = 5;
// Major Sweep Arpeggio
pitch_shift_up.Arpeggios[4][0]  = 2000.0;
pitch_shift_up.Arpeggios[4][1]  = 16.0;
pitch_shift_up.Arpeggios[4][2]  = 6.5;
pitch_shift_up.Arpeggios[4][3]  = 5;
// Blues
pitch_shift_up.Arpeggios[5][0]  = 2000.0;
pitch_shift_up.Arpeggios[5][1]  = 28.0;
pitch_shift_up.Arpeggios[5][2]  = 16.0;
pitch_shift_up.Arpeggios[5][3]  = 12.5;
// Chromatic
pitch_shift_up.Arpeggios[6][0]  = 2000.0;
pitch_shift_up.Arpeggios[6][1]  = 90.0;
pitch_shift_up.Arpeggios[6][2]  = 42.0;
pitch_shift_up.Arpeggios[6][3]  = 28.0;
// Egypt
pitch_shift_up.Arpeggios[7][0]  = 2000.0;
pitch_shift_up.Arpeggios[7][1]  = 90.0;
pitch_shift_up.Arpeggios[7][2]  = 20.0;
pitch_shift_up.Arpeggios[7][3]  = 16.0;


// *** Pitch Shift Up Initializations *** //
pitch_shift_down.pitch            		= 0;
pitch_shift_down.return_flag      		= 0;
pitch_shift_down.zcross_i         		= 0;
pitch_shift_down.zcross_detect_search   = 0;
pitch_shift_down.zcross_detect    		= 0;
pitch_shift_down.Direction 				= 1;
pitch_shift_down.PerUpper 				= 300;
pitch_shift_down.Per 					= 50;
pitch_shift_down.count_delay  			= 0;
pitch_shift_down.pitch_index  			= 0;
pitch_shift_down.pitch_samp_prev  		= 0;
pitch_shift_down.PerReset				= 1;
pitch_shift_down.delay_Ovfl       		= 0xfff;  // 2047 is the max alllowable delay for pitch down, only happens when overflow occurs
pitch_shift_down.delay_return     		= 0x7ff;  // 1023 is what we aim for when returning back in time to a zero crossing
pitch_shift_down.search_window    		= 1024;   // start seahc for a zero crossing to jump when the pitch ptr delay is between 512 and 0
// half step
pitch_shift_down.PerLower[0] 			= 9.0;
pitch_shift_up.PerLower[0] 				= 90.0;
// whole step
pitch_shift_down.PerLower[1] 			= 9.0;
pitch_shift_up.PerLower[1] 				= 42.0;
// minor 3rd
pitch_shift_down.PerLower[2] 			= 9.0;
pitch_shift_up.PerLower[2] 				= 28.0;
//major third
pitch_shift_down.PerLower[3] 			= 9.0;
pitch_shift_up.PerLower[3] 				= 20.0;
// fourth
pitch_shift_down.PerLower[4] 			= 9.0;
pitch_shift_up.PerLower[4] 				= 16.0;
// tritone
pitch_shift_down.PerLower[5] 			= 9.0;
pitch_shift_up.PerLower[5] 				= 12.5;
// fifth
pitch_shift_down.PerLower[6] 			= 9.0;
pitch_shift_up.PerLower[6] 				= 10.5;
// sixth
pitch_shift_down.PerLower[7] 			= 9.0;
pitch_shift_up.PerLower[7] 				= 8.5;
// seventh
pitch_shift_down.PerLower[8] 			= 9.0;
pitch_shift_up.PerLower[8] 				= 6.0;
// octave
pitch_shift_down.PerLower[9] 			= 9.0;
pitch_shift_up.PerLower[9] 				= 5.0;

}



Uint16 absU16(Uint16 val)
{
	Uint16 tester;
	tester = val & 0x1000;
	if(tester != 0)
	{
		val = ~val + 0x0001;
	}
	return val;
}


//===========================================================================
// End of file.
//===========================================================================

