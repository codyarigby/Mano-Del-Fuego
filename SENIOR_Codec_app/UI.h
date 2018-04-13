/*
 * UI.h
 *
 *  Created on: Apr 3, 2018
 *      Author: Cody
 */

#ifndef UI_H_
#define UI_H_


#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "LCD.h"
#include "math.h"


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

#define NumFX 14

//effect defines
#define BYPASS      0
#define WAH         1
#define VOLSWELL    2
#define FLANGER     3
#define PITCHDOWN   4
#define PITCHUP     5
#define DISTORTION  6
#define FUZZ        7
#define BASSBOOST   8
#define ECHO        9
#define CHORUS      10
#define TREMOLOO     11
#define PHASER      12
#define VIBRATO     13





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

extern Board_State Global_Board_State, Prev_Board_State;
extern Effect Global_FXLib[NumFX];
extern char FX_names[NumFX][11];
extern char param1_names[NumFX][4];
extern int value1_init[NumFX];
extern char param2_names[NumFX][4];
extern int value2_init[NumFX];

extern bool Button1;
extern bool Button2;
extern bool Button3;
extern bool Button4;
extern bool ButtonUp;
extern bool ButtonDown;
extern bool Switch1;
extern bool Switch2;
extern bool Switch3;
extern bool Switch4;

extern bool state_change_flag;

extern bool rx_flag;




#endif /* UI_H_ */
