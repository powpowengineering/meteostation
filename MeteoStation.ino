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
//#include "ModuleRAK811.h"
#include "Init.h"
#include <math.h>
#include <TimeLib.h>
#include <EEPROM.h>

/********************************************************************/				

DATE_MENU_SCREEN menuDate;
TIME_MENU_SCREEN menuTime;
ALARM_MENU_SCREEN menuAlarm;

bool alarmTime = false;
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
const uint8_t* t3_deviceAddress = t_deviceAddress[0];

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
    delay(2000);
    lcd.clear();
    lcd.setBacklight(false);
    lcd.setCursor(0,5);
    lcd.setInverted(true);
	lcd.print("  ");

    lcd.print("Hello ");
	lcd.print("world!");
    lcd.setInverted(false);
    //lcd.println("\nI am here.");
    lcd.print("\nI am here.", 1, 2);

    pinMode(PIN_INT_ALARM, INPUT);// пин для внешнего прерывания от RTC
    pinMode(PIN_INT_BUTTON, INPUT);// пин для внешнего прерывания от button
    pinMode(PIN_CS_SD_CARD_1, OUTPUT); 
    pinMode(PIN_CS_SD_CARD_2, OUTPUT);
    pinMode(53, OUTPUT);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // настройка режима сна


    sensors.begin();

    barometer.begin(Wire, LPS25HB_I2C_ADDR_ALT);

    humidity_sensor.begin();

    rtc.setSecond(BUILD_SEC);    // устанавливаем секунд
    rtc.setMinute(BUILD_MIN);    // установка минут
    rtc.setHour(BUILD_HOUR);     //установка часов
    rtc.setDate(BUILD_DAY);     // устанавливаем число
    rtc.setMonth(BUILD_MONTH);   // Устанавливаем месяц
    rtc.setYear(BUILD_YEAR);    // Устанавливаем год
    rtc.setClockMode(false);    // установка режима 12/24h. True is 12-h, false is 24-hour.
    DateTime timeCurrent_al = RTClib::now();
    rtc.setA1Time(0,0,0,10,0x0e, false, false, false);//setA1Time(byte A1Day, byte A1Hour, byte A1Minute, byte A1Second, byte AlarmBits, bool A1Dy, bool A1h12, bool A1PM)
    rtc.turnOnAlarm(1);

  	menuDate.date.day = 1;
  	menuDate.date.month = 1;
  	menuDate.date.year = 1;

    attachInterrupt(INT_ALARM, isrAlarm, FALLING);  // прерывание от RTC
    attachInterrupt(INT_BUTTON,isrButtonPressed,FALLING); // прерывание от button

//    RAK811_init();
  //  RAK811_sendMessage(RAK811_confMode);
    //delay(200);
    //RAK811_sendMessage(RAK811_confPrm);
    //delay(200);
    //RAK811_sendMessage(RAK811_taransferMode);
   // delay(200);

}

void loop()
{
    if (true == pressAnyButton)
    {
        Serial.println("\r\npressAnyButton");    
        ReadSensors();
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


    if ((timeCurrent.unixtime() - timeOld.unixtime())>TIME_SCREEN_ON)
    {
        SetAlarm(menuAlarm.alarm.scale, menuAlarm.alarm.period);
        rtc.checkIfAlarm(ALARM_1);// сбрасываем флаг ALARM_1
        attachInterrupt(INT_ALARM,isrAlarm,FALLING);  // прерывание от RTC
        attachInterrupt(INT_BUTTON,isrButtonPressed,FALLING); // прерывание от button
        lcd.clear();
        lcd.setCursor(0,2);
        lcd.print("Sleep");
        sleep_mode(); // Переводим МК в сон
        lcd.clear();
        lcd.print("Wakeup");
    }
    else
    {
        // читаем время RTC раз в секунду
        timeDelay = millis();
        if ((timeDelay - timeDelayOld)>1000)
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
  alarmTime = true;
}






/********************************обработчик прерывания по кнопке********************/
void isrButtonPressed()
{
 Serial.println("\r\nisr Button");

 //RAK811_sendMessage(RAK811_FirstPartStrToSend);

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
