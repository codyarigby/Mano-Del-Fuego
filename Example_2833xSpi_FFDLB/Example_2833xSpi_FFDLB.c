//###########################################################################
// Description:
//!  \addtogroup f2833x_example_list
//!  <h1>SPI Digital Loop Back (spi_loopback)</h1>
//!
//!  This program uses the internal loop back test mode of the peripheral.
//!  Other then boot mode pin configuration, no other hardware configuration
//!  is required. Interrupts are not used.
//!
//!  A stream of data is sent and then compared to the received stream.
//!  The sent data looks like this: \n
//!  0000 0001 0002 0003 0004 0005 0006 0007 .... FFFE FFFF \n
//!  This pattern is repeated forever.
//!
//!  \b Watch \b Variables \n
//!  - sdata - Sent data
//!  - rdata - Received data
//
////###########################################################################
// $TI Release: F2833x/F2823x Header Files and Peripheral Examples V141 $
// $Release Date: November  6, 2015 $
// $Copyright: Copyright (C) 2007-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "LCD.h"
#include "math.h"
#include "notes.h"

// Prototype statements for functions found within this file.
// __interrupt void ISRTimer2(void);
void delay_loop(void);
void spi_xmit(Uint16 a);
void spi_fifo_init(void);
void spi_init(void);
void error(void);
int16_t cents(int16_t);

//LCD Functions
void UI_LCD_Logo_mode();
void UI_LCD_RF_Connect_mode();
void UI_LCD_RF_Try_Again_mode();
void UI_LCD_Main_mode();
void UI_LCD_Deactivate_FX_mode(int);
void UI_LCD_Activate_FX_mode(int);
void UI_LCD_Effect_Setting_mode();
void UI_LCD_Flex_Mode_Change(int);
void UI_LCD_Effect_Shift_Setting(char*, char*, char*, char*, char*);
void UI_LCD_Effect_Change_Param(int, char*);
void UI_LCD_Tuner_mode();
void UI_LCD_Tune(int);

//UI State Functions
void State_Initialize();
void Attempt_RF_Connect();
bool BT_Connect();
void Main_Mode();
void Tuner();
void Initialize_Board();
void State_Change();
void Clear_Buttons();

#define HOLD_MODE 0
#define MOD_MODE 1
#define PASS_MODE 2

#define FX1 0
#define FX2 1
#define FX3 2
#define FX4 3

#define P1 1
#define P2 2

#define NumFX 5

#define Bypass 0
#define Wah 1
#define VolSwell 2
#define Flanger 3
#define PitchShift 4

#define EXIT_RF 0
#define EXIT_Setting 1
#define EXIT_Tuner 2

typedef struct
{
    char* name;
    char* param1;
    int value1;
    char* param2;
    int value2;
    int FX_index;
} Effect;

typedef enum
{
    Logo_Mode = 0,
    RF_Connect_Mode = 1,
    Main_Menu_Mode = 2,
    Effect_Setting_Mode = 3,
    Tuner_Mode = 4
} Mode;

typedef struct
{
    Mode boardMode;
    Effect FX[4];
    bool RF_Connected;
    int currentEffect;
    int Flex_State;
} Board_State;

Board_State Global_Board_State, Prev_Board_State;
Effect Global_FXLib[NumFX];
char FX_names[NumFX][11] = {"Bypass", "Wah", "Volume", "Flange", "PShift"};
char param1_names[NumFX][4] = {"N/A", "TBD", "TBD", "TBD", "TBD"};
int value1_init[NumFX] = {0, 0, 0, 0, 0};
char param2_names[NumFX][4] = {"N/A", "TBD", "TBD", "TBD", "TBD"};
int value2_init[NumFX] = {0, 0, 0, 0, 0};

bool Button1 = false;
bool Button2 = false;
bool Button3 = false;
bool Button4 = false;
bool ButtonUp = false;
bool ButtonDown = false;
bool Switch1 = false;
bool Switch2 = false;
bool Switch3 = false;
bool Switch4 = false;

void main(void)
{
   Uint16 sdata;  // send data
   Uint16 rdata;  // received data

// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP2833x_SysCtrl.c file.
   InitSysCtrl();

// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
   DINT;

// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
// This function is found in DSP2833x_PieVect.c.
   InitPieVectTable();

   /*LCD_Init();

   char Effect_name[] = "Wah";
   char Param1_name[] = "Q";
   char Value1_name[] = "3.0";
   char Param2_name[] = "Swp Spd";
   char Value2_name[] = "4";

   char Value1_name1[] = "Deez";
   char Value2_name1[] = "Nutz";

   UI_LCD_Logo_mode();
   UI_LCD_RF_Connect_mode();
   UI_LCD_RF_Try_Again_mode();
   UI_LCD_Main_mode();
   UI_LCD_Activate_FX_mode(FX1);
   UI_LCD_Activate_FX_mode(FX2);
   UI_LCD_Activate_FX_mode(FX3);
   UI_LCD_Activate_FX_mode(FX4);
   UI_LCD_Effect_Setting_mode();
   UI_LCD_Flex_Mode_Change(HOLD_MODE);
   UI_LCD_Flex_Mode_Change(MOD_MODE);
   UI_LCD_Flex_Mode_Change(PASS_MODE);
   UI_LCD_Effect_Shift_Setting(Effect_name, Param1_name, Value1_name, Param2_name, Value2_name);
   UI_LCD_Effect_Change_Param(P1, Value1_name1);
   UI_LCD_Effect_Change_Param(P2, Value2_name1);
   UI_LCD_Tuner_mode();
   UI_LCD_Tune(440);*/

   Initialize_Board();


   while(1)
   {
       if(Global_Board_State.boardMode == Tuner_Mode) UI_LCD_Tune(440);
       State_Change();
   }
}

int16_t cents(int16_t freq)
{
    return (int16_t) 1200*log2(freq/16.35);
}



void UI_LCD_Logo_mode()
{
    char name1[] = "Mano";
    char name2[] = "Del";
    char name3[] = "Fuego";
    char name4[] = "A Dope Ass Senior Design Project";
    char name5[] = "By Cody Rigby and Nick Landy";
    //LCD_Text(0, 0, (uint16_t *)name, LCD_GREEN);
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    LCD_Text(80, 60, (uint16_t *) name1, LCD_RED, 1);
    LCD_Text(120, 100, (uint16_t *) name2, LCD_ORANGE, 2);
    LCD_Text(160, 140, (uint16_t *) name3, LCD_YELLOW, 3);
    LCD_Text(40, 200, (uint16_t *) name4, LCD_BLUE, 1);
    LCD_Text(60, 220, (uint16_t *) name5, LCD_BLUE, 1);
}

void UI_LCD_RF_Connect_mode()
{
    char RFConn[] = "RF Connecting...";
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    LCD_Text(100, 100, (uint16_t *) RFConn, LCD_WHITE, 1);
}

void UI_LCD_RF_Try_Again_mode()
{
    char RFNoConn[] = "RF Cannot Connect";
    char Try[] = "Try Again?";
    char Yes[] = "Yes";
    char No[] = "No";
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    LCD_Text(100, 100, (uint16_t *) RFNoConn, LCD_WHITE, 1);
    LCD_Text(120, 120, (uint16_t *) Try, LCD_WHITE, 1);
    LCD_DrawRectangle(0, 80-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(80, 160-4, 200, 240, LCD_WHITE);
    LCD_Text(30, 210, (uint16_t *) Yes, LCD_BLACK, 1);
    LCD_Text(110, 210, (uint16_t *) No, LCD_BLACK, 1);
}

void UI_LCD_Main_mode()
{
    char One[] = "1";
    char Two[] = "2";
    char Three[] = "3";
    char Four[] = "4";
    char RF[] = "RF";
    char Tuner[] = "Tuner";
    char Effect[] = "Effect";
    char name1[] = "Mano";
    char name2[] = "Del";
    char name3[] = "Fuego";
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    LCD_Text(60, 20, (uint16_t *) name1, LCD_RED, 2);
    LCD_Text(140, 60, (uint16_t *) name2, LCD_ORANGE, 2);
    LCD_Text(220, 100, (uint16_t *) name3, LCD_YELLOW, 2);
    LCD_DrawRectangle(0, 80-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(80, 160-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(160, 240-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(240, 320-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(0, 50, 0, 60-4, LCD_BLUE);
    LCD_DrawRectangle(0, 50, 60, 120-4, LCD_BLUE);
    LCD_Text(35, 210, (uint16_t *) One, LCD_BLACK, 1);
    LCD_Text(115, 210, (uint16_t *) Two, LCD_BLACK, 1);
    LCD_Text(195, 210, (uint16_t *) Three, LCD_BLACK, 1);
    LCD_Text(275, 210, (uint16_t *) Four, LCD_BLACK, 1);
    LCD_Text(15, 75, (uint16_t *) RF, LCD_BLACK, 1);
    LCD_Text(5, 15, (uint16_t *) Tuner, LCD_BLACK, 1);
    LCD_DrawRectangle(0, 80-4, 180, 195, LCD_WHITE);
    LCD_DrawRectangle(80, 160-4, 180, 195, LCD_WHITE);
    LCD_DrawRectangle(160, 240-4, 180, 195, LCD_WHITE);
    LCD_DrawRectangle(240, 320-4, 180, 195, LCD_WHITE);
    LCD_Text(0, 180, (uint16_t *) Global_Board_State.FX[0].name, LCD_BLACK, 1);
    LCD_Text(80, 180, (uint16_t *) Global_Board_State.FX[1].name, LCD_BLACK, 1);
    LCD_Text(160, 180, (uint16_t *) Global_Board_State.FX[2].name, LCD_BLACK, 1);
    LCD_Text(240, 180, (uint16_t *) Global_Board_State.FX[3].name, LCD_BLACK, 1);

    int i;
    for(i = 0; i < 15; i++)
    {
        //LCD_DrawRectangle(30 + abs(i-7), 30 + 15 - abs(i-7), 160 + i, 160 + i + 1, LCD_WHITE);
        LCD_DrawRectangle(30 + 10 - i, 30 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
    }
    for(i = 0; i < 15; i++)
    {
        //LCD_DrawRectangle(110 + abs((i-7)/2*2), 110 + 15 - abs((i-7)/2*2), 160 + i, 160 + i + 1, LCD_WHITE);
        LCD_DrawRectangle(110 + 10 - i, 110 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
    }
    for(i = 0; i < 15; i++)
    {
        LCD_DrawRectangle(190 + 10 - i, 190 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
    }
    for(i = 0; i < 15; i++)
    {
        //LCD_DrawRectangle(270 - 15 + i, 270 + 15 - i, 160 + i, 160 + i + 1, LCD_WHITE);
        LCD_DrawRectangle(270 + 10 - i, 270 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
    }
}
void UI_LCD_Deactivate_FX_mode(int FX)
{
    char One[] = "1";
    char Two[] = "2";
    char Three[] = "3";
    char Four[] = "4";
    char Effect[] = "Effect";
    int i;


    if(FX == FX1)
    {
        LCD_DrawRectangle(0, 80-4, 200, 240, LCD_WHITE);
        LCD_Text(35, 210, (uint16_t *) One, LCD_BLACK, 1);
        for(i = 0; i < 15; i++)
        {
            LCD_DrawRectangle(30 + 10 - 15 + i, 30 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_BLACK);
        }
        for(i = 0; i < 15; i++)
        {
            //LCD_DrawRectangle(30 + abs(i-7), 30 + 15 - abs(i-7), 160 + i, 160 + i + 1, LCD_WHITE);
            LCD_DrawRectangle(30 + 10 - i, 30 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
        }
        LCD_DrawRectangle(0, 80-4, 180, 195, LCD_WHITE);
        LCD_Text(0, 180, (uint16_t *) Global_Board_State.FX[0].name, LCD_BLACK, 1);
    }
    else if(FX == FX2)
    {
        LCD_DrawRectangle(80, 160-4, 200, 240, LCD_WHITE);
        LCD_Text(115, 210, (uint16_t *) Two, LCD_BLACK, 1);
        for(i = 0; i < 15; i++)
        {
            LCD_DrawRectangle(110 + 10 - 15 + i, 110 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_BLACK);
        }
        for(i = 0; i < 15; i++)
        {
            //LCD_DrawRectangle(110 + abs((i-7)/2*2), 110 + 15 - abs((i-7)/2*2), 160 + i, 160 + i + 1, LCD_WHITE);
            LCD_DrawRectangle(110 + 10 - i, 110 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
        }
        LCD_DrawRectangle(80, 160-4, 180, 195, LCD_WHITE);
        LCD_Text(80, 180, (uint16_t *) Global_Board_State.FX[1].name, LCD_BLACK, 1);
    }
    else if(FX == FX3)
    {
        LCD_DrawRectangle(160, 240-4, 200, 240, LCD_WHITE);
        LCD_Text(195, 210, (uint16_t *) Three, LCD_BLACK, 1);
        for(i = 0; i < 15; i++)
        {
            LCD_DrawRectangle(190 + 10 - 15 + i, 190 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_BLACK);
        }
        for(i = 0; i < 15; i++)
        {
            LCD_DrawRectangle(190 + 10 - i, 190 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
        }
        LCD_DrawRectangle(160, 240-4, 180, 195, LCD_WHITE);
        LCD_Text(160, 180, (uint16_t *) Global_Board_State.FX[2].name, LCD_BLACK, 1);
    }
    else if(FX == FX4)
    {
        LCD_DrawRectangle(240, 320-4, 200, 240, LCD_WHITE);
        LCD_Text(275, 210, (uint16_t *) Four, LCD_BLACK, 1);
        for(i = 0; i < 15; i++)
        {
            LCD_DrawRectangle(270 + 10 - 15 + i, 270 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_BLACK);
        }
        for(i = 0; i < 15; i++)
        {
            //LCD_DrawRectangle(270 - 15 + i, 270 + 15 - i, 160 + i, 160 + i + 1, LCD_WHITE);
            LCD_DrawRectangle(270 + 10 - i, 270 + 10 + i, 160 + i, 160 + i + 1, LCD_WHITE);
        }
        LCD_DrawRectangle(240, 320-4, 180, 195, LCD_WHITE);
        LCD_Text(240, 180, (uint16_t *) Global_Board_State.FX[3].name, LCD_BLACK, 1);
    }
}
void UI_LCD_Activate_FX_mode(int FX)
{
    //static int prev_FX = -1;
    //if(prev_FX == FX) return;

    char One[] = "1";
    char Two[] = "2";
    char Three[] = "3";
    char Four[] = "4";
    char Effect[] = "Effect";
    int i;

    //UI_LCD_Deactivate_FX_mode(prev_FX);
    //prev_FX = FX;

    if(FX == FX1)
    {
    LCD_DrawRectangle(0, 80-4, 200, 240, LCD_RED);
       LCD_Text(35, 210, (uint16_t *) One, LCD_BLACK, 1);
       LCD_DrawRectangle(0, 80-4, 180, 195, LCD_RED);
       LCD_Text(0, 180, (uint16_t *) Global_Board_State.FX[0].name, LCD_BLACK, 1);
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(30 + 10 - i, 30 + 10 + i, 160 + i, 160 + i + 1, LCD_BLACK);
       }
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(30 + 10 - 15 + i, 30 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_RED);
       }
    }
    if(FX == FX2)
    {
       LCD_DrawRectangle(80, 160-4, 200, 240, LCD_ORANGE);
       LCD_Text(115, 210, (uint16_t *) Two, LCD_BLACK, 1);
       LCD_DrawRectangle(80, 160-4, 180, 195, LCD_ORANGE);
       LCD_Text(80, 180, (uint16_t *) Global_Board_State.FX[1].name, LCD_BLACK, 1);
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(110 + 10 - i, 110 + 10 + i, 160 + i, 160 + i + 1, LCD_BLACK);
       }
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(110 + 10 - 15 + i, 110 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_ORANGE);
       }
    }
    if(FX == FX3)
    {
       LCD_DrawRectangle(160, 240-4, 200, 240, LCD_YELLOW);
       LCD_Text(195, 210, (uint16_t *) Three, LCD_BLACK, 1);
       LCD_DrawRectangle(160, 240-4, 180, 195, LCD_YELLOW);
       LCD_Text(160, 180, (uint16_t *) Global_Board_State.FX[2].name, LCD_BLACK, 1);
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(190 + 10 - i, 190 + 10 + i, 160 + i, 160 + i + 1, LCD_BLACK);
       }
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(190 + 10 - 15 + i, 190 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_YELLOW);
       }
    }
    if(FX == FX4)
    {
       LCD_DrawRectangle(240, 320-4, 200, 240, LCD_BLUE);
       LCD_Text(275, 210, (uint16_t *) Four, LCD_BLACK, 1);
       LCD_DrawRectangle(240, 320-4, 180, 195, LCD_BLUE);
       LCD_Text(240, 180, (uint16_t *) Global_Board_State.FX[3].name, LCD_BLACK, 1);
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(270 + 10 - i, 270 + 10 + i, 160 + i, 160 + i + 1, LCD_BLACK);
       }
       for(i = 0; i < 15; i++)
       {
           LCD_DrawRectangle(270 + 10 - 15 + i, 270 + 10 + 15 - i, 160 + i, 160 + i + 1, LCD_BLUE);
       }
    }
}

void UI_LCD_Effect_Setting_mode()
{
    char Enter[] = "Enter";
    char Cancel[] = "Cancel";
    char Param1[] = "Param1";
    char Param2[] = "Param2";
    char Value1[] = "Value1";
    char Value2[] = "Value2";
    char Effect[] = "Effect";
    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    LCD_DrawRectangle(0, 80-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(80, 160-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(160, 240-4, 200, 240, LCD_WHITE);
    LCD_DrawRectangle(240, 320-4, 200, 240, LCD_WHITE);
    LCD_Text(20, 210, (uint16_t *) Enter, LCD_BLACK, 1);
    LCD_Text(95, 210, (uint16_t *) Cancel, LCD_BLACK, 1);
    LCD_Text(175, 210, (uint16_t *) Param1, LCD_BLACK, 1);
    LCD_Text(255, 210, (uint16_t *) Param2, LCD_BLACK, 1);

    LCD_DrawRectangle(160, 240-4, 120, 200 - 4, LCD_RED);
    LCD_DrawRectangle(165, 235-4, 130, 150 - 4, LCD_WHITE);
    LCD_DrawRectangle(165, 235-4, 155, 190 - 4, LCD_WHITE);
    LCD_Text(165, 130, (uint16_t *) Param1, LCD_BLACK, 1);
    LCD_Text(165, 155, (uint16_t *) Value1, LCD_BLACK, 1);

    LCD_DrawRectangle(240, 320-4, 120, 200 - 4, LCD_ORANGE);
    LCD_DrawRectangle(245, 315-4, 130, 150 - 4, LCD_WHITE);
    LCD_DrawRectangle(245, 315-4, 155, 190 - 4, LCD_WHITE);
    LCD_Text(245, 130, (uint16_t *) Param2, LCD_BLACK, 1);
    LCD_Text(245, 155, (uint16_t *) Value2, LCD_BLACK, 1);

    int i;

    for(i = 0; i < 15; i++)
    {
        LCD_DrawRectangle(20 + 10 - i, 20 + 10 + i, 40 + i, 40 + i + 1, LCD_RED);
    }
    for(i = 0; i < 15; i++)
    {
        LCD_DrawRectangle(30 - 15 + i, 30 + 15 - i, 100 + i, 100 + i + 1, LCD_RED);
    }
    LCD_DrawRectangle(80, 240, 60, 100, LCD_WHITE);
    LCD_Text(90, 60, (uint16_t *) Effect, LCD_BLACK, 3);

    char Hold[] = "Hold";
    char Mod[] = "Mod";
    char Pass[] = "Pass";

    LCD_DrawRectangle(260, 300, 20, 100, LCD_WHITE);
    LCD_DrawRectangle(263, 297, 30, 45, LCD_BLACK);
    LCD_Text(264, 30, (uint16_t *) Hold, LCD_WHITE, 1);
    LCD_DrawRectangle(263, 297, 50, 65, LCD_BLACK);
    LCD_Text(264, 50, (uint16_t *) Mod, LCD_WHITE, 1);
    LCD_DrawRectangle(263, 297, 70, 85, LCD_BLACK);
    LCD_Text(264, 70, (uint16_t *) Pass, LCD_WHITE, 1);


}

void UI_LCD_Flex_Mode_Change(int mode)
{
    char Hold[] = "Hold";
    char Mod[] = "Mod";
    char Pass[] = "Pass";

    LCD_DrawRectangle(260, 300, 20, 100, LCD_WHITE);
    LCD_DrawRectangle(263, 297, 30, 45, LCD_BLACK);
    LCD_Text(264, 30, (uint16_t *) Hold, LCD_WHITE, 1);
    LCD_DrawRectangle(263, 297, 50, 65, LCD_BLACK);
    LCD_Text(264, 50, (uint16_t *) Mod, LCD_WHITE, 1);
    LCD_DrawRectangle(263, 297, 70, 85, LCD_BLACK);
    LCD_Text(264, 70, (uint16_t *) Pass, LCD_WHITE, 1);

    if(mode == HOLD_MODE)
    {
        LCD_DrawRectangle(263, 297, 30, 45, LCD_BLUE);
        LCD_Text(264, 30, (uint16_t *) Hold, LCD_YELLOW, 1);
    }
    if(mode == MOD_MODE)
    {
        LCD_DrawRectangle(263, 297, 50, 65, LCD_BLUE);
        LCD_Text(264, 50, (uint16_t *) Mod, LCD_YELLOW, 1);
    }
    if(mode == PASS_MODE)
    {
        LCD_DrawRectangle(263, 297, 70, 85, LCD_BLUE);
        LCD_Text(264, 70, (uint16_t *) Pass, LCD_YELLOW, 1);
    }


}

void UI_LCD_Effect_Shift_Setting(char* FX_name, char* Param1_name, char* Value1_name, char* Param2_name, char* Value2_name)
{
    LCD_DrawRectangle(80, 240, 60, 100, LCD_WHITE);
    LCD_Text(90, 60, (uint16_t *) FX_name, LCD_BLACK, 3);

    LCD_DrawRectangle(160, 240-4, 120, 200 - 4, LCD_RED);
    LCD_DrawRectangle(165, 235-4, 130, 150 - 4, LCD_WHITE);
    LCD_DrawRectangle(165, 235-4, 155, 190 - 4, LCD_WHITE);
    LCD_Text(165, 130, (uint16_t *) Param1_name, LCD_BLACK, 1);
    LCD_Text(165, 155, (uint16_t *) Value1_name, LCD_BLACK, 1);

    LCD_DrawRectangle(240, 320-4, 120, 200 - 4, LCD_ORANGE);
    LCD_DrawRectangle(245, 315-4, 130, 150 - 4, LCD_WHITE);
    LCD_DrawRectangle(245, 315-4, 155, 190 - 4, LCD_WHITE);
    LCD_Text(245, 130, (uint16_t *) Param2_name, LCD_BLACK, 1);
    LCD_Text(245, 155, (uint16_t *) Value2_name, LCD_BLACK, 1);
}

void UI_LCD_Effect_Change_Param(int param, char* val)
{
    if(param == P1)
    {
        LCD_DrawRectangle(165, 235-4, 155, 190 - 4, LCD_WHITE);
        LCD_Text(165, 155, (uint16_t *) val, LCD_BLACK, 1);
    }
    if(param == P2)
    {
        LCD_DrawRectangle(245, 315-4, 155, 190 - 4, LCD_WHITE);
        LCD_Text(245, 155, (uint16_t *) val, LCD_BLACK, 1);
    }
}

void UI_LCD_Tuner_mode()
{

    LCD_DrawRectangle(MIN_SCREEN_X, MAX_SCREEN_X, MIN_SCREEN_Y, MAX_SCREEN_Y, LCD_BLACK);
    char Close[] = "Close";
    LCD_DrawRectangle(0, 50, 0, 60-4, LCD_BLUE);
    LCD_Text(5, 15, (uint16_t *) Close, LCD_BLACK, 1);
    LCD_DrawRectangle(50, 270, 160, 162, LCD_WHITE);

    /*int i;
    for(i = 16; i < 8000; i++)
    {
        Tune(i);
    }*/


}

void UI_LCD_Tune(int freq)
{

   int i;
   char Low[] = "Low";
   char High[] = "High";

   static int16_t olddiff = 0;
   static int16_t old_closest_note = 254;
   if(freq < 0)
   {
       olddiff = 0;
       old_closest_note = 254;
   }
   int16_t note_cents = cents(freq);
   int16_t closest_note = (note_cents + 50)/100;
   //char note_name[7] = note_names[closest_note];
   int16_t diff = (note_cents - closest_note*100)*110/50;

   if(old_closest_note != closest_note)
   {
       LCD_DrawRectangle(80, 240, 60, 100, LCD_WHITE);
       if(closest_note >= 0 && closest_note <= 107)
       {
           LCD_Text(90, 60, (uint16_t *) note_names[closest_note], LCD_BLACK, 3);
       }
       if(closest_note < 0)
       {
           LCD_Text(90, 60, (uint16_t *) Low, LCD_BLACK, 3);
       }
       if(closest_note > 107)
       {
           LCD_Text(90, 60, (uint16_t *) High, LCD_BLACK, 3);
       }
       old_closest_note = closest_note;
   }


   for(i = 0; i < 16; i++)
   {
       LCD_DrawRectangle(161 - 15 + i + olddiff, 161 + 15 - i + olddiff, 145 + i, 145 + i + 1, LCD_BLACK);
   }
   for(i = 0; i < 11; i++)
   {
      LCD_DrawRectangle(50 + i*22, 50 + i*22 + 1, 150, 170, LCD_WHITE);
   }
   uint16_t color;
   if(abs(diff) < 33) color = LCD_GREEN;
   else if(abs(diff) < 77) color = LCD_YELLOW;
   else color = LCD_RED;
   for(i = 0; i < 15; i++)
   {
       LCD_DrawRectangle(161 - 15 + i + diff, 161 + 15 - i + diff, 145 + i, 145 + i + 1, color);
   }
   olddiff = diff;
   DELAY_US(20000);


}

void State_Initialize()
{
    LCD_Init();
    UI_LCD_Logo_mode();

    int i;
    for(i = 0; i < NumFX; i++)
    {
        Global_FXLib[i].name = FX_names[i];
        Global_FXLib[i].param1 = param1_names[i];
        Global_FXLib[i].value1 = value1_init[i];
        Global_FXLib[i].param2 = param2_names[i];
        Global_FXLib[i].value2 = value2_init[i];
        Global_FXLib[i].FX_index = i;
    }

    Global_Board_State.boardMode = Logo_Mode;
    for(i = 0; i < 4; i++)
    {
        Global_Board_State.FX[i] = Global_FXLib[Bypass];
    }
    Global_Board_State.RF_Connected = false;
    Global_Board_State.currentEffect = FX1;
    Global_Board_State.Flex_State = HOLD_MODE;


}

bool BT_Connect()
{
    DELAY_US(100000);
    return false;
}
void Attempt_RF_Connect()
{
    bool done = false;
    Global_Board_State.boardMode = RF_Connect_Mode;
    UI_LCD_RF_Connect_mode();
    if(BT_Connect())
    {
        Global_Board_State.RF_Connected = true;
        done = true;
        Main_Mode();
    }
    else
    {
        Global_Board_State.RF_Connected = false;
        UI_LCD_RF_Try_Again_mode();
    }

}

void Main_Mode()
{
    Global_Board_State.boardMode = Main_Menu_Mode;
    UI_LCD_Main_mode();
    UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
    /*while(1){
        while(!Button1 && !Button2 && !Button3 && !Button4 &&
                !ButtonUp && !ButtonDown && !Switch1 && !Switch2 &&
                !Switch3 && !Switch4){};
        if(Switch1)
        {
            Switch1 = false;
            if(Global_Board_State.currentEffect != FX1)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX1;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch2)
        {
            Switch2 = false;
            if(Global_Board_State.currentEffect != FX2)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX2;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch3)
        {
            Switch3 = false;
            if(Global_Board_State.currentEffect != FX3)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX3;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch4)
        {
            Switch4 = false;
            if(Global_Board_State.currentEffect != FX4)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX4;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(ButtonUp) return EXIT_RF;
        if(ButtonDown) return EXIT_Tuner;
    }*/
}

void Tuner()
{
    Global_Board_State.boardMode = Tuner_Mode;
    UI_LCD_Tuner_mode();
    UI_LCD_Tune(-1);
}

void Initialize_Board()
{
    State_Initialize();
    Attempt_RF_Connect();
    while(Global_Board_State.RF_Connected == false)
    {
        while(Button1 == false && Button2 == false){};
        if(Button1)
        {
            Clear_Buttons();
            Attempt_RF_Connect();
        }
        else if(Button2)
        {
            Clear_Buttons();
            break;
        }
    }
    Main_Mode();

}

void State_Change()
{
    static int FX_Modify = -1;
    if(Global_Board_State.boardMode == Main_Menu_Mode)
    {
        if(Switch1)
        {
            Clear_Buttons();
            if(Global_Board_State.currentEffect != FX1)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX1;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch2)
        {
            Clear_Buttons();
            if(Global_Board_State.currentEffect != FX2)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX2;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch3)
        {
            Clear_Buttons();
            if(Global_Board_State.currentEffect != FX3)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX3;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(Switch4)
        {
            Clear_Buttons();
            if(Global_Board_State.currentEffect != FX4)
            {
                UI_LCD_Deactivate_FX_mode(Global_Board_State.currentEffect);
                Global_Board_State.currentEffect = FX4;
                UI_LCD_Activate_FX_mode(Global_Board_State.currentEffect);
            }
        }
        if(ButtonUp)
        {
            Clear_Buttons();
            Attempt_RF_Connect();
        }

        if(ButtonDown)
        {
            Clear_Buttons();
            Tuner();
        }

        if(Button1)
        {
            Clear_Buttons();
            Global_Board_State.boardMode = Effect_Setting_Mode;
            FX_Modify = FX1;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX1].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX1].value2;
            Prev_Board_State = Global_Board_State;
            UI_LCD_Effect_Setting_mode();
            UI_LCD_Flex_Mode_Change(Global_Board_State.Flex_State);
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX1].name, Global_Board_State.FX[FX1].param1, val1, Global_Board_State.FX[FX1].param2, val2);
        }

        else if(Button2)
        {
            Clear_Buttons();
            Global_Board_State.boardMode = Effect_Setting_Mode;
            FX_Modify = FX2;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX2].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX2].value2;
            Prev_Board_State = Global_Board_State;
            UI_LCD_Effect_Setting_mode();
            UI_LCD_Flex_Mode_Change(Global_Board_State.Flex_State);
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX2].name, Global_Board_State.FX[FX2].param1, val1, Global_Board_State.FX[FX2].param2, val2);
        }

        else if(Button3)
        {
            Clear_Buttons();
            Global_Board_State.boardMode = Effect_Setting_Mode;
            FX_Modify = FX3;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX3].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX3].value2;
            Prev_Board_State = Global_Board_State;
            UI_LCD_Effect_Setting_mode();
            UI_LCD_Flex_Mode_Change(Global_Board_State.Flex_State);
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX3].name, Global_Board_State.FX[FX3].param1, val1, Global_Board_State.FX[FX3].param2, val2);
        }

        else if(Button4)
        {
            Clear_Buttons();
            Global_Board_State.boardMode = Effect_Setting_Mode;
            FX_Modify = FX4;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX4].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX4].value2;
            Prev_Board_State = Global_Board_State;
            UI_LCD_Effect_Setting_mode();
            UI_LCD_Flex_Mode_Change(Global_Board_State.Flex_State);
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX4].name, Global_Board_State.FX[FX4].param1, val1, Global_Board_State.FX[FX4].param2, val2);
        }


    }
    if(Global_Board_State.RF_Connected == false && Global_Board_State.boardMode == RF_Connect_Mode)
    {
        if(Button1)
        {
            Clear_Buttons();
            Attempt_RF_Connect();
        }
        if(Button2)
        {
            Clear_Buttons();
            Main_Mode();

        }
    }
    if(Global_Board_State.boardMode == Effect_Setting_Mode)
    {
        if(Button1)
        {
            Clear_Buttons();
            Main_Mode();
        }
        if(Button2)
        {
            Clear_Buttons();
            Global_Board_State = Prev_Board_State;
            Main_Mode();
        }
        if(Button3 && Button4)
        {
            Clear_Buttons();
            Global_Board_State.Flex_State++;
            if(Global_Board_State.Flex_State > 2) Global_Board_State.Flex_State = 0;
            UI_LCD_Flex_Mode_Change(Global_Board_State.Flex_State);
        }
        if(Button3)
        {
            Clear_Buttons();
            Global_Board_State.FX[FX_Modify].value1++;
            if(Global_Board_State.FX[FX_Modify].value1 > 9) Global_Board_State.FX[FX_Modify].value1 = 0;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX_Modify].value1;
            UI_LCD_Effect_Change_Param(P1, val1);
        }
        if(Button4)
        {
            Clear_Buttons();
            Global_Board_State.FX[FX_Modify].value2++;
            if(Global_Board_State.FX[FX_Modify].value2 > 9) Global_Board_State.FX[FX_Modify].value2 = 0;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX_Modify].value2;
            UI_LCD_Effect_Change_Param(P2, val2);
        }
        if(ButtonUp)
        {
            Clear_Buttons();
            int FX_tag = Global_Board_State.FX[FX_Modify].FX_index + 1;
            if(FX_tag >= NumFX) FX_tag = 0;
            Global_Board_State.FX[FX_Modify].name = Global_FXLib[FX_tag].name;
            Global_Board_State.FX[FX_Modify].param1 = Global_FXLib[FX_tag].param1;
            Global_Board_State.FX[FX_Modify].param2 = Global_FXLib[FX_tag].param2;
            Global_Board_State.FX[FX_Modify].FX_index = Global_FXLib[FX_tag].FX_index;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX_Modify].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX_Modify].value2;
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX_Modify].name, Global_Board_State.FX[FX_Modify].param1, val1, Global_Board_State.FX[FX_Modify].param2, val2);

        }
        if(ButtonDown)
        {
            Clear_Buttons();
            int FX_tag = Global_Board_State.FX[FX_Modify].FX_index - 1;
            if(FX_tag < 0) FX_tag = NumFX - 1;
            Global_Board_State.FX[FX_Modify].name = Global_FXLib[FX_tag].name;
            Global_Board_State.FX[FX_Modify].param1 = Global_FXLib[FX_tag].param1;
            Global_Board_State.FX[FX_Modify].param2 = Global_FXLib[FX_tag].param2;
            Global_Board_State.FX[FX_Modify].FX_index = Global_FXLib[FX_tag].FX_index;
            char val1[] = "0";
            val1[0] += Global_Board_State.FX[FX_Modify].value1;
            char val2[] = "0";
            val2[0] += Global_Board_State.FX[FX_Modify].value2;
            UI_LCD_Effect_Shift_Setting(Global_Board_State.FX[FX_Modify].name, Global_Board_State.FX[FX_Modify].param1, val1, Global_Board_State.FX[FX_Modify].param2, val2);

        }

    }
    if(Global_Board_State.boardMode == Tuner_Mode)
    {
        if(ButtonUp)
        {
            Clear_Buttons();
            Main_Mode();
        }
    }
}

void Clear_Buttons()
{
    ButtonUp = 0;
    ButtonDown = 0;
    Button1 = 0;
    Button2 = 0;
    Button3 = 0;
    Button4 = 0;
    Switch1 = 0;
    Switch2 = 0;
    Switch3 = 0;
    Switch4 = 0;
}



