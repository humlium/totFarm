#include <TimerOne.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

#define DHT_PIN 5
#define DS18_PIN 6
#define GY30_PIN 4
#define DHT_TYPE DHT11
#define TEMPERATURE_PRECISION 9

OneWire oneWire(DS18_PIN);
DeviceAddress tempDeviceAddress;
DallasTemperature sensors(&oneWire);

int BH1750address = 0x23; 
byte buff[2];
byte state = 1;
int tCount = 0;

enum caseDefine{
  operate = 1,
  setting
};

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  pinMode(GY30_PIN, OUTPUT);
  digitalWrite(GY30_PIN, 0);
  pinMode(7, OUTPUT);
  digitalWrite(7, 1);
  // put your setup code here, to run once:
  //Serial.begin(9600);
  Serial1.begin(9600);
  Timer1.initialize(10000);
  Timer1.attachInterrupt(interruptTimer);
  sensors.begin();
  dht.begin();
  Wire.begin();
}

void interruptTimer(void){
  if(state == operate) tCount++;
}

void loop() {
  switch (state){
    case operate:
        if(Serial.available()) {
          char inChar = (char)Serial.read();
          if(inChar=='S')state = setting;   
        }
        if(tCount >= 500){
          uint16_t val=0;
          float dhtTemp = dht.readTemperature();
          float dhtHumi = dht.readHumidity();
          Serial1.print("$");
          Serial1.print("temp:");
          serialPrintFloat(dhtTemp);
          Serial1.print(" humi:");
          serialPrintFloat(dhtHumi);
  
          Serial1.print(" DS_TEMP:");
          sensors.requestTemperatures(); 
          Serial1.print(sensors.getTempCByIndex(0)); 

          Wire.beginTransmission(BH1750address);
          Wire.write(0x10);//1lx reolution 120ms
          Wire.endTransmission();
          delay(200); 
          if(2==BH1750_Read(BH1750address))
          {
              val=((buff[0]<<8)|buff[1]);
              Serial1.print(" light:");
              Serial1.print(val,DEC);     
              Serial1.print(" lux"); 
          }
          Serial1.print("\n");
    
          tCount = 0;
        }
    break;
    case setting:
        if (Serial.available()) {
        int inByte = Serial.read();
        Serial1.write(inByte);
        }
    break;
    default:
    break;   
  }
  
}


void serialPrintFloat(float f){
  Serial1.print((int)f);
  Serial1.print(".");
  int decplace = (f - (int)f) * 100;
  Serial1.print(abs(decplace));
}

int BH1750_Read(int address) //
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();  
  return i;
}

void serialEvent1() {
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    Serial.print(inChar);   
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if(inChar=='R')state = operate;
    if(inChar=='S')state = setting;   
  }
}
