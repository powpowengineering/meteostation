#include <Arduino.h>

#include <OneWire.h> // ?????? ???????????? ?? ???????????????? ?????????????????????? DS18B20
#include <DallasTemperature.h> // ?????? ???????????? ?? ???????????????? ?????????????????????? DS18B20
#include <DHT.h> // ???????????????????? ?????? ???????????? ?? ?????????????????? DHT11, DHT21, DHT22
#include <DS3231.h> //????????
#include <Wire.h>
#include <SparkFun_LPS25HB_Arduino_Library.h>
#include <SD.h> //sd ??????????
#include <SPI.h> //sd ??????????
#include <avr/sleep.h>
#include "buildTime.h" // ?????? ???????????????? ???????????? ???????? ?? ??????????????, ???????????????????? ?????? ????????????????????
#include <Nokia_LCD.h> //nokia 5110 display
#include "Init.h"
#include <math.h>
#include <TimeLib.h>
#include <EEPROM.h>


//**************************************************************************************************
// @Function      anybuttonPressed()
//--------------------------------------------------------------------------------------------------
// @Description   Has any button pressed?
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   true  - button was pressed
//                false - button wasn't pressed
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
bool anybuttonPressed(void)
{
    int reading = analogRead(BUTTON_PIN);
    return (reading > 300) && (reading < 900);
}// end of anybuttonPressed()



//**************************************************************************************************
// @Function      whbuttonPressed()
//--------------------------------------------------------------------------------------------------
// @Description   What button was pressed?
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Number button
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
int whbuttonPressed(void)
{
    int reading = analogRead(BUTTON_PIN);
    if (reading > 400 && reading < 500) return 1;
     else {if (reading>500 && reading<600) return 5;
            else {if (reading > 600 && reading < 700) return 2;
                    else {if (reading > 800 && reading < 900) return 3;
                          else {if (reading >300 && reading < 400) return 4;
                                  else return 0;
                                  }
                          }
                  }
        }
}// end of whbuttonPressed()



//**************************************************************************************************
// @Function      write2sd()
//--------------------------------------------------------------------------------------------------
// @Description   Write data in two SD card
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void write2sd(void)
{
	static int reg = 0;
    static int reg2 = 0;
    static int count = 1;
    static int count2 = 1;
    static uint32_t lt_day=0;
    static uint32_t lt_day1=0;
    static String stringOne("d1.txt");
	static String stringOne2("d1.txt");
    uint32_t currentUnixTime;

  digitalWrite(PIN_CS_SD_CARD_1,HIGH);
  digitalWrite(PIN_CS_SD_CARD_2,HIGH);

	currentUnixTime = timeCurrent.unixtime();
    #ifdef DEBUG
    Serial.print("Initializing SD card #1...");
    #endif
    if (SD.begin(SPI_HALF_SPEED,PIN_CS_SD_CARD_1))
      {
        #ifdef DEBUG
            Serial.println("initialization done.");
        #endif

        if ((reg>=10) || ((currentUnixTime-lt_day)>(uint32_t)(SECONDS_IN_DAY)))
        {
            stringOne = "d"+String(count)+".txt"; //or .csv?
            count=count+1;
            reg = 0;
            lt_day = currentUnixTime;
        }
        reg=reg+1;

        myFile = SD.open(stringOne, FILE_WRITE);
        if (myFile)
        {
            cntWriteSD_1++;
            myFile.print(t1);
            myFile.print(" | ");
            myFile.print(t2);
            myFile.print(" | ");
            myFile.print(t3);
            myFile.print(" | ");
            myFile.print(pressurePascals);
            myFile.print(" | ");
            myFile.print(humidity);
            myFile.print(" | ");
            myFile.print(vbat);
            myFile.print(" | ");
            myFile.print(timeCurrent.day());
            myFile.print(".");
            myFile.print(timeCurrent.month());
            myFile.print(".");
            myFile.print(timeCurrent.year());
            myFile.print("  ");
            myFile.print(timeCurrent.hour());
            myFile.print(":");
            myFile.print(timeCurrent.minute());
            myFile.print(":");
            myFile.println(timeCurrent.second());
            myFile.close();
        }
        else
        {
          #ifdef DEBUG
          Serial.println("Could not open file on sd 1.");
		  Serial.println(stringOne);
          #endif
        }
    }
    else
    {
      #ifdef DEBUG
      Serial.println("initialization failed SD card #1!");
      Serial.print("error code = ");
      Serial.println(SD.card.errorCode());
      #endif
    }
    SD.end();

    #ifdef DEBUG
    Serial.print("Initializing SD card #2...");
    #endif

    if ((reg2>=10) || ((currentUnixTime-lt_day1)>(uint32_t)(SECONDS_IN_DAY)))
    {
        stringOne2 = "d"+String(count2)+".txt"; //or .csv?
        count2=count2+1;
        reg2 = 0;
        lt_day1 = currentUnixTime;
    }
    reg2=reg2+1;

    if (SD.begin(SPI_HALF_SPEED,PIN_CS_SD_CARD_2))
      {
        #ifdef DEBUG
          Serial.println("initialization done.");
        #endif

         myFile = SD.open(stringOne2, FILE_WRITE);
        if (myFile)
        {
            cntWriteSD_2++;
            myFile.print(t1);
            myFile.print(" | ");
            myFile.print(t2);
            myFile.print(" | ");
            myFile.print(t3);
            myFile.print(" | ");
            myFile.print(pressurePascals);
            myFile.print(" | ");
            myFile.print(humidity);
            myFile.print(" | ");
            myFile.print(vbat);
            myFile.print(" | ");
            myFile.print(timeCurrent.day());
            myFile.print(".");
            myFile.print(timeCurrent.month());
            myFile.print(".");
            myFile.print(timeCurrent.year());
            myFile.print("  ");
            myFile.print(timeCurrent.hour());
            myFile.print(":");
            myFile.print(timeCurrent.minute());
            myFile.print(":");
            myFile.println(timeCurrent.second());
            myFile.close();
        }
        else
        {
          #ifdef DEBUG
          Serial.println("Could not open file on sd 2");
		  Serial.println(stringOne2);
          #endif
        }
    }
    else
    {
      #ifdef DEBUG
      Serial.println("initialization failed SD card #2!");
      Serial.print("error code = ");
      Serial.println(SD.card.errorCode());
      #endif
    }
    SD.end();
}// end of write2sd()



//**************************************************************************************************
// @Function      LCDShow()
//--------------------------------------------------------------------------------------------------
// @Description   Show menu on LCD
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
 void LCDShow(void)
{
  static MENU_SCREEN menuLCD = MAIN_MENU;
/*------------------------------ MAIN menu ----------------------------*/
    if (MAIN_MENU == menuLCD)
    {
        if (BUTTON_DOWN == buttonNum)// down
        {
            if ((NUMBER_SHOW_PARAM-1) != cursorPos)
            {
                if ((firstRowPos+(NUMBER_ROWS_SCREEN-1)) == cursorPos)
                {
                    firstRowPos +=1;
                }
                cursorPos +=1;
            }
        }
        else if (BUTTON_UP == buttonNum)// up
           {
                if (0 != cursorPos)
                {
                    if(firstRowPos == cursorPos)
                    {
                        firstRowPos -=1;
                    }
                    cursorPos -=1;
                }
            }
        else if (BUTTON_SELECT == buttonNum)// select
            {
				lcd.setInverted(false);// ?????????? ???????????? ???????????????? ???????? ???????????? ??????????????????
                if (SCREEN_DATE_POS == cursorPos)
                {
                    menuLCD = DATE_MENU;
                    menuDate.state = DAY;

                }
                else if (SCREEN_TIME_POS == cursorPos)
                {
                    menuLCD = TIME_MENU;
					menuTime.state = HOUR;

                }
                else if (SCREEN_ALARM_POS == cursorPos)
                {
                    menuLCD = ALARM_MENU;
                     //menuAlarm.state = SCALE; - commented because it was same in pasha's file
                    //printCurrentMenuOnLCD(menuLCD);
                }
				else if ((SCREEN_TEMP1_POS == cursorPos)||(SCREEN_TEMP2_POS == cursorPos)||(SCREEN_TEMP3_POS == cursorPos)\
						||(SCREEN_HUM_POS == cursorPos)||(SCREEN_PRES_POS == cursorPos)|| (SCREEN_VBAT_POS == cursorPos))
				{
					ReadSensors();
				}
            }
		printCurrentMenuOnLCD(menuLCD);
    }
/*------------------------------ DATE menu ----------------------------*/
    else if (DATE_MENU == menuLCD)
    {
        if (BUTTON_DOWN == buttonNum)// down
        {
            if (DAY == menuDate.state)
            {
                if (menuDate.date.day > 1)
                {
                    menuDate.date.day--;
                }
            }
            else if (MONTH == menuDate.state)
            {
                if (menuDate.date.month > 1)
                {
                    menuDate.date.month--;
                }
            }
            else if (YEAR == menuDate.state)
            {
                if (menuDate.date.year > 0)
                {
                    menuDate.date.year--;
                }
            }
        }
        else if (BUTTON_UP == buttonNum)// up
        {
            if (DAY == menuDate.state)
            {
				menuDate.date.day++;
            }
            else if (MONTH == menuDate.state)
            {
                if (menuDate.date.month < 12)
                {
                    menuDate.date.month++;

                }
            }
            else if (YEAR == menuDate.state)
            {
                if (menuDate.date.year < 99)
                {
                    menuDate.date.year++;
                }
            }
        }
        else if (BUTTON_LEFT == buttonNum)// left
        {
            if (DAY == menuDate.state)
            {
               ;
            }
            else if (MONTH == menuDate.state)
            {
                menuDate.state = DAY;
            }
            else if (YEAR == menuDate.state)
            {
                menuDate.state = MONTH;
            }
        }
        else if (BUTTON_RIGHT == buttonNum)// right
        {
            if (DAY == menuDate.state)
            {
                menuDate.state = MONTH;
            }
            else if (MONTH == menuDate.state)
            {
                menuDate.state = YEAR;
            }
            else if (YEAR == menuDate.state)
            {
                ;
            }
        }
        else if (BUTTON_SELECT == buttonNum)// select
        {
            SetDate(menuDate.date.day, menuDate.date.month, menuDate.date.year);
            timeCurrent = RTClib::now();  // ???????????? ???????????????? ??????????????
            menuLCD = MAIN_MENU;
        }

		if(((menuDate.date.month == 1) || (menuDate.date.month == 3) || (menuDate.date.month == 5) || (menuDate.date.month == 7)\
			|| (menuDate.date.month == 8) || (menuDate.date.month == 10) || (menuDate.date.month == 12)) && (menuDate.date.day > 31))
		{
			menuDate.date.day = 31;
		}
		else if(((menuDate.date.month == 4) || (menuDate.date.month == 6) || (menuDate.date.month == 9) || (menuDate.date.month == 11))
                    && (menuDate.date.day > 30))
		{
			menuDate.date.day = 30;
		}
		else if ((menuDate.date.month == 2) && (menuDate.date.day > 28))
		{
			menuDate.date.day = 28;
		}


        printCurrentMenuOnLCD(menuLCD);
    }
/*------------------------------ TIME menu ----------------------------*/
    else if (TIME_MENU == menuLCD)
    {
        if (BUTTON_DOWN == buttonNum)// down
        {
            if (HOUR == menuTime.state)
            {
                if (menuTime.time.hour == 0)
                {
                    menuTime.time.hour = 23;
                }
                else
                {
                    menuTime.time.hour--;
                }
            }
            else if (MINUTE == menuTime.state)
            {
                if (menuTime.time.minute == 0)
                {
                    menuTime.time.minute = 59;
                }
                else
                {
                    menuTime.time.minute--;
                }
            }
            else if (SECOND == menuTime.state)
            {
                if (menuTime.time.second == 0)
                {
                    menuTime.time.second = 59;
                }
                else
                {
                    menuTime.time.second--;
                }
            }
        }
        else if (BUTTON_UP == buttonNum)// up
        {
            if (HOUR == menuTime.state)
            {
                if (menuTime.time.hour == 23)
                {
                    menuTime.time.hour = 0;
                }
                else
                {
                    menuTime.time.hour++;
                }
            }
            else if (MINUTE == menuTime.state)
            {
                if (menuTime.time.minute == 59)
                {
                    menuTime.time.minute = 0;
                }
                else
                {
                    menuTime.time.minute++;
                }
            }
            else if (SECOND == menuTime.state)
            {
                if (menuTime.time.second == 59)
                {
                    menuTime.time.second = 0;
                }
                else
                {
                    menuTime.time.second++;
                }
            }
        }
        else if(BUTTON_LEFT == buttonNum)// left
        {
            if (HOUR == menuTime.state)
            {
               ;
            }
            else if (MINUTE == menuTime.state)
            {
                menuTime.state = HOUR;
            }
            else if (SECOND == menuTime.state)
            {
                menuTime.state = MINUTE;
            }
        }
        else if(BUTTON_RIGHT == buttonNum)// right
        {
            if(HOUR == menuTime.state)
            {
                menuTime.state = MINUTE;
            }
            else if (MINUTE == menuTime.state)
            {
                menuTime.state = SECOND;
            }
            else if (SECOND == menuTime.state)
            {
                ;
            }
        }
        else if (BUTTON_SELECT == buttonNum)// select
        {
            SetTime(menuTime.time.hour, menuTime.time.minute, menuTime.time.second);
            timeCurrent = RTClib::now();  // ???????????? ???????????????? ??????????????
            menuLCD = MAIN_MENU;
        }
        printCurrentMenuOnLCD(menuLCD);
    }
    /*--------------------------- ALARM menu ------------------------*/
    else if (ALARM_MENU == menuLCD)
    {
	if (BUTTON_DOWN == buttonNum)// down
      {
          if (SCALE == menuAlarm.state)
          {
            if (menuAlarm.alarm.scale == SEC)
            {
              menuAlarm.alarm.scale = MIN;
              menuAlarm.alarm.period = 10;
            }
            else if (menuAlarm.alarm.scale == MIN)
                {
                  menuAlarm.alarm.scale = HOURS;
                  menuAlarm.alarm.period = 1;
                }
                else
                {
                  menuAlarm.alarm.scale = HOURS;
                }
          }
          else if (PERIOD == menuAlarm.state)
          {
              if (menuAlarm.alarm.scale == SEC)
              {
                if (menuAlarm.alarm.period > 10)
                {
                  menuAlarm.alarm.period--;
                }
                else {menuAlarm.alarm.period=10;}
              }
              if (menuAlarm.alarm.scale == MIN)
              {
                if (menuAlarm.alarm.period > 1)
                {
                  menuAlarm.alarm.period--;
                }
                else {menuAlarm.alarm.period=1;}

              }
              else if (menuAlarm.alarm.period > 1)
                  {menuAlarm.alarm.period--;}
                  else {menuAlarm.alarm.period = 1;}
          }
      }
      else if (BUTTON_UP == buttonNum)// up
      {
          if (SCALE == menuAlarm.state)
          {
            if (menuAlarm.alarm.scale == HOURS)
            {
              menuAlarm.alarm.scale = MIN;
              menuAlarm.alarm.period = 10;
            }
            else if (menuAlarm.alarm.scale == MIN)
            {
              menuAlarm.alarm.scale = SEC;
              menuAlarm.alarm.period = 10;
            } else {menuAlarm.alarm.scale = SEC;}
          }
          else if (PERIOD == menuAlarm.state)
          {
              if ((menuAlarm.alarm.scale == SEC || menuAlarm.alarm.scale == MIN))
              {
                if (menuAlarm.alarm.period<60)
                {
                  menuAlarm.alarm.period++;
                }
                else {menuAlarm.alarm.period = 10;}
              }
              else if (menuAlarm.alarm.period<4)
                  {
                    menuAlarm.alarm.period++;
                  }
                  else {menuAlarm.alarm.period = 4;}

          }
  }
      else if (BUTTON_LEFT == buttonNum)// left
      {
          if (SCALE == menuAlarm.state)
          {
             ;
          }
          else if (PERIOD == menuAlarm.state)
          {
              menuAlarm.state = SCALE;
          }
      }
      else if (BUTTON_RIGHT == buttonNum)// right
      {
          if (SCALE == menuAlarm.state)
          {
              menuAlarm.state = PERIOD;
          }
          else if (PERIOD == menuAlarm.state)
          {
             ;
          }
      }
      else if (BUTTON_SELECT == buttonNum)// select
      {
         Serial.println("test");
         Serial.print("state in func lcd show menu alarm button select ");
         Serial.println(menuAlarm.state);
         Serial.print("period in func lcd show menu alram button select ");
         Serial.println(menuAlarm.alarm.period);
         Serial.print("period from alarm.period in func lacd show menu alarm buton selct");
         Serial.println(menuAlarm.alarm.scale);
        // Serial.println(alarm.period);
         SetAlarm(menuAlarm.alarm.scale, menuAlarm.alarm.period);
         menuLCD = MAIN_MENU;
      }
      printCurrentMenuOnLCD(menuLCD);
    }
}//end of LCDShow()



//**************************************************************************************************
// @Function      makeStringsForLCD()
//--------------------------------------------------------------------------------------------------
// @Description   Make strings for LCD's rows
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters   date,time,alarm -> parameters, which we need to show
//**************************************************************************************************
void makeStringsForLCD(DATE *date, TIME *time, ALARM *alarm)
{
    char str_temp[10];
	char str_scale[6];
	dtostrf(t1, 3, 1, str_temp);
	snprintf(screenValue[0],LCD_NUM_SYMBOL_IN_ROW,"t1    %5s^C",str_temp);
	dtostrf(t2, 3, 1, str_temp);
	snprintf(screenValue[1],LCD_NUM_SYMBOL_IN_ROW,"t2    %5s^C",str_temp);
	dtostrf(t3, 3, 1, str_temp);
	snprintf(screenValue[2],LCD_NUM_SYMBOL_IN_ROW,"t3    %5s^C",str_temp);
	dtostrf(humidity, 3, 1, str_temp);
	snprintf(screenValue[3],LCD_NUM_SYMBOL_IN_ROW,"Hum    %5s%%",str_temp);
	dtostrf(pressurePascals, 6, 1, str_temp);
	snprintf(screenValue[4],LCD_NUM_SYMBOL_IN_ROW,"P  %s hPa",str_temp);
    dtostrf(vbat, 4, 2, str_temp);
    snprintf(screenValue[5],LCD_NUM_SYMBOL_IN_ROW,"Vbat   %s V",str_temp);
	snprintf(screenValue[6],LCD_NUM_SYMBOL_IN_ROW,"Date %2d.%2d.%2d",date->day,date->month,date->year);
	snprintf(screenValue[7],LCD_NUM_SYMBOL_IN_ROW,"Time %2d:%2d:%2d",time->hour,time->minute,time->second);
	if (alarm->scale == HOURS)
    {
        snprintf(screenValue[8],LCD_NUM_SYMBOL_IN_ROW,"Freq HOURS %d",alarm->period);
    }
    else if (alarm->scale == MIN)
    {
        snprintf(screenValue[8],LCD_NUM_SYMBOL_IN_ROW,"Freq MIN   %d",alarm->period);
    }
    else
	{
		snprintf(screenValue[8],LCD_NUM_SYMBOL_IN_ROW,"Freq SEC   %d",alarm->period);
	}

	snprintf(screenValue[9],LCD_NUM_SYMBOL_IN_ROW,"Cnt_1  %u",cntWriteSD_1);
    snprintf(screenValue[10],LCD_NUM_SYMBOL_IN_ROW,"Cnt_2  %u",cntWriteSD_2);

}// end of makeStringsForLCD()

//**************************************************************************************************
// @Function      ReadSensors()
//--------------------------------------------------------------------------------------------------
// @Description   read all sensors
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void ReadSensors(void)
{
    timeCurrent = RTClib::now();  // ???????????? ???????????????? ??????????????
    pressurePascals = barometer.getPressure_hPa();
    humidity = humidity_sensor.readHumidity();
    vbat = ReadVbat();

	// ???????? ???????????? t1 ?????????????????????? ?????????????????? ??????????????????????
	if (true == sensors.requestTemperaturesByAddress(t1_deviceAddress))
	{
	  t1 = sensors.getTempC(t1_deviceAddress);
	}
	else
	{
	  t1 = NAN;
	}
    // ???????? ???????????? t2 ?????????????????????? ?????????????????? ??????????????????????
	if (true == sensors.requestTemperaturesByAddress(t2_deviceAddress))
	{
	  t2 = sensors.getTempC(t2_deviceAddress);
	}
	else
	{
	  t2 = NAN;
	}
    // ???????? ???????????? t3 ?????????????????????? ?????????????????? ??????????????????????
	if (true == sensors.requestTemperaturesByAddress(t3_deviceAddress))
	{
	  t3 = sensors.getTempC(t3_deviceAddress);
	}
	else
	{
	  t3 = NAN;
	}

	/**************************print debug*******************************/

    #ifdef DEBUG
    Serial.print(timeCurrent.day());
    Serial.print(".");
	Serial.print(timeCurrent.month());
    Serial.print(".");
    Serial.print(timeCurrent.year());
    Serial.print("  ");
    Serial.print(timeCurrent.hour());
    Serial.print(":");
    Serial.print(timeCurrent.minute());
    Serial.print(":");
    Serial.print(timeCurrent.second());

	  Serial.print(" ");
    Serial.print("Sensor t1:");
    Serial.print(t1);
    Serial.print((char)176);//shows degrees character
    Serial.print("C  |  ");

    Serial.print("Sensor t2:");
    Serial.print(t2);
    Serial.print((char)176);//shows degrees character
    Serial.print("C  |  ");

    Serial.print("Sensor t3:");
    Serial.print(t3);
    Serial.print((char)176);//shows degrees character
    Serial.print("C  |  ");

    Serial.print("Pressure_P: ");
    Serial.print(pressurePascals);
    Serial.println("Pa  ");


    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%  ");

    #endif

}// end of ReadSensors()


//**************************************************************************************************
// @Function      printCurrentMenuOnLCD()
//--------------------------------------------------------------------------------------------------
// @Description   print cuurent menu on LCD
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    menuLCD - cuurent menu
//**************************************************************************************************
void printCurrentMenuOnLCD(MENU_SCREEN menuLCD)
{
    DATE date;
    TIME time;
    ALARM alarm;

    if ( MAIN_MENU == menuLCD )
    {
        menuDate.date.day = timeCurrent.day();
        menuDate.date.month = timeCurrent.month();
        menuDate.date.year = timeCurrent.year()-2000;
		menuTime.time.hour = timeCurrent.hour();
        menuTime.time.minute = timeCurrent.minute();
        menuTime.time.second = timeCurrent.second();

		date.day = timeCurrent.day();
        date.month = timeCurrent.month();
        date.year = timeCurrent.year()-2000;
        time.hour = timeCurrent.hour();
        time.minute = timeCurrent.minute();
        time.second = timeCurrent.second();

        alarm.scale = menuAlarm.alarm.scale;
        alarm.period = menuAlarm.alarm.period;

        makeStringsForLCD(&date,&time,&alarm);

        lcd.clear();
        for (byte i=0;i< NUMBER_ROWS_SCREEN;i++)
        {
           lcd.setCursor(0,i);
           if (i == (cursorPos-firstRowPos))
           {
                lcd.setInverted(true);
           }
           else
           {
               lcd.setInverted(false);
           }
           lcd.print(screenValue[firstRowPos + i]);
        }
    }
    else if ( DATE_MENU == menuLCD )
    {
        date.day = menuDate.date.day;
        date.month = menuDate.date.month;
        date.year = menuDate.date.year;
        time.hour = timeCurrent.hour();
        time.minute = timeCurrent.minute();
        time.second = timeCurrent.second();

        alarm.scale = menuAlarm.alarm.scale;
        alarm.period = menuAlarm.alarm.period;

        makeStringsForLCD(&date,&time,&alarm);

        lcd.clear();
        for (byte i=0;i< NUMBER_ROWS_SCREEN;i++)
        {
            lcd.setCursor(0,i);
            if (i == (SCREEN_DATE_POS-firstRowPos))
			{
				if ( DAY == menuDate.state )
				{
					lcd.print(screenValue[firstRowPos + i], 5, 2);

				}
				else if ( MONTH == menuDate.state )
				{
					lcd.print(screenValue[firstRowPos + i], 8, 2);
				}
				else if ( YEAR == menuDate.state )
				{
					lcd.print(screenValue[firstRowPos + i], 11, 2);
				}

			}
			else
			{
				lcd.print(screenValue[firstRowPos + i]);
			}

        }
    }
    else if ( TIME_MENU == menuLCD )
    {
		date.day = timeCurrent.day();
        date.month = timeCurrent.month();
        date.year = timeCurrent.year()-2000;
        time.hour = menuTime.time.hour;
        time.minute = menuTime.time.minute;
        time.second = menuTime.time.second;

        alarm.scale = menuAlarm.alarm.scale;
        alarm.period = menuAlarm.alarm.period;

        makeStringsForLCD(&date,&time,&alarm);

		lcd.clear();
        for (byte i=0;i< NUMBER_ROWS_SCREEN;i++)
        {
            lcd.setCursor(0,i);
            if (i == (SCREEN_TIME_POS-firstRowPos))
			{
				if ( HOUR == menuTime.state )
				{
					lcd.print(screenValue[firstRowPos + i], 5, 2);
				}
				else if ( MINUTE == menuTime.state )
				{
					lcd.print(screenValue[firstRowPos + i], 8, 2);
				}
				else if ( SECOND == menuTime.state )
				{
					lcd.print(screenValue[firstRowPos + i], 11, 2);
				}

			}
			else
			{
				lcd.print(screenValue[firstRowPos + i]);
			}
        }
    }
    else if ( ALARM_MENU == menuLCD )
    {
		date.day = timeCurrent.day();
        date.month = timeCurrent.month();
        date.year = timeCurrent.year()-2000;
        time.hour = timeCurrent.hour();
        time.minute = timeCurrent.minute();
        time.second = timeCurrent.second();
        alarm.scale = menuAlarm.alarm.scale;
        alarm.period = menuAlarm.alarm.period;
        makeStringsForLCD(&date,&time,&alarm);


        lcd.clear();
        for (byte i=0;i< NUMBER_ROWS_SCREEN;i++)
        {
            lcd.setCursor(0,i);
            if (i == (SCREEN_ALARM_POS-firstRowPos))
            {

              if ( SCALE == menuAlarm.state )
              {
                lcd.print(screenValue[firstRowPos + i], 5, 5);
              }
              else if ( PERIOD == menuAlarm.state )
                  {
                    lcd.print(screenValue[firstRowPos + i], 11, 2);
                  }

    }
	else
            {
              lcd.print(screenValue[firstRowPos + i]);
            }
        }
    }

}



//**************************************************************************************************
// @Function      SetDate()
//--------------------------------------------------------------------------------------------------
// @Description   Set date in RTC
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    d -> day,
//                m -> month
//                yOff -> year, only last two digits
//**************************************************************************************************
void SetDate(uint8_t  d , uint8_t  m, uint8_t yOff )
{
    rtc.setDate(d);     // ?????????????????????????? ??????????
    rtc.setMonth(m);   // ?????????????????????????? ??????????
    rtc.setYear(yOff);    // ?????????????????????????? ??????
}// end of SetDate()



//**************************************************************************************************
// @Function      SetTime()
//--------------------------------------------------------------------------------------------------
// @Description   Set time in RTC
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    h -> hour,
//                m -> minute
//                s -> second
//**************************************************************************************************
void SetTime(uint8_t  h, uint8_t  m, uint8_t s)
{
    rtc.setSecond(s);        // ?????????????????????????? ??????????????
    rtc.setMinute(m);        // ?????????????????????????? ????????????
    rtc.setHour(h);          // ?????????????????????????? ????????
}// end of SetTime()

//**************************************************************************************************
// @Function      Voltage()
//--------------------------------------------------------------------------------------------------
// @Description   Measure voltage
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   voltage.
//--------------------------------------------------------------------------------------------------
// @Parameters    sum->sum of samples taken
//                sample count->no samples
//                r1=30kom
//                r2=10kom
//                calibration value = 6.17/6.2=0.99516. measured 6 showed 6.24 on voltimeter         \
//                NUM_SAMPLES = 10 NO measures
//**************************************************************************************************
float ReadVbat(void)
{
  float voltage;
  float sum = 0;
  unsigned char sample_count = 0; // current sample number

      while (sample_count < NUM_SAMPLES) {
          sum += analogRead(VBAT_PIN);
          sample_count++;
      }
      // use 3.3 for a 3.3V ADC reference voltage
      voltage = 0.99516*(sum/NUM_SAMPLES)* (3.3/1024.0)/(r1/(r1+r2));
      // send voltage for display o n Serial Monitor
      // divides by 6.24/6 is the calibrated voltage divide

      return voltage;

      sample_count = 0;
      sum = 0;
}//end of ReadVbat

//**************************************************************************************************
// @Function      SetAlarm()
//--------------------------------------------------------------------------------------------------
// @Description   Set alarm
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    s -> scale (hour/minute/second)
//                p -> period (1-4 hours/ 1-60 minutes or seconds)
//                t -> current time
//**************************************************************************************************
void SetAlarm(SCALE_enum  s, uint8_t  p)
{
    DateTime timeCurrent;
    char buf[20];
    timeCurrent = RTClib::now();
    uint32_t nextTimeAlarm;
    int al_days;
    int al_hours;
    int al_minutes;
    int al_seconds;
    uint32_t unix_time;

    unix_time = timeCurrent.unixtime();

    if (s == SEC) //seconds
    {
      nextTimeAlarm = unix_time + p;
    }
    else if (s == MIN)
          {
            nextTimeAlarm = unix_time + p*60;
          }
          else {nextTimeAlarm = unix_time+p*3600;}

    al_days = day(nextTimeAlarm);
    al_hours = hour(nextTimeAlarm);
    al_minutes = minute(nextTimeAlarm);
    al_seconds = second(nextTimeAlarm);


    /*sprintf(buf, "%02d:%02d:%02d %02d/%02d/%02d",  timeCurrent.hour(), timeCurrent.minute(), timeCurrent.second(), timeCurrent.day(), timeCurrent.month(), timeCurrent.year());
    Serial.println(buf);
    Serial.println(timeCurrent.unixtime());
    Serial.println(nextTimeAlarm);
*/
    //setA1Time(byte A1Day, byte A1Hour, byte A1Minute, byte A1Second, byte AlarmBits, bool A1Dy, bool A1h12, bool A1PM)
    Serial.println(unix_time);
    Serial.println(nextTimeAlarm);
    Serial.println(al_days);
    Serial.println(al_hours);
    Serial.println(al_minutes);
    rtc.setA1Time(al_days,al_hours,al_minutes,al_seconds,0x0, false, false, false);
}// end of SetAlarm()
