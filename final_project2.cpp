#include "LedControl.h"
#include <Button.h>

LedControl lc = LedControl(12, 11, 10, 4);

//global variable
unsigned long delaytime = 200;
int settanggal = true;
int buttonstate;
int buttonpre = false;
byte intensity = 7;         // Default intensity/brightness (0-15)
byte clock_mode = 0;        // Default clock mode. Default = 0 (basic_mode)
bool random_mode = 0;       // Define random mode - changes the display type every few hours. Default = 0 (off)
byte old_mode = clock_mode; // Stores the previous clock mode, so if we go to date or whatever, we know what mode to go back to after.

//define constants
#define NUM_DISPLAY_MODES 0  // Number display modes (conting zero as the first mode)
#define NUM_SETTINGS_MODES 4 // Number settings modes = 6 (conting zero as the first mode)
#define SLIDE_DELAY 20       // The time in milliseconds for the slide effect per character in slide mode. Make this higher for a slower effect
//#define cls clear_display    // Clear display

//jam
uint8_t secs = 54;
uint8_t minutes = 59;
uint8_t hours = 23;

//date
uint8_t days = 29;
uint8_t months = 2;
uint8_t years = 20;

int delayjamtag;

byte Digits[12][3] = {
    {B00111100, B01000010, B00111100}, //0
    {B00000000, B01111110, B00000000}, //1
    {B01001110, B01001010, B01111010}, //2
    {B01001010, B01001010, B01111110}, //3
    {B00011000, B00101000, B01111110}, //4
    {B01111010, B01001010, B01001110}, //5
    {B01111110, B01001010, B01001110}, //6
    {B01000000, B01001110, B01110000}, //7
    {B01111110, B01001010, B01111110}, //8
    {B01111010, B01001010, B01111110}, //9
    {B00000000, B00100100, B00000000}, //:
    {B00001000, B00001000, B00001000}  //-
};

int dig1;
int dig2;
int dig3;
int dig4;
int dig5;
int dig6;

Button button_A = Button(2, BUTTON_PULLUP); // mode button
Button button_B = Button(3, BUTTON_PULLUP); // set button
Button button_C = Button(4, BUTTON_PULLUP); // adjust down
Button button_D = Button(5, BUTTON_PULLUP); // adjust up

void setup()
{
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);

    Serial.begin(9600);

    int devices = lc.getDeviceCount();
    for (int address = 0; address < devices; address++)
    {
        lc.shutdown(address, false);
        lc.setIntensity(address, intensity);
        lc.clearDisplay(address);
    }
}

void loop()
{
    small_mode()
}

void small_mode()
{
    dig1 = (hours / 10) % 10;
    dig2 = hours % 10;
    dig3 = (minutes / 10) % 10;
    dig4 = minutes % 10;
    dig5 = (secs / 10) % 10;
    dig6 = secs % 10;


    for (int i = 0; i < 3; i++)
    {
        lc.setColumn(0, i + 5, Digits[dig6][i]);
        lc.setColumn(0, i + 1, Digits[dig5][i]);
        lc.setColumn(1, i + 5, Digits[10][i]);
        lc.setColumn(1, i + 1, Digits[dig4][i]);
        lc.setColumn(2, i + 5, Digits[dig3][i]);
        lc.setColumn(2, i + 1, Digits[10][i]);
        lc.setColumn(3, i + 5, Digits[dig2][i]);
        lc.setColumn(3, i + 1, Digits[dig1][i]);
    }
    if (secs > 59)
    {
        secs = 0;
        minutes++;
    }
    if (minutes > 59)
    {
        minutes = 0;
        hours++;
    }
    if (hours > 23)
    {
        hours = 0;
        days++;
    }
    secs = secs + 1;
    delay(delaytime);
    if(secs%5 == 0){
        date_mode()
    }
}


void date_mode()
{
    dig1 = (years / 10) % 10;
    dig2 = years % 10;
    dig3 = (months / 10) % 10;
    dig4 = months % 10;
    dig5 = (days / 10) % 10;
    dig6 = days % 10;

    for (int i = 0; i < 3; i++)
    {
        lc.setColumn(0, i + 1, Digits[dig1][i]);
        lc.setColumn(0, i + 5, Digits[dig2][i]);
        lc.setColumn(1, i + 5, Digits[11][i]);
        lc.setColumn(1, i + 1, Digits[dig4][i]);
        lc.setColumn(2, i + 5, Digits[dig3][i]);
        lc.setColumn(2, i + 1, Digits[11][i]);
        lc.setColumn(3, i + 1, Digits[dig5][i]);
        lc.setColumn(3, i + 5, Digits[dig6][i]);
    }
    if (secs > 59)
    {
        secs = 0;
        minutes++;
    }
    if (minutes > 59)
    {
        minutes = 0;
        hours++;
    }
    if (hours > 23)
    {
        hours = 0;
        days++;
    }
}