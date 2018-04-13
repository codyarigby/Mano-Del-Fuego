/*
 * LCDLib.c
 *
 *  Created on: Mar 2, 2017
 *      Author: Danny
 */

#include "LCD.h"
#include "AsciiLib.h"
#include "DSP28x_Project.h"

/************************************  Private Functions  *******************************************/

/*
 * Delay x ms
 */
static void Delay(unsigned long interval)
{
    while(interval > 0)
    {
        DELAY_US(1000L);
        interval--;
    }
}

/*******************************************************************************
 * Function Name  : LCD_initSPI
 * Description    : Configures LCD Control lines
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
static void LCD_initSPI()
{

    // Step 2. Initialize GPIO:
    // This example function is found in the DSP2833x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    // InitGpio();  // Skipped for this example
    // Setup only the GP I/O only for SPI-A functionality
    // This function is found in DSP2833x_Spi.c
    EALLOW;
 /* Enable internal pull-up for the selected pins */
 // Pull-ups can be enabled or disabled by the user.
 // This will enable the pullups for the specified pins.
 // Comment out other unwanted lines.

     //GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;   // Enable pull-up on GPIO16 (SPISIMOA)
     //GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;   // Enable pull-up on GPIO17 (SPISOMIA)
     //GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;   // Enable pull-up on GPIO18 (SPICLKA)
     //GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;   // Enable pull-up on GPIO19 (SPISTEA)


     GpioCtrlRegs.GPBPUD.bit.GPIO54 = 0;   // Enable pull-up on GPIO54 (SPISIMOA)
     GpioCtrlRegs.GPBPUD.bit.GPIO55 = 0;   // Enable pull-up on GPIO55 (SPISOMIA)
     GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0;   // Enable pull-up on GPIO56 (SPICLKA)
 //    GpioCtrlRegs.GPBPUD.bit.GPIO57 = 0;   // Enable pull-up on GPIO57 (SPISTEA)

 /* Set qualification for selected pins to asynch only */
 // This will select asynch (no qualification) for the selected pins.
 // Comment out other unwanted lines.

     //GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 (SPISIMOA)
     //GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 (SPISOMIA)
     //GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 (SPICLKA)
     //GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; // Asynch input GPIO19 (SPISTEA)

     GpioCtrlRegs.GPBQSEL2.bit.GPIO54 = 3; // Asynch input GPIO16 (SPISIMOA)
     GpioCtrlRegs.GPBQSEL2.bit.GPIO55 = 3; // Asynch input GPIO17 (SPISOMIA)
     GpioCtrlRegs.GPBQSEL2.bit.GPIO56 = 3; // Asynch input GPIO18 (SPICLKA)
 //    GpioCtrlRegs.GPBQSEL2.bit.GPIO57 = 3; // Asynch input GPIO19 (SPISTEA)


 /* Configure SPI-A pins using GPIO regs*/
 // This specifies which of the possible GPIO pins will be SPI functional pins.
 // Comment out other unwanted lines.

     //GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1; // Configure GPIO16 as SPISIMOA
     //GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1; // Configure GPIO17 as SPISOMIA
     //GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1; // Configure GPIO18 as SPICLKA
     //GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1; // Configure GPIO19 as SPISTEA

     GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 1; // Configure GPIO54 as SPISIMOA
     GpioCtrlRegs.GPBMUX2.bit.GPIO55 = 1; // Configure GPIO55 as SPISOMIA
     GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 1; // Configure GPIO56 as SPICLKA
 //    GpioCtrlRegs.GPBMUX2.bit.GPIO57 = 1; // Configure GPIO57 as SPISTEA

     //EDIS;

    GpioCtrlRegs.GPBMUX2.bit.GPIO57 = 0; // Configure GPIO19 as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO57 = 1; //Configure GPIO19 as output
    GpioCtrlRegs.GPBPUD.bit.GPIO57 = 1;   // Disable pull-up on GPIO19 (SPISTEA)
    SPI_CS_LCD_HIGH;


    // Initialize SPI FIFO registers
    SpiaRegs.SPIFFTX.all=0xE040;
    SpiaRegs.SPIFFRX.all=0x204f;
    SpiaRegs.SPIFFCT.all=0x0;

    SpiaRegs.SPICCR.all =0x000F;                 // Reset on, rising edge, 16-bit char bits
    SpiaRegs.SPICTL.all =0x0006;                 // Enable master mode, normal phase,
                                                 // enable talk, and SPI int disabled.
    SpiaRegs.SPIBRR =0x000A;
    SpiaRegs.SPICCR.all =0x00D7;                 // Relinquish SPI from Reset
    SpiaRegs.SPIPRI.bit.FREE = 1;                // Set so breakpoints don't disturb xmission

}

/*******************************************************************************
 * Function Name  : LCD_reset
 * Description    : Resets LCD
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : Uses P10.0 for reset
 *******************************************************************************/
static void LCD_reset()
{
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0; // Configure GPIO8 as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1; //Configure GPIO8 as output
    GpioDataRegs.GPADAT.bit.GPIO8 = 1; //Set GPIO8
    Delay(100);
    GpioDataRegs.GPADAT.bit.GPIO8 = 0; //Clear GPIO8
    Delay(100);
    GpioDataRegs.GPADAT.bit.GPIO8 = 1; //Set GPIO8

}

/************************************  Private Functions  *******************************************/


/************************************  Public Functions  *******************************************/

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : Must draw from left to right, top to bottom!
 *******************************************************************************/
void LCD_DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color)
{
    // Optimization complexity: O(64 + 2N) Bytes Written 

    /* Check special cases for out of bounds */


    if(xEnd > MAX_SCREEN_X)
    {
        xEnd = MAX_SCREEN_X;
    }
    else if(xStart < MIN_SCREEN_X)
    {
        xStart = MIN_SCREEN_X;
    }

    if(yEnd > MAX_SCREEN_Y)
    {
        yEnd = MAX_SCREEN_Y;
    }
    else if(yStart < MIN_SCREEN_Y)
    {
        yStart = MIN_SCREEN_Y;
    }



    /* Set window area for high-speed RAM write */
    LCD_WriteReg(HOR_ADDR_START_POS, yStart);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, yEnd-1);  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, xStart);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, xEnd-1); /* Vertical GRAM Start Address */

    /* Set cursor */ 
    LCD_SetCursor(xStart, yStart);

    /* Set index to GRAM */ 
    LCD_WriteIndex(GRAM);


    uint32_t size = ((uint32_t)(xEnd - xStart))*((uint32_t)(yEnd - yStart));                     // Calculate the size of the rectangle


    /* Send out data only to the entire area */ 
    SPI_CS_LCD_LOW;

    LCD_Write_Data_Start();                                         // Start write of continuous data

    uint32_t i = 0;
    while(i <= size)
    {
        i++;
        LCD_Write_Data_Only(Color);                                 // Output the data to the specified area
    }

    SPI_CS_LCD_HIGH;

}

/******************************************************************************
 * Function Name  : PutChar
 * Description    : Lcd screen displays a character
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - ASCI: Displayed character
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void PutChar( uint16_t Xpos, uint16_t Ypos, uint16_t ASCI, uint16_t charColor, int size)
{
    uint16_t i, j;
    unsigned char buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* get font data */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                //LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* Character color */
                LCD_DrawRectangle(Xpos + size*j, Xpos + size*j + size, Ypos + size*i, Ypos + size*i + size, charColor);
            }
        }
    }
}

/******************************************************************************
 * Function Name  : GUI_Text
 * Description    : Displays the string
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - str: Displayed string
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Text(uint16_t Xpos, uint16_t Ypos, uint16_t *str, uint16_t Color, int size)
{
    uint16_t TempChar;

    /* Set area back to span the entire LCD */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color, size);
        if( Xpos < MAX_SCREEN_X - 8*size)
        {
            Xpos += 8*size;
        }
        else if ( Ypos < MAX_SCREEN_X - 16*size)
        {
            Xpos = 0;
            Ypos += 16*size;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}


/*******************************************************************************
 * Function Name  : LCD_Clear
 * Description    : Fill the screen as the specified color
 * Input          : - Color: Screen Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Clear(uint16_t Color)
{
    /* Set area back to span the entire LCD */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    
    /* Set cursor to (0,0) */ 
    LCD_SetCursor(0, 0);

    /* Set write index to GRAM */     
    LCD_WriteIndex(GRAM);

    /* Start data transmission */

    SPI_CS_LCD_LOW;

    LCD_Write_Data_Start();                                 // Start write of continuous data

    long int i = 0;
    while(i < SCREEN_SIZE)
    {
        LCD_Write_Data_Only(Color);                         // Output data to whole LCD
        i++;
    }

    SPI_CS_LCD_HIGH;


    // You'll need to call LCD_Write_Data_Start() and then send out only data to fill entire screen with color 
}

/******************************************************************************
 * Function Name  : LCD_SetPoint
 * Description    : Drawn at a specified point coordinates
 * Input          : - Xpos: Row Coordinate
 *                  - Ypos: Line Coordinate
 * Output         : None
 * Return         : None
 * Attention      : 18N Bytes Written
 *******************************************************************************/
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
    /* Should check for out of bounds */ 
    if(Xpos > MAX_SCREEN_X)
    {
        Xpos = MIN_SCREEN_X;
    }
    if(Ypos > MAX_SCREEN_Y)
    {
        Ypos = MIN_SCREEN_Y;
    }

    /* Set cursor to Xpos and Ypos */ 
    LCD_SetCursor(Xpos, Ypos);

    /* Write color to GRAM reg */ 
    LCD_WriteReg(GRAM, color);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Only
 * Description    : Data writing to the LCD controller
 * Input          : - data: data to be written
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Only(uint16_t data)
{
    /* Send out MSB */ 
    SPISendRecvByte((data >>   8) << 8);                    /* Write D8..D15                */

    /* Send out LSB */ 
    SPISendRecvByte((data & 0xFF) << 8);                    /* Write D0..D7                 */
    DELAY_US(1);

}

/*******************************************************************************
 * Function Name  : LCD_WriteData
 * Description    : LCD write register data
 * Input          : - data: register data
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteData(uint16_t data)
{
    SPI_CS_LCD_LOW;

    SPISendRecvByte((SPI_START | SPI_WR | SPI_DATA) << 8);    /* Write : RS = 1, RW = 0       */
    SPISendRecvByte((data));                    /* Write D8..D15                */
    SPISendRecvByte((data << 8));                    /* Write D0..D7                 */

    SPI_CS_LCD_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 * Attention      : None
 *******************************************************************************/
inline uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);

    /* Return 16-bit Reg using LCD_ReadData() */
    return LCD_ReadData();

}

/*******************************************************************************
 * Function Name  : LCD_WriteIndex
 * Description    : LCD write register address
 * Input          : - index: register address
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteIndex(uint16_t index)
{
    SPI_CS_LCD_LOW;
    /* SPI write data */
    SPISendRecvByte((SPI_START | SPI_WR | SPI_INDEX) << 8);   /* Write : RS = 0, RW = 0  */
    SPISendRecvByte(0);
    SPISendRecvByte((index) << 8);

    SPI_CS_LCD_HIGH;
}

/*******************************************************************************
 * Function Name  : SPISendRecvByte
 * Description    : Send one byte then receive one byte of response
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : Received value
 * Attention      : None
 *******************************************************************************/
inline uint16_t SPISendRecvByte (uint16_t byte)
{
    /* Send byte of data */
    SpiaRegs.SPITXBUF=byte;

    /* Wait as long as s */
    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }


    /* Return received value*/
    return SpiaRegs.SPIRXBUF;
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Start
 * Description    : Start of data writing to the LCD controller
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Start(void)
{
    SPISendRecvByte((SPI_START | SPI_WR | SPI_DATA) << 8);    /* Write : RS = 1, RW = 0 */
}


/*******************************************************************************
 * Function Name  : LCD_ReadData
 * Description    : LCD read data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t LCD_ReadData()
{
    uint16_t value;
    SPI_CS_LCD_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    SPISendRecvByte(0);                               /* Dummy read 2           */
    SPISendRecvByte(0);                               /* Dummy read 3           */
    SPISendRecvByte(0);                               /* Dummy read 4           */
    SPISendRecvByte(0);                               /* Dummy read 5           */

    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_LCD_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_ReadPixelColor
 * Description    : Reads color from designated pixel
 * Input          : x and y coordinate
 * Output         : None
 * Return         : Pixel color
 * Attention      : None
 *******************************************************************************/
uint16_t LCD_ReadPixelColor(uint16_t x, uint16_t y)
{
    LCD_SetCursor(x, y);
    LCD_WriteIndex(GRAM);
    return LCD_ReadData();
}


/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_Reg: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{

    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);

    DELAY_US(5);

    /* Write 16-bit Reg Data */
    LCD_WriteData(LCD_RegValue);

}

/*******************************************************************************
 * Function Name  : LCD_SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos )
{
    /* Should just be two LCD_WriteReg to appropriate registers */ 

    /* Set horizonal GRAM coordinate (Ypos) */ 
    LCD_WriteReg(HORIZONTAL_GRAM_SET, Ypos);

    /* Set vertical GRAM coordinate (Xpos) */
    LCD_WriteReg(VERTICAL_GRAM_SET, Xpos);

}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt 
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Init(void)
{
    LCD_initSPI();

    LCD_reset();

    LCD_WriteReg(0xE5, 0x78F0); /* set SRAM internal timing */
    LCD_WriteReg(DRIVER_OUTPUT_CONTROL, 0x0100); /* set Driver Output Control */
    LCD_WriteReg(DRIVING_WAVE_CONTROL, 0x0700); /* set 1 line inversion */
    LCD_WriteReg(ENTRY_MODE, 0x1038); /* set GRAM write direction and BGR=1 */
    LCD_WriteReg(RESIZING_CONTROL, 0x0000); /* Resize register */
    LCD_WriteReg(DISPLAY_CONTROL_2, 0x0207); /* set the back porch and front porch */
    LCD_WriteReg(DISPLAY_CONTROL_3, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
    LCD_WriteReg(DISPLAY_CONTROL_4, 0x0000); /* FMARK function */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_1, 0x0000); /* RGB interface setting */
    LCD_WriteReg(FRAME_MARKER_POSITION, 0x0000); /* Frame marker Position */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_2, 0x0000); /* RGB interface polarity */

    /* Power On sequence */
    LCD_WriteReg(POWER_CONTROL_1, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(POWER_CONTROL_3, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(POWER_CONTROL_4, 0x0000); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0001);
    Delay(200);

    /* Dis-charge capacitor power voltage */
    LCD_WriteReg(POWER_CONTROL_1, 0x1090); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0227); /* Set DC1[2:0], DC0[2:0], VC[2:0] */
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(POWER_CONTROL_3, 0x001F);
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(POWER_CONTROL_4, 0x1500); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(POWER_CONTROL_7, 0x0027); /* 04 VCM[5:0] for VCOMH */
    LCD_WriteReg(FRAME_RATE_AND_COLOR_CONTROL, 0x000D); /* Set Frame Rate */
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, 0x0000); /* GRAM horizontal Address */
    LCD_WriteReg(GRAM_VERTICAL_ADDRESS_SET, 0x0000); /* GRAM Vertical Address */

    /* Adjust the Gamma Curve */
    LCD_WriteReg(GAMMA_CONTROL_1,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_2,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_3,    0x0307);
    LCD_WriteReg(GAMMA_CONTROL_4,    0x0200);
    LCD_WriteReg(GAMMA_CONTROL_5,    0x0008);
    LCD_WriteReg(GAMMA_CONTROL_6,    0x0004);
    LCD_WriteReg(GAMMA_CONTROL_7,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_8,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_9,    0x0002);
    LCD_WriteReg(GAMMA_CONTROL_10,   0x1D04);

    /* Set GRAM area */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X60, 0x2700); /* Gate Scan Line */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X61, 0x0001); /* NDL,VLE, REV */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X6A, 0x0000); /* set scrolling line */

    /* Partial Display Control */
    LCD_WriteReg(PART_IMAGE_1_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x81, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x82, 0x0000);
    LCD_WriteReg(PART_IMAGE_2_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x84, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x85, 0x0000);

    /* Panel Control */
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_1, 0x0010);
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_2, 0x0600);
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0133); /* 262K color and display ON */
    Delay(50); /* delay 50 ms */

    LCD_Clear(LCD_BLACK);
}
