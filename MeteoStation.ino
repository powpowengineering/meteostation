/********************************************************************/
#include <OneWire.h> // для работы с датчиком температуры DS18B20
#include <DallasTemperature.h> // для работы с датчиком температуры DS18B20
#include <DHT.h> // библиотека для работы с датчиками DHT11, DHT21, DHT22
#include <DS3231.h> //часы
#include <Wire.h>
#include <SparkFun_LPS25HB_Arduino_Library.h>//barometer
#include <SD.h> //sd карта
#include <SPI.h> //sd карта
#include <avr/sleep.h>
#include "buildTime.h" // для парсинга строки даты и времени, полученной при компиляции
#include <Nokia_LCD.h> //nokia 5110 display
#include "ModuleRAK811.h"
#include "Init.h"
#include <math.h>
#include <TimeLib.h>
#include <EEPROM.h>

/********************************************************************/

DATE_MENU_SCREEN menuDate;
TIME_MENU_SCREEN menuTime;
ALARM_MENU_SCREEN menuAlarm;
ALARM_MENU_SCREEN menuAlarmGSM;

bool alarmTime = false;
bool alarmTimeGSM = false;
byte buttonNum = 0;     // какая кнопка нажата
bool pressAnyButton = false; // была ли нажата кнопка любая
char screenValue[NUMBER_SHOW_PARAM][LCD_NUM_SYMBOL_IN_ROW];
uint8_t firstRowPos = 0;
uint8_t cursorPos = 0;
int timeDelay = 0;
int timeDelayOld = 0;
float r1 = 11;
float r2 = 30;
float vbat = 0.0;            // calculated voltage



/********************************************************************/

const uint8_t t_deviceAddress[5][8] =
{
  { 0x28, 0xdb, 0x04, 0x75, 0xd0, 0x01, 0x3c, 0xd4 },//0
  { 0x28, 0x4b, 0xeb, 0x07, 0xb6, 0x01, 0x3c, 0x0b },//1
  { 0x28, 0x48, 0xdf, 0x07, 0xb6, 0x01, 0x3c, 0xc9 },//2
  { 0x28, 0x87, 0x13, 0x7f, 0x12, 0x21, 0x01, 0x2f },//3
  { 0x28, 0x17, 0x62, 0x80, 0x12, 0x21, 0x01, 0xd5 } //4
};

const uint8_t* t1_deviceAddress = t_deviceAddress[4];
const uint8_t* t2_deviceAddress = t_deviceAddress[2];
const uint8_t* t3_deviceAddress = t_deviceAddress[3];

//button addresses analogread()
const int buttonDOWN = 372; //shitty
const int buttonEnter = 420;
const int buttonLeft = 520;
const int buttonUP = 630;    //also shitty sometimes
const int buttonRight = 850;

float pressurePascals = 0;// Создаём переменную для значения атмосферного давления в Паскалях
float humidity = 0;// переменная показания влажности
float t1 = 0; //температура первого датчика
float t2 = 0; //температура второго датчика
float t3 = 0; //температура третьего датчика
float Vbat = 0; // напряжение батареи
uint32_t cntWriteSD_1 = 0;//счетчик записей на sd 1
uint32_t cntWriteSD_2 = 0;//счетчик записей на sd 2
DateTime timeCurrent; // текущее время
DateTime timeOld;

DS3231  rtc;
File myFile;
OneWire oneWire(ONE_WIRE_BUS); //вход датчика DS18B20
DallasTemperature sensors(&oneWire);
DHT humidity_sensor(PIN_AM2305, DHT22); //датчик влажности am2305
LPS25HB barometer;
Nokia_LCD lcd(PIN_CLK_LCD /* CLK */, PIN_CLK_DIN /* DIN */, PIN_CLK_DC /* DC */, PIN_CLK_CE /* CE */, PIN_CLK_RST /* RST */); //nokia lcd pins



void setup()
{
    //#ifdef DEBUG
    Serial.begin(9600); // открываем последовательный порт для мониторинга действий в программе
    //#endif
    Wire.begin();// Start the I2C interface

    lcd.begin(); //initialize screen
    lcd.setBacklight(true);
    lcd.setContrast(60);
    lcd.clear(true);
    delay(500);
    lcd.clear();
    lcd.setBacklight(false);
    lcd.setCursor(0, 5);

    pinMode(PIN_INT_ALARM, INPUT);// пин для внешнего прерывания от RTC
    pinMode(PIN_INT_BUTTON, INPUT);// пин для внешнего прерывания от button
    pinMode(PIN_CS_SD_CARD_1, OUTPUT);
    pinMode(PIN_CS_SD_CARD_2, OUTPUT);
    pinMode(53, OUTPUT);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // настройка режима сна

    sensors.begin();//датчики температуры
    sensors.setResolution(t1_deviceAddress, 12, true);
    sensors.setResolution(t2_deviceAddress, 12, true);
    sensors.setResolution(t3_deviceAddress, 12, true);

    barometer.begin(Wire, LPS25HB_I2C_ADDR_ALT);

    humidity_sensor.begin();

    RAK811_setState( SLEEP );

    rtc.setSecond(BUILD_SEC);    // устанавливаем секунд
    rtc.setMinute(BUILD_MIN);    // установка минут
    rtc.setHour(BUILD_HOUR);     //установка часов
    rtc.setDate(BUILD_DAY);     // устанавливаем число
    rtc.setMonth(BUILD_MONTH);   // Устанавливаем месяц
    rtc.setYear(BUILD_YEAR - 2000);  // Устанавливаем год
    rtc.setClockMode(false);    // установка режима 12/24h. True is 12-h, false is 24-hour.

    timeCurrent = RTClib::now();  // чтение текущего времени
    menuDate.date.day = timeCurrent.day();
    menuDate.date.month = timeCurrent.month();
    menuDate.date.year =  timeCurrent.year() - 2000;

    menuAlarm.alarm.scale = MIN;
    menuAlarm.alarm.period = 15;

    menuAlarmGSM.alarm.scale = HOURS;
    menuAlarmGSM.alarm.period = 3;   

	rtc.turnOffAlarm(ALARM_1);
	rtc.turnOffAlarm(ALARM_2);

    attachInterrupt(INT_ALARM, isrAlarm, FALLING);  // прерывание от RTC
    attachInterrupt(INT_BUTTON, isrButtonPressed, FALLING); // прерывание от button
    
    ReadSensors();
    pressAnyButton = true;

}

void loop()
{
    if (true == pressAnyButton)
    {
        timeCurrent = RTClib::now();  // чтение текущего времени
        LCDShow();
        pressAnyButton = false;
        timeOld = timeCurrent;
        timeDelay = millis();
        timeDelayOld = timeDelay;
    }

    if (true == alarmTime)
    {
        ReadSensors();
        write2sd();
        alarmTime = false;
    }

    if (true == alarmTimeGSM)
    {
        char buf[256];
        char str_temp_t1[10];
        char str_temp_Hum[10];
        char str_temp_P[10];
        char str_temp_Vbat[10];
        
		ReadSensors();
		
        dtostrf(t1, 3, 1, str_temp_t1);
        dtostrf(humidity, 3, 1, str_temp_Hum);
        dtostrf(pressurePascals, 6, 1, str_temp_P);
        dtostrf(vbat, 4, 2, str_temp_Vbat);
        snprintf(buf, 256, "%2d.%2d.%2d|%2d:%2d:%2d|t1=%5s|Hum=%5s|P=%6s|Vbat=%4s|Cnt_1=%u|Cnt_2=%u", menuDate.date.day, menuDate.date.month, menuDate.date.year, \
                 menuTime.time.hour, menuTime.time.minute, menuTime.time.second, str_temp_t1, str_temp_Hum, str_temp_P, str_temp_Vbat, cntWriteSD_1, cntWriteSD_2);
        
        RAK811_setState( WAKE_UP );
        delay(2000);
        RAK811_init();
        RAK811_confMode(RAK811_MODE_LORA_P2P);
        delay(500);
        RAK811_confP2Pprm("869525000", 12, 0, 1, 8, 20);
        delay(500);
        RAK811_confTransferMode(RAK811_SENDER_MODE);
        delay(500);        
        RAK811_sendData(buf);
        delay(3000);
        RAK811_setState( SLEEP );
        alarmTimeGSM = false;
    }


    if ((timeCurrent.unixtime() - timeOld.unixtime()) > TIME_SCREEN_ON)
    {      
        attachInterrupt(INT_ALARM, isrAlarm, FALLING); // прерывание от RTC
        attachInterrupt(INT_BUTTON, isrButtonPressed, FALLING); // прерывание от button
        
        if (false == rtc.checkAlarmEnabled(ALARM_1))
        {
            Serial.println("\r\nSet ALARM_1");
			SetAlarm(menuAlarm.alarm.scale, menuAlarm.alarm.period);
            rtc.clearFlagAlarm(ALARM_1);
            rtc.turnOnAlarm(ALARM_1);
        }     
        if (false == rtc.checkAlarmEnabled(ALARM_2))
        {
            Serial.println("\r\nSet ALARM_2");
			SetAlarm2(menuAlarmGSM.alarm.scale, menuAlarmGSM.alarm.period);
            rtc.clearFlagAlarm(ALARM_2);
            rtc.turnOnAlarm(ALARM_2);
        }
        
       
	    if (!((true == rtc.checkIfAlarm(ALARM_1)) || (true == rtc.checkIfAlarm(ALARM_2))))
		{
			lcd.clear();
			lcd.setInverted(false);
			lcd.setCursor(0, 2);
			lcd.print("Sleep");
			
			sleep_mode(); // Переводим МК в сон
			if (pressAnyButton != true)
			{
				delay(3000);
			}
			lcd.clear();
        }
		
        if((true == rtc.checkIfAlarm(ALARM_1)) || (true == rtc.checkIfAlarm(ALARM_2)))
        {
            if(true == rtc.checkIfAlarm(ALARM_1))
            {
                rtc.turnOffAlarm(ALARM_1);
                rtc.clearFlagAlarm(ALARM_1);
                alarmTime = true;
                lcd.println("Writing to SD");
                Serial.println("\r\nisr Writing to SD");
            }
            if(true == rtc.checkIfAlarm(ALARM_2))
            {
                rtc.turnOffAlarm(ALARM_2);
                rtc.clearFlagAlarm(ALARM_2);
                alarmTimeGSM = true;
                lcd.print("Sending to    Lora");
                Serial.println("\r\nisr Sending to Lora");
            }
        }
        else
        {
            lcd.print("Wakeup");    
        }
        
    }
    else
    {
        // читаем время RTC раз в секунду
        timeDelay = millis();
        if ((timeDelay - timeDelayOld) > 1000)
        {
            timeCurrent = RTClib::now();  // чтение текущего времени
            timeDelayOld = timeDelay;

        }
    }


}



/********************************обработчик аппаратного прерывания********************/
void isrAlarm()
{
    Serial.println("\r\nisr RTC");
}




/********************************обработчик прерывания по кнопке********************/
void isrButtonPressed()
{
  Serial.println("\r\nisr Button");

  ADCSRA |= (1 << ADEN);
  buttonNum = whbuttonPressed();
  if (0 != buttonNum)
  {
    pressAnyButton = true;
  }
  else
  {
    ;
  }

}
