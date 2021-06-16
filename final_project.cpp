/***********************************************************************

Dympna Tinezsia Adhisti - 07211840000029
Modified from Mini Clock v1.0, Jul 2014 by Nick Hall
Distributed under the terms of the GPL.

For help on how to build the clock see my blog:
http://123led.wordpress.com/

***********************************************************************/
#include <FontLEDClock.h>
#include <Wire.h>
#include <Button.h>
#include "LedControl.h"
#include "RTClib.h"

//pin 12 ke DIN, pin 11 ke CLK, pin 10 ke LOAD
//sets 3 pin 12, 11 & 10 dan set parameter terakhir ke jumlah display
LedControl lc = LedControl(12, 11, 10, 4);

//variable global
byte intensity = 7;            // default intensity/brightness (0-15)
byte clock_mode = 0;           // default clock mode (clock)
byte old_mode = clock_mode;    
int rtc[7];                    //array untuk menyimpan

//konstanta
#define NUM_DISPLAY_MODES 1  
#define cls clear_display    

RTC_DS1307 ds1307;

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
  //init matrix
  for (int address = 0; address < devices; address++)
  {
    lc.shutdown(address, false);
    lc.setIntensity(address, intensity);
    lc.clearDisplay(address);
  }

//Setup DS1307 RTC
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin();
#endif
  ds1307.begin();

  if (!ds1307.isrunning())
  {
    Serial.println("RTC is NOT running!");
    ds1307.adjust(DateTime(__DATE__, __TIME__)); // sets the RTC to the date & time this sketch was compiled
  }
}

void loop()
{
  switch (clock_mode)
  {
  case 0:
    small_mode();
    break;
  case 1:
    set_time();
    break;
  case 2:
    set_datetime();
    break;
  }
}

void plot(byte x, byte y, byte val)
{
  //select which matrix depending on the x coord
  byte address;
  if (x >= 0 && x <= 7)
  {
    address = 0;
  }
  if (x >= 8 && x <= 15)
  {
    address = 1;
    x = x - 8;
  }
  if (x >= 16 && x <= 23)
  {
    address = 2;
    x = x - 16;
  }
  if (x >= 24 && x <= 31)
  {
    address = 3;
    x = x - 24;
  }

  if (val == 1)
  {
    lc.setLed(address, y, x, true);
  }
  else
  {
    lc.setLed(address, y, x, false);
  }
}

//clear screen
void clear_display()
{
  for (byte address = 0; address < 4; address++)
  {
    lc.clearDisplay(address);
  }
}

//fade screen down
void fade_down()
{
  //fade from global intensity to 1
  for (byte i = intensity; i > 0; i--)
  {
    for (byte address = 0; address < 4; address++)
    {
      lc.setIntensity(address, i);
    }
    delay(30);
  }

  clear_display();

  //reset intentsity
  for (byte address = 0; address < 4; address++)
  {
    lc.setIntensity(address, intensity);
  }
}

//tiny_font
void tiny_font(byte x, byte y, char c)
{
  byte dots;
  if (c >= 'A' && c <= 'Z' || (c >= 'a' && c <= 'z'))
  {
    c &= 0x1F; // A-Z maps to 1-26
  }
  else if (c >= '0' && c <= '9')
  {
    c = (c - '0') + 32;
  }
  else if (c == ' ')
  {
    c = 0; // space
  }
  else if (c == '.')
  {
    c = 27; // full stop
  }
  else if (c == ':')
  {
    c = 28; // colon
  }
  else if (c == '\'')
  {
    c = 29; // single quote mark
  }
  else if (c == '!')
  {
    c = 30; // single quote mark
  }
  else if (c == '?')
  {
    c = 31; // single quote mark
  }

  for (byte col = 0; col < 3; col++)
  {
    dots = pgm_read_byte_near(&mytinyfont[c][col]);
    for (char row = 0; row < 5; row++)
    {
      if (dots & (16 >> row))
        plot(x + col, y + row, 1);
      else
        plot(x + col, y + row, 0);
    }
  }
}

//menunjukkan jam
void small_mode()
{
  char textchar[8];     // the 16 characters on the display
  byte mins = 100;      
  byte secs = rtc[0];   
  byte old_secs = secs; 

  cls();

  //run clock main loop
  while (1)
  {
    get_time();

    if (button_A.uniquePress())
    {
      switch_mode();
      return;
    }

    if (button_C.uniquePress())
    {
      set_intensity();
      return;
    }

    //update display sekon
    secs = rtc[0];
    if (secs != old_secs)
    {
      //secs
      char buffer[3];
      itoa(secs, buffer, 10);

      //jika secs kurang dari 0, e.g. "03" secs, itoa mengkonversi karakter dengan spasi "3 ".
      if (secs < 10)
      {
        buffer[1] = buffer[0];
        buffer[0] = '0';
      }
      tiny_font(20, 1, ':');       
      tiny_font(24, 1, buffer[0]); 
      tiny_font(28, 1, buffer[1]); 
      old_secs = secs;
    }

    //ganti tanggal untuk setiap 10 detik
    if (secs % 10 == 0)
    {
      display_date();
      return;
    }

    //update menit
    if (mins != rtc[1])
    {
      //reset
      mins = rtc[1];
      byte hours = rtc[2];

      //set karakter
      char buffer[3];
      
      //set karakter jam
      itoa(hours, buffer, 10);
      if (hours < 10)
      {
        buffer[1] = buffer[0];
        buffer[0] = '0';
      }
      textchar[0] = buffer[0];
      textchar[1] = buffer[1];
      textchar[2] = ':';

      //set karakter menit
      itoa(mins, buffer, 10);
      if (mins < 10)
      {
        buffer[1] = buffer[0];
        buffer[0] = '0';
      }
      textchar[3] = buffer[0];
      textchar[4] = buffer[1];
      textchar[5] = ':';

      //set karakter detik
      buffer[3];
      secs = rtc[0];
      itoa(secs, buffer, 10);
      if (secs < 10)
      {
        buffer[1] = buffer[0];
        buffer[0] = '0';
      }
      textchar[6] = buffer[0];
      textchar[7] = buffer[1];

      //print each char
      byte x = 0;
      byte y = 0;
      for (byte x = 0; x < 6; x++)
      {
        tiny_font(x * 4, 1, textchar[x]);
      }
    }
    delay(50);
  }
  fade_down();
}

//menunjukkan tanggal
void display_date()
{
  cls();

  char textchar[8];
  byte date = rtc[4];
  byte month = rtc[5] - 1;
  byte year = rtc[6] - 2000;

  //array nama bulan
  char monthnames[12][4] = {
      "Jan", "Feb", "Mar", "Apr", "Mei", "Jun",
      "Jul", "Agu", "Sep", "Okt", "Nov", "Des"};
  
  //print tanggal
  char buffer[3];
  itoa(date, buffer, 10);
  tiny_font(0, 1, buffer[0]);
  if (date > 9)
  {
    tiny_font(5, 1, buffer[1]);
  }
  
  //print bulan
  //get length of text in pixels, that way we can centre it on the display by divindin the remaining pixels b2 and using that as an offset
  byte len = 0;
  while (monthnames[month][len])
  {
    len++;
  };
  byte offset = (28 - ((len - 1) * 4)) / 2;
  int i = 0;
  while (monthnames[month][i])
  {
    tiny_font((i * 4) + offset, 1, monthnames[month][i]);
    i++;
  }

  //print tahun
  itoa(year, buffer, 10);
  tiny_font(24, 1, buffer[0]);
  tiny_font(28, 1, buffer[1]);
  delay(5000);
  fade_down();
}

//ganti mode
void switch_mode()
{
  //remember mode we are in. We use this value if we go into settings mode, so we can change back from settings mode (6) to whatever mode we were in.
  old_mode = clock_mode;

  char *modes[] = {
      "Clock", "Set Time", "Set Date"};

  byte next_clock_mode;
  byte firstrun = 1;

  //loop waiting for button (timeout after 35 loops to return to mode X)
  for (int count = 0; count < 35; count++)
  {
    if (button_A.uniquePress() || firstrun == 1)
    {
      count = 0;
      cls();

      if (firstrun == 0)
      {
        clock_mode++;
      }
      if (clock_mode > NUM_DISPLAY_MODES + 1)
      {
        clock_mode = 0;
      }

      //print arrown and current clock_mode name on line one and print next clock_mode name on line two
      char str_top[9];

      //strcpy (str_top, "-");
      strcpy(str_top, modes[clock_mode]);

      next_clock_mode = clock_mode + 1;
      if (next_clock_mode > NUM_DISPLAY_MODES + 1)
      {
        next_clock_mode = 0;
      }

      byte i = 0;
      while (str_top[i])
      {
        tiny_font(i * 4, 1, str_top[i]);
        i++;
      }
      firstrun = 0;
    }
    delay(50);
  }
}

//ganti intensitas
void set_intensity()
{

  cls();

  byte i = 0;
  char text[7] = "Bright";
  while (text[i])
  {
    tiny_font((i * 4) + 4, 0, text[i]);
    i++;
  }

  //wait for button input
  while (!button_C.uniquePress())
  {

    levelbar(0, 6, (intensity * 2) + 2, 2); //display the intensity level as a bar
    while (button_D.isPressed())
    {
      if (intensity == 15)
      {
        intensity = 0;
        cls();
      }
      else
      {
        intensity++;
      }

      i = 0;
      while (text[i])
      {
        tiny_font((i * 4) + 4, 0, text[i]);
        i++;
      }

      //display the intensity level as a bar
      levelbar(0, 6, (intensity * 2) + 2, 2);

      //change the brightness setting on the displays
      for (byte address = 0; address < 4; address++)
      {
        lc.setIntensity(address, intensity);
      }
      delay(150);
    }
  }
}

// display a horizontal bar on the screen at offset xposr by ypos with height and width of xbar, ybar
void levelbar(byte xpos, byte ypos, byte xbar, byte ybar)
{
  for (byte x = 0; x < xbar; x++)
  {
    for (byte y = 0; y <= ybar; y++)
    {
      plot(x + xpos, y + ypos, 1);
    }
  }
}

//set time
void set_time()
{
  cls();

  //fill settings with current clock values read from clock
  get_time();
  byte set_scnd = rtc[0]; 
  byte set_min = rtc[1];
  byte set_hr = rtc[2];
  byte set_date = rtc[4];
  byte set_mnth = rtc[5];
  int set_yr = rtc[6];

  //Set function - we pass in: which 'set' message to show at top, current value, reset value, and rollover limit.
  set_hr = set_timevalue(2, set_hr, 0, 23);
  set_min = set_timevalue(1, set_min, 0, 59);
  set_scnd = set_timevalue(0, set_scnd, 0, 59);

  ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));

  cls();
  clock_mode = old_mode;
}

//set tanggal
void set_datetime()
{
  cls();

  //fill settings with current clock values read from clock
  get_time();
  byte set_scnd = rtc[0]; 
  byte set_min = rtc[1];
  byte set_hr = rtc[2];
  byte set_date = rtc[4];
  byte set_mnth = rtc[5];
  int set_yr = rtc[6];

  //Set function - we pass in: which 'set' message to show at top, current value, reset value, and rollover limit.
  set_date = set_datevalue(2, set_date, 1, 31);
  set_mnth = set_datevalue(1, set_mnth, 1, 12);
  set_yr = set_datevalue(0, set_yr, 2013, 2099);

  ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));

  cls();
  clock_mode = old_mode;
}

int set_timevalue(byte message, int current_value, int reset_value, int rollover_limit)
{
  cls();
  char messages[4][17] = {
      "Set Sncd", "Set Mins", "Set Hour"};

  //Print "set xyz" top line
  byte i = 0;
  while (messages[message][i])
  {
    tiny_font(i * 4, 1, messages[message][i]);
    i++;
  }

  delay(500);
  cls();

  //print digits bottom line
  char buffer[5] = "    ";
  itoa(current_value, buffer, 10);
  tiny_font(0, 1, buffer[0]);
  tiny_font(4, 1, buffer[1]);
  tiny_font(8, 1, buffer[2]);
  tiny_font(12, 1, buffer[3]);

  delay(300);
  //wait for button input
  while (!button_A.uniquePress())
  {
    while (button_B.isPressed())
    {

      if (current_value < rollover_limit)
      {
        current_value++;
      }
      else
      {
        current_value = reset_value;
      }
      //print the new value
      itoa(current_value, buffer, 10);
      tiny_font(0, 1, buffer[0]);
      tiny_font(4, 1, buffer[1]);
      tiny_font(8, 1, buffer[2]);
      tiny_font(12, 1, buffer[3]);
      delay(150);
    }
  }
  return current_value;
}

int set_datevalue(byte message, int current_value, int reset_value, int rollover_limit)
{
  cls();
  char messages[4][17] = {
      "Set Year", "Set Mnth", "Set Day"};

  byte i = 0;
  while (messages[message][i])
  {
    tiny_font(i * 4, 1, messages[message][i]);
    i++;
  }

  delay(500);
  cls();

  //print digits bottom line
  char buffer[5] = "    ";
  itoa(current_value, buffer, 10);
  tiny_font(0, 1, buffer[0]);
  tiny_font(4, 1, buffer[1]);
  tiny_font(8, 1, buffer[2]);
  tiny_font(12, 1, buffer[3]);

  delay(300);
  //wait for button input
  while (!button_A.uniquePress())
  {
    while (button_B.isPressed())
    {
      if (current_value < rollover_limit)
      {
        current_value++;
      }
      else
      {
        current_value = reset_value;
      }
      //print the new value
      itoa(current_value, buffer, 10);
      tiny_font(0, 1, buffer[0]);
      tiny_font(4, 1, buffer[1]);
      tiny_font(8, 1, buffer[2]);
      tiny_font(12, 1, buffer[3]);
      delay(150);
    }
  }
  return current_value;
}

void get_time()
{
  DateTime now = ds1307.now();

  rtc[6] = now.year();
  rtc[5] = now.month();
  rtc[4] = now.day();
  rtc[3] = now.dayOfTheWeek(); //returns 0-6 where 0 = Sunday
  rtc[2] = now.hour();
  rtc[1] = now.minute();
  rtc[0] = now.second();
}
