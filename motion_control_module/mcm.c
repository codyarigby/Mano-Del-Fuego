/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05 
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 SPI - 3-wire Master MPU2950 interface
 *
 * This example shows how SPI master (msp432) talks to the
 * MPU2950 as a SPI slave using 3-wire mode.
 *
 *
 * Note that in this example, EUSCIB0 is used for the SPI port. If the user
 * wants to use EUSCIA for SPI operation, they are able to with the same APIs
 * with the EUSCI_AX parameters.
 *
 * Use with SPI Slave Data Echo code example.
 *
 *                MSP432P401
 *              -----------------
 *             |                 |
 *             |                 |
 *             |                 |
 *             |             P1.6|-> Data Out (UCB0SIMO)
 *             |                 |
 *             |             P1.7|<- Data In (UCB0SOMI)
 *             |                 |
 *             |             P1.5|-> Serial Clock Out (UCB0CLK)
 *             |				 |
 *             |		    P4.6 |-> CS
 *
 * Author: Timothy Logan
*******************************************************************************/
/* DriverLib Includes */
#include "driverlib.h"

#include "ClockSys.h"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>

// this is the list of memory maped register addresses of the mpu2950
static uint8_t ADDR_PWR_MGMT1 		=  	0x6b; // used to enable clock source
static uint8_t ADDR_PWR_MGMT2 		=  	0x6c; // used to enable accel and gyro
static uint8_t ADDR_USER_CTRL 		=  	0x6a; // used to disable I2C
static uint8_t ADDR_ACCEL_CONFIG_2	=	0x1D;  // write 0x02 to disable lpf, used for lpf configurations
static uint8_t ADDR_ACCEL_CONFIG	=	0x1C; // [4:3], 00 2g 01 4g 10 8g 11 16g
static uint8_t ADDR_CONFIG			=	0x1A; // fchoice_b[1:0] = b00, and fchoice[1:0] = b11 (this reg is used to set lpf for gyro) DLPF_CFG[2:0] = 1
static uint8_t ADDR_GYRO_CONFIG		=	0x1B;

static uint8_t ADDR_ACCEL_DAT		=	0x3B; // start addr of accel data xh, xl, yh, yl, zh, zl
static uint8_t ADDR_GYRO_DAT		=	0x43; // start addr of gyro  data xh, xl, yh, yl, zh, zl

// this disables the mpu2950 as an I2C slave
// write this to ADDR_USER_CTRL
static uint8_t dis_i2c		  =  0x10;


// Accelerometer lowpass filter bandwidths
// for ADDR_ACCEL_CONFIG_2
// assume ACCEL_FCHOICE = 0; set to 1 to bypass filter
// writes to bits [2:0]
// bw's are measured in hz		        	DELAY:
static uint8_t alpf_bw_460	  	= 0x00; //	1.94ms
static uint8_t alpf_bw_184	  	= 0x01; //	5.80ms
static uint8_t alpf_bw_92	  	= 0x02; //	7.80ms
static uint8_t alpf_bw_41		= 0x03;	//	11.80ms
static uint8_t alpf_bw_20		= 0x04;	//	19.80ms
static uint8_t alpf_bw_10		= 0x05;	//	35.70ms
static uint8_t alpf_bw_5		= 0x06; //	66.96ms
// 7 is 460hz
// writing these directly to the register will set accel_fchoice_b to 0 which enables low pass filters


// Accelorometer Full Scale Selection
// for ADDR_ACCEL_CONFIG
// writes to bits [4:3] ACCEL_FS_SEL
static uint8_t afs_2g			= 0x00; // +-2g
static uint8_t afs_4g			= 0x08; // +-4g
static uint8_t afs_8g			= 0x10; // +-8g
static uint8_t afs_16g			= 0x18; // +-16g


// Gyroscope lowpass filter bandwidths
// for ADDR_CONFIG
// dlfp works when fchoice_b [1:0] = 2b00 in the gyro config register
// writes to bits [2:0] DLPF_CFG
// bw's are measured in hz		        	DELAY:
static uint8_t glpf_bw_250	  	= 0x00; //	0.97ms
static uint8_t glpf_bw_184	  	= 0x01; //	2.90ms
static uint8_t glpf_bw_92	  	= 0x02; //	3.90ms
static uint8_t glpf_bw_41		= 0x03;	//	5.90ms
static uint8_t glpf_bw_20		= 0x04;	//	9.90ms
static uint8_t glpf_bw_10		= 0x05;	//	17.85ms
static uint8_t glpf_bw_5		= 0x06; //	33.48ms
// 7 is 3600hz delay of 0.17ms
// when fchoice_b is 2b11 then bw is 8800 with delay 0.064ms
// when fchoice_b is 2b10 then bw is 3600 with delay 0.11ms


// Gyro Full Scale Selection
// for ADDR_GYRO_CONFIG
// writes to bits [4:3] GYRO_FS_SEL
static uint8_t gfs_250dps			= 0x00; // +-250dps
static uint8_t gfs_500dps			= 0x08; // +-500dps
static uint8_t gfs_1kdps			= 0x10; // +-1000dps
static uint8_t gfs_2kdps			= 0x18; // +-2000dps
// write these directly to set fchoice_b to 2b00 to enable gyro lowpass filter


int write_reg(uint8_t addr, uint8_t data);
int read_reg(uint8_t addr);
void init_imu(uint8_t * addr, uint8_t * data, uint8_t len);
void read_sensors(uint8_t acceladdr, uint8_t gyroaddr, uint16_t * accelDat, uint16_t * gyroDat);

//Statics
static volatile uint8_t RXData = 0;
static uint8_t TXData = 0x00;

// SPI Master Configuration Parameter
// the code ti provided ran the DCO at 3MHz but this application
// runs at 6MHz so the spiclk is twice the frequency that
// is specified in the structure below
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,             // SMCLK Clock Source
        3000000,                                   // SMCLK = DCO = 3MHZ
        1000000,                                    // SPICLK = 500khz
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, // High polarity
		EUSCI_B_SPI_3PIN            // 3Wire SPI Mode
};

//UART config for RN42 module
//Baud rate is 115200.
const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    26,                                     // BRDIV = 6
    0,                                       // UCxBRF = 8
    0,                                       // UCxBRS = 0
    EUSCI_A_UART_NO_PARITY,                  // No Parity
    EUSCI_A_UART_LSB_FIRST,                  // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
    EUSCI_A_UART_MODE,                       // UART mode
    EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Oversampling
};

/* Statics */
static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;
bool activate = false;
bool next = false;

int main(void)
{

// these are the initialization arrays
// they take care of writing to all the registers to get started with reading data!
// pass the address of the first index to the addr and data parameters in the init_imu() function
// 0x01 to PWR_MGMT1 sets the clk frequency for the imu
// 0x00 to ADDR_PWR_MGMT2 enables the accelerometer and gyroscope
// 0x10 to ADDR_USER_CTRL and disables the mpu9250 as an i2c slave
// the rest are initializations for accel/gyro scales and lpf's, probably shouldn't be included in this array but its a good start
uint8_t initAddr_array[7] = {ADDR_PWR_MGMT1, ADDR_PWR_MGMT2, ADDR_USER_CTRL,ADDR_ACCEL_CONFIG_2, ADDR_ACCEL_CONFIG, ADDR_CONFIG,  ADDR_GYRO_CONFIG};
uint8_t initData_array[7] = {0x01,			 0x00,			 0x10, 			alpf_bw_184,		 0x01,			    0x01,		  0x00,		  };



// buffer that stores 16 bit 3 axis accel and gyro data
// 						   |Z|X|Y|
	int16_t accelDat[3] = {0,0,0};
	uint16_t gyroDat[3]	 = {0,0,0};

// buffer that takes samples from the imu
// this is just to view the sensor data in real time
	int16_t sensorbuff[512];
	int buffer_size = 512;



    //Halting WDT
    WDT_A_holdTimer();

    /* Initialize Clock */
    //ClockSys_SetMaxFreq();

    //Setting DCO to 6MHz
    //Setting Flash wait state
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    //Setting DCO to 6MHz (run msp432 at 6MHz)
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_6);

    //Selecting P1.5 P1.6 and P1.7 in SPI mode
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
     GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);


    // we will just use a gpio for the chip select, keep in mind that it can be pretty slow
    GPIO_setAsOutputPin (GPIO_PORT_P4, GPIO_PIN6);		// use P4.6 for chip select
    GPIO_setOutputHighOnPin (GPIO_PORT_P4, GPIO_PIN6);	// initialize chip select as high

    //Configuring SPI in 3wire master mode
    SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);

    //Enable SPI module
    SPI_enableModule(EUSCI_B0_BASE);
    // Interrupt_enableMaster();


    /* Configure pins P3.2(RX) and P3.3(TX) in UART mode.
     * Doesn't matter if they are set to inputs or outputs
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_BASE);

    // Enabling interrupts
    // SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    // Interrupt_enableInterrupt(INT_EUSCIB0);
    // Interrupt_enableSleepOnIsrExit();

    // whoami is a register inside the mpu9250
    // the register always contains the value 0x71 so it is a good way to
    // check your connection by reading from it after the initialization
    uint16_t whoami = 0;

    // initialize the mpu registers
    // after this function you will be able to read from the sensor data registers
    // give the initialization function the handles for the register addresses and the data to go into those registers
    init_imu(&initAddr_array[0], &initData_array[0], 7);

    // read register 0x75 which contains the slave address
    // if the msp432 reads 0x71 from this register you have a good connection
    whoami = read_reg(0x75);  //whoami
    if(whoami != 0x71){
        whoami = -1;
    }

    // this is the index for the sensor buffer
    int index = 0;

    /* Initializing Variables */
    curADCResult = 0;

    //![Single Sample Mode Configure]
   /* Initializing ADC (MCLK/1/4) */
   MAP_ADC14_enableModule();
   MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_64, ADC_DIVIDER_8,
           0);

   /* Configuring GPIOs (5.5 A0) */
   MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
   GPIO_TERTIARY_MODULE_FUNCTION);
   GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);

   /* Configuring ADC Memory */
   MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
   MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
   ADC_INPUT_A0, false);

   /* Configuring Sample Timer */
   MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

   /* Enabling/Toggling Conversion */
   MAP_ADC14_enableConversion();
   MAP_ADC14_toggleConversionTrigger();
   //![Single Sample Mode Configure]



    //Wait a bit after start up to talk to RN42
    __delay_cycles(3000000);

    //Send '$$$' to make RN42 advertise. RN42 red LED should start flashing more quickly.
    UART_transmitData(EUSCI_A2_BASE, '$');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, '$');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, '$');
    __delay_cycles(100000);

    //Pause a bit to let RN42 settle.
    __delay_cycles(3000000);

    UART_transmitData(EUSCI_A2_BASE, 'C');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, 'F');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, 'R');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, '\n');
    __delay_cycles(3000000);

    /*UART_transmitData(EUSCI_A2_BASE, 'S');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, 'M');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, ',');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, '3');
    __delay_cycles(100000);
    UART_transmitData(EUSCI_A2_BASE, '\n');
    __delay_cycles(3000000);*/

    /*UART_transmitData(EUSCI_A2_BASE, 'S');
    UART_transmitData(EUSCI_A2_BASE, 'T');
    UART_transmitData(EUSCI_A2_BASE, ',');
    UART_transmitData(EUSCI_A2_BASE, '0');
    UART_transmitData(EUSCI_A2_BASE, '\n');
    __delay_cycles(100000);

    UART_transmitData(EUSCI_A2_BASE, 'S');
    UART_transmitData(EUSCI_A2_BASE, 'Y');
    UART_transmitData(EUSCI_A2_BASE, ',');
    UART_transmitData(EUSCI_A2_BASE, '0');
    UART_transmitData(EUSCI_A2_BASE, '0');
    UART_transmitData(EUSCI_A2_BASE, '1');
    UART_transmitData(EUSCI_A2_BASE, '0');
    UART_transmitData(EUSCI_A2_BASE, '\n');
    __delay_cycles(100000);*/

/*
 *  This loop reads all of the accel and gyro sensors every
 *  400 cycles and stores one of those values in the sensor buffer
 *  X,Y, and Z axis for the accel and gyro sensors are all read
 */
    int16_t average = 0;

    /* Enabling interrupts */
    MAP_ADC14_enableInterrupt(ADC_INT0);
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();

while(1){

	read_sensors(ADDR_ACCEL_DAT, ADDR_GYRO_DAT, &accelDat[0], &gyroDat[0]);
	sensorbuff[index] = gyroDat[0];
	index++;

	if(index > (buffer_size-1)){
	index = 0;
	}

	//accel[0] is punching
	//accel[1] is strumming
	//accel[2] is knocking

	//gyro[0] is doorknob


	if(activate)
	{
	    accelDat[1] |= 0x0001;
	}
	else
	{
	    accelDat[1] &= 0xFFFE;
	}

    if(next)
    {
        gyroDat[0] |= 0x0001;
        next = false;
    }
    else
    {
        gyroDat[0] &= 0xFFFE;
    }

    __delay_cycles(150);
    UART_transmitData(EUSCI_A2_BASE, (int8_t)(accelDat[1] >> 8));
	__delay_cycles(150);
    UART_transmitData(EUSCI_A2_BASE, (int8_t)accelDat[1]);
    __delay_cycles(150);
    UART_transmitData(EUSCI_A2_BASE, (uint8_t)(gyroDat[0] >> 8));
    __delay_cycles(150);
    UART_transmitData(EUSCI_A2_BASE, (uint8_t)gyroDat[0]);
    __delay_cycles(150);
    UART_transmitData(EUSCI_A2_BASE, '\n');
    MAP_ADC14_toggleConversionTrigger();



}

	// go to low power mode
	// this program won't go here
    PCM_gotoLPM0();
    __no_operation();
}


// this isr is not used but we could consider having the msp432 in low power mode
// with a timer and then in the isr we handle the spi transactions with the mpu9250


// commenting this ISR out throws an unresolved error symbol in the startup c file
//******************************************************************************
//This is the EUSCI_B0 interrupt vector service routine.
//******************************************************************************
void EUSCIB0_IRQHandler(void)
{
    uint32_t status = SPI_getEnabledInterruptStatus(EUSCI_B0_BASE);
    uint32_t jj;
    SPI_clearInterruptFlag(EUSCI_B0_BASE, status);
    if(status & EUSCI_B_SPI_RECEIVE_INTERRUPT)
    {
        //USCI_B0 TX buffer ready?
        while (!(SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
        RXData = SPI_receiveData(EUSCI_B0_BASE);
        // Send the next data packet
        SPI_transmitData(EUSCI_B0_BASE, ++TXData);
        // Delay between transmissions for slave to process information
        for(jj=50;jj<50;jj++);
    }
}


// routine that writes to the register in the mpu2950
// two parameters, addr of the register to write to
// data to write to the registers
// MSB of the addr is zero
int write_reg(uint8_t addr, uint8_t data)
{
	// write to the address
	while (!(SPI_getInterruptStatus(EUSCI_B0_BASE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));

	// set pin 4.6 to low as the chip select
	GPIO_setOutputLowOnPin (GPIO_PORT_P4, GPIO_PIN6);

	// transmit the address to the mpu9250 as the first 8 bits
	SPI_transmitData(EUSCI_B0_BASE, addr);

	// transmite the data to write to the register that was chosen
	SPI_transmitData(EUSCI_B0_BASE, data);

	// delay a little bit before setting the CS or else it will be set before the transaction is over
	__delay_cycles(2000);
	GPIO_setOutputHighOnPin (GPIO_PORT_P4, GPIO_PIN6);
}


// routine that reads from the register in the mpu2950
// only one parameter, which is the address, returns the data
// MSB of the addr is one so the address must be or'd with 0x80
int read_reg(uint8_t addr)
{
	// data to return
	uint8_t data = 0;

	// read from the address
	while (!(SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
	GPIO_setOutputLowOnPin (GPIO_PORT_P4, GPIO_PIN6);

	// send address with the MSB set to 1
	SPI_transmitData(EUSCI_B0_BASE, (addr | 0x80));

	// send dummy data
	SPI_transmitData(EUSCI_B0_BASE, 0x00);

	// set CS after delay
	__delay_cycles(21);
	GPIO_setOutputHighOnPin (GPIO_PORT_P4, GPIO_PIN6);
	__delay_cycles(12);

	// read the data from the spi rx buffer
	data = SPI_receiveData(EUSCI_B0_BASE);
	return data;
}

// initialize the imu
// this function grabs two initialization array starting addresses
// then runs through them to write to all the control registers
// before reading data, refer to initAddr_array and initData_array
// to know what registers are initialized with what
// the length of both arrays is also specified
void init_imu(uint8_t * addr, uint8_t * data, uint8_t len)
{

	int i;

	// data check is used for debugging
	// after writing to a register this function
	// then reads that register to confrim that
	// it contains the data that was just written to it
	int16_t dataCheck = 0; // change this back to uint8_t is there are problems

	// loop that does all the register writting and checking
	for(i = 0; i < len; i++)
	{
		// write to register
		write_reg(*addr,*data);
		// check the data in the register with the data you just tried to write
		dataCheck = read_reg(*addr);
		if(dataCheck != *data)
		{
			dataCheck = -1;		// this is to make sure that the data is actually written to the registers
		}
	addr++;
	data++;
	}
}


// This routine filles up two arrays of 3axis 16bit accel and gyro data
// the routine is passed the address of the first gyro and accel data registers in the imu
// reading the accel data starts at Xdata high then Xdata low, repeated for Y and Z axis, reads from 6 registers
// reading the gyro  data starts at Xdata high then Xdata low, repeated for Y and Z axis, reads from 6 registers
void read_sensors(uint8_t acceladdr, uint8_t gyroaddr, uint16_t * accelDat, uint16_t * gyroDat)
{
	int i = 0;
	uint16_t tmp = 0;

	//fill accel data

	//Accel X axis
	tmp = read_reg(acceladdr++);
	*accelDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(acceladdr++);
	*accelDat |= tmp & 0x00FF;
	accelDat++;

	//Accel Y axis
	tmp = read_reg(acceladdr++);
	*accelDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(acceladdr++);
	*accelDat |= tmp & 0x00FF;
	accelDat++;

	//Accel Z axis
	tmp = read_reg(acceladdr++);
	*accelDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(acceladdr++);
	*accelDat |= tmp & 0x00FF;
	accelDat++;

	//Gyro X axis
	tmp = read_reg(gyroaddr++);
	*gyroDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(gyroaddr++);
	*gyroDat |= tmp & 0x00FF;
	gyroDat++;

	//Gyro Y axis
	tmp = read_reg(gyroaddr++);
	*gyroDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(gyroaddr++);
	*gyroDat |= tmp & 0x00FF;
	gyroDat++;

	//Gyro Z axis
	tmp = read_reg(gyroaddr++);
	*gyroDat = (tmp & 0x00FF) << 8;
	tmp = read_reg(gyroaddr++);
	*gyroDat |= tmp & 0x00FF;
	gyroDat++;

}

//![Single Sample Result]
/* ADC Interrupt Handler. This handler is called whenever there is a conversion
 * that is finished for ADC_MEM0.
 */

uint32_t counter1 = 0;
uint32_t counter2 = 0;
const uint32_t count_val1 = 50;
const uint32_t count_val2 = 10;
void ADC14_IRQHandler(void)
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    if (ADC_INT0 & status)
    {
        curADCResult = MAP_ADC14_getResult(ADC_MEM0);
        normalizedADCRes = (curADCResult * 3.3) / 16384;

        if(curADCResult > 12000){
            GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0);
            if(activate == false)
            {
                counter1 = 0;
            }
            else
            {
                if(counter1 < count_val1) counter1++;
            }
            activate = true;
        }
        else{
            GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
            if(activate == true)
            {
                if(counter1 < count_val1 && next == false)
                {
                    next = true;
                    counter2 = 0;
                }
            }
            if(counter2 < count_val2 && next == true) counter2++;
            else
            {
                next = false;
            }
            activate = false;
        }

        //MAP_ADC14_toggleConversionTrigger();
    }
}
//![Single Sample Result]



