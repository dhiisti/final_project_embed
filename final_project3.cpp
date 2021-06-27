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
byte intensity = 7;  // default intensity/brightness (0-15)
byte clock_mode = 0; // default clock mode (clock)
byte set_mode = 0;   // default clock mode (clock)
byte old_mode = clock_mode;
byte old_set_mode = set_mode;
int rtc[7]; //array untuk menyimpan

//konstanta
#define NUM_DISPLAY_MODES 1
#define NUM_SETTING_MODES 3
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
        address = 3;
    }
    if (x >= 8 && x <= 15)
    {
        address = 2;
        x = x - 8;
    }
    if (x >= 16 && x <= 23)
    {
        address = 1;
        x = x - 16;
    }
    if (x >= 24 && x <= 31)
    {
        address = 0;
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

//menunjukkan jam
void small_mode()
{
    char textchar[8]; // the 16 characters on the display
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

        if (button_D.uniquePress())
        {
            up_intensity();
            return;
        }

        if (button_C.uniquePress())
        {
            down_intensity();
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
        if (secs == 10 || secs == 25 || secs == 40 || secs == 55)
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

    if (button_A.uniquePress())
    {
        switch_mode();
        return;
    }

    if (button_D.uniquePress())
    {
        up_intensity();
        return;
    }

    if (button_C.uniquePress())
    {
        down_intensity();
        return;
    }

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
        "Jam", "Set Jam", "Set Tggl"};

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
void up_intensity()
{
    byte i = 0;

    //wait for button input
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (intensity == 15)
            {
                intensity = 0;
            }
            else
            {
                intensity++;
            }

            //change the brightness setting on the displays
            for (byte address = 0; address < 4; address++)
            {
                lc.setIntensity(address, intensity);
            }
            delay(150);
        }

        if (button_C.uniquePress())
        {
            down_intensity();
            return;
        }
    }
}

void down_intensity()
{
    byte i = 0;
    while (!button_A.uniquePress())
    {
        while (button_C.isPressed())
        {
            if (intensity > 0)
            {
                intensity--;
            }
            else
            {
                intensity = 15;
            }

            //change the brightness setting on the displays
            for (byte address = 0; address < 4; address++)
            {
                lc.setIntensity(address, intensity);
            }
            delay(150);
        }
    }
}

//set time
//dislpay menu to change the clock settings
void set_time()
{
    cls();
    char *set[] = {"Jam", "Menit", "Detik"};

    byte next_set_mode;
    byte firstrun = 1;

    for (int count = 0; count < 35; count++)
    {
        if (button_B.uniquePress() || firstrun == 1)
        {
            count = 0;
            cls();

            if (firstrun == 0)
            {
                set_mode++;
            }
            if (set_mode > NUM_DISPLAY_MODES + 1)
            {
                set_mode = 0;
            }

            //print arrown and current set_mode name on line one and print next set_mode name on line two
            char str_top[9];

            //strcpy (str_top, "-");
            strcpy(str_top, set[set_mode]);

            next_set_mode = set_mode + 1;
            if (next_set_mode > NUM_DISPLAY_MODES + 1)
            {
                next_set_mode = 0;
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

    // cls();
    switch (set_mode)
    {
    case 0:
        hour_mode();
        break;
    case 1:
        min_mode();
        break;
    case 2:
        secs_mode();
        break;
    }

    clock_mode = old_mode;
}

void hour_mode()
{
    get_time();
    byte set_scnd = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];

    cls();
    set_hr = rtc[2];
    char buffer[5] = "    ";
    itoa(set_hr, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_hr < 23)
            {
                set_hr++;
            }
            else
            {
                set_hr = 0;
            }
            //print the new value
            itoa(set_hr, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_hr > 0)
            {
                set_hr--;
            }
            else
            {
                set_hr = 23;
            }
            //print the new value
            itoa(set_hr, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));
    }

    fade_down();
    clock_mode = old_mode;
}

void min_mode()
{
    get_time();
    byte set_scnd = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];

    cls();
    set_min = rtc[1];
    char buffer[5] = "    ";
    itoa(set_min, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_min < 59)
            {
                set_min++;
            }
            else
            {
                set_min = 0;
            }
            //print the new value
            itoa(set_min, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_min > 0)
            {
                set_min--;
            }
            else
            {
                set_min = 59;
            }
            //print the new value
            itoa(set_min, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));
    }

    fade_down();
    clock_mode = old_mode;
}

void secs_mode()
{
    get_time();
    byte set_secs = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];

    get_time();
    cls();
    set_secs = rtc[0];
    char buffer[5] = "    ";
    itoa(set_secs, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);

    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_secs < 59)
            {
                set_secs++;
            }
            else
            {
                set_secs = 0;
            }
            //print the new value
            itoa(set_secs, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_secs > 0)
            {
                set_secs--;
            }
            else
            {
                set_secs = 59;
            }
            //print the new value
            itoa(set_secs, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_secs));
    }

    fade_down();
    clock_mode = old_mode;
}

//set date time
void set_datetime()
{
    //remember mode we are in. We use this value if we go into settings mode, so we can change back from settings mode (6) to whatever mode we were in.
    old_set_mode = set_mode;
    cls();
    char *set[] = {
        "Hari", "Bulan", "Tahun"};

    byte next_set_mode;
    byte firstrun = 1;

    //loop waiting for button (timeout after 35 loops to return to mode X)
    for (int count = 0; count < 35; count++)
    {
        if (button_B.uniquePress() || firstrun == 1)
        {
            count = 0;
            cls();

            if (firstrun == 0)
            {
                set_mode++;
            }
            if (set_mode > NUM_DISPLAY_MODES + 1)
            {
                set_mode = 0;
            }

            //print arrown and current set_mode name on line one and print next set_mode name on line two
            char str_top[9];

            //strcpy (str_top, "-");
            strcpy(str_top, set[set_mode]);

            next_set_mode = set_mode + 1;
            if (next_set_mode > NUM_DISPLAY_MODES + 1)
            {
                next_set_mode = 0;
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
    // cls();
    switch (set_mode)
    {
    case 0:
        day_mode();
        break;
    case 1:
        month_mode();
        break;
    case 2:
        year_mode();
        break;
    }

    clock_mode = old_mode;
}

void day_mode()
{
    get_time();
    byte set_scnd = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];

    cls();
    set_date = rtc[4];
    char buffer[5] = "    ";
    itoa(set_date, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_date < 31)
            {
                set_date++;
            }
            else
            {
                set_date = 1;
            }
            //print the new value
            itoa(set_date, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_date > 0)
            {
                set_date--;
            }
            else
            {
                set_date = 31;
            }
            //print the new value
            itoa(set_date, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));
    }
    fade_down();
}

void month_mode()
{
    get_time();
    byte set_scnd = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];

    cls();
    set_mnth = rtc[5];
    char buffer[5] = "    ";
    itoa(set_mnth, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_mnth < 12)
            {
                set_mnth++;
            }
            else
            {
                set_mnth = 1;
            }
            //print the new value
            itoa(set_mnth, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_mnth > 0)
            {
                set_mnth--;
            }
            else
            {
                set_mnth = 12;
            }
            //print the new value
            itoa(set_mnth, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_scnd));
    }

    fade_down();
}

void year_mode()
{
    get_time();
    byte set_secs = rtc[0];
    byte set_min = rtc[1];
    byte set_hr = rtc[2];
    byte set_date = rtc[4];
    byte set_mnth = rtc[5];
    int set_yr = rtc[6];
    
    cls();
    set_yr = rtc[6];
    char buffer[5] = "    ";
    itoa(set_yr, buffer, 10);
    tiny_font(0, 1, buffer[0]);
    tiny_font(4, 1, buffer[1]);
    tiny_font(8, 1, buffer[2]);
    tiny_font(12, 1, buffer[3]);
    while (!button_A.uniquePress())
    {
        while (button_D.isPressed())
        {
            if (set_yr < 2099)
            {
                set_yr++;
            }
            else
            {
                set_yr = 2000;
            }
            //print the new value
            itoa(set_yr, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        while (button_C.isPressed())
        {
            if (set_yr > 0)
            {
                set_yr--;
            }
            else
            {
                set_yr = 2000;
            }
            //print the new value
            itoa(set_yr, buffer, 10);
            tiny_font(0, 1, buffer[0]);
            tiny_font(4, 1, buffer[1]);
            tiny_font(8, 1, buffer[2]);
            tiny_font(12, 1, buffer[3]);
            delay(150);
        }
        ds1307.adjust(DateTime(set_yr, set_mnth, set_date, set_hr, set_min, set_secs));
    }

    fade_down();
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
