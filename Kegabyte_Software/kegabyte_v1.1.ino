/**********************************************************************************
// Kegabyte Project: Kegerator Monitoring System
// Author: Lance Gundersen https://github.com/LGProspects/Kegabyte
// Version 1.1
// The Kegabyte project and documentation, sketches and examples are free software; you can 
// redistribute it and/or modify it under the terms of the Creative Commons 
// Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) License 
// (http://creativecommons.org/licenses/by-sa/4.0/); either version 4 of the License, 
// or (at your option) any later version.
// I imp


#include <DHT.h>
#include <Wire.h>
#include <math.h>

#define DHT1 10
#define DHT2 9
#define DHTTYPE DHT11
DHT dht_1(DHT1, DHTTYPE);
DHT dht_2(DHT2, DHTTYPE);

#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 3
#define REDPOT A1
#define GREENPOT A2
#define BLUEPOT A3

#include <LiquidCrystal_I2C.h>
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.

// set the LCD address to 0x20 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

const int buttonPin = 13;
int buttonState = 0;

/*
LCD Connections:
rs (LCD pin 4) to Arduino pin 12
rw (LCD pin 5) to Arduino pin 11
enable (LCD pin 6) to Arduino pin 10
LCD pin 15 to Arduino pin 13
LCD pins d4, d5, d6, d7 to Arduino pins 5, 4, 3, 2

Thermistor Connections:
Thermistor Pin 1 to +5v
Thermistor Pin 2 to Analog Pin 0
10k ohm resistor pin 1 to Analog Pin 0
10k ohm resistor pin 2 to Gnd
*/

#define FLOWSENSORPIN 7

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}



void setup(void) {
  
// --- Set up LCD --- //  
  lcd.begin(20, 4);              // 20,4 for a 20x4 LCD.
  lcd.clear();                   // start with a blank screen
  lcd.setCursor(0,0);            // set cursor to column 0, row 0
  
// --- Set up DHT's --- //
  lcd.print("Hello Lance!");
  delay(1000);
  dht_1.begin();
  dht_2.begin();
  
// --- Set up flow sensors --- //
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);
  
// --- Set up buttons --- //
  pinMode(buttonPin, INPUT);
  
// --- Set up RGB output pins --- //  
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  
// --- Set up RGB potentiometers --- //
  pinMode(REDPOT, INPUT);
  pinMode(GREENPOT, INPUT);
  pinMode(BLUEPOT, INPUT);

}

void printMeters(void) {
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

  float ounces;
  ounces = (liters * 33.8);
  lcd.setCursor(0, 0);
  lcd.print("Line 1 "); lcd.print(ounces); lcd.print(" Oz        ");
  lcd.setCursor(0, 1);
  lcd.print("Line 2 "); lcd.print(ounces); lcd.print(" Oz        ");
 
  delay(100);
}

void loop(void) {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Fahrenheit
  float f_A = dht_1.readTemperature(true);
  float f_B = dht_2.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(f_A || f_B)) {
    lcd.println("Failed to read from DHT sensor!");
    return;
  }
  
// --- Whenever the RGB potentiometers are rotated they change colors --- //  
  int r, g, b;
  int rPot, gPot, bPot;
    analogWrite(REDPIN, analogRead(REDPOT)/4);  // Copy 10-bit input to 8-bit output
    analogWrite(GREENPIN, analogRead(GREENPOT)/4);  // Copy 10-bit input to 8-bit output
    analogWrite(BLUEPIN, analogRead(BLUEPOT)/4);  // Copy 10-bit input to 8-bit output
  
  
// --- Reading button. Unpressed displays temperatures from dht1 and dht2 while pressed displays ounces poured  --- //  
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == LOW) {
    lcd.clear();
    lcd.print("Chest Temp: "); 
    lcd.print(f_A);
    lcd.setCursor(0,2);
    lcd.print("Tower Temp: ");
    lcd.print(f_B);
  } 
  else {
    lcd.clear();
    printMeters(); 
  }
  
  
  //printTemp();
  //printMeters();
  delay(1000);
}
