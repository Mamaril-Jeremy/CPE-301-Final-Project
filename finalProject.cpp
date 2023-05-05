//Author: Jeremy Mamaril and Brandon (Ri) Yu

#include <LiquidCrystal.h>
#include <Stepper.h>
#include <dht.h>
#include <RTClib.h>

#define WATER_SENSOR_PIN 5 //ADC channel 5
#define FAN_ENABLE 30
#define WATER_LEVEL_THRESHOLD 275
#define TEMP_THRESHOLD 25

#define DHT11_PIN 36

#define RDA 0x80
#define TBE 0x20 

volatile unsigned char *portPinB = (unsigned char *) 0x23;
volatile unsigned char *portDDRB = (unsigned char *) 0x24; //LEDs 
volatile unsigned char *portB = (unsigned char *) 0x25; //pins 9-13 taken

volatile unsigned char *portPinA = (unsigned char *) 0x20;
volatile unsigned char *portDDRA = (unsigned char *) 0x21; //Button
volatile unsigned char *portA = (unsigned char *) 0x22; //pins 22-28 taken

volatile unsigned char *portPinC = (unsigned char *) 0x26;
volatile unsigned char *portDDRC = (unsigned char *) 0x27; //For motor 
volatile unsigned char *portC = (unsigned char *) 0x28; //pins 32 and 34

//UART registers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

//Timers
volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;


LiquidCrystal lcd(43,45,47,49,51,53); //creates lcd object - pins 43-53 taken
dht DHT;

RTC_DS1307 RTC; //real-time clock module
//Step Motor
const int stepsPerRev = 2048;
const int motorSpeed = 150;
Stepper myStepper = Stepper(stepsPerRev, 2, 4, 3, 5); //pins 2-5 taken

bool moveLeft = false, moveRight = false, start = false, reset = false;
int error, temp, humidity, waterLevel;


enum States {
  DISABLED = 1, //yellow LED
  IDLE = 2, //green LED
  ERROR = 3, //red LED
  RUNNING = 4, //blue LED
};

enum States currentState = DISABLED;

void setup(){
  Serial.begin(9600);
  adc_init(); //initializes the ADC
  
  *portDDRB |= 0b11111000; //using pwm pins 10-13 for LEDs, pin 50 for power to water sensor
  *portB &= 0b11110111; //turn the sensor off

  lcd.begin(16, 2); //starts the lcd

  RTC.begin(); //starts the real-time clock module
  DateTime now = DateTime(2023, 5, 2, 0, 0, 0);
  RTC.adjust(now);

  *portDDRA &= 0b10101010; //set all port A to input
  *portB |= 0b10000000; //turn on yellow Led on for disabled state
  *portDDRC |= 0b00101000; //initializes pins 32 (DIR1:PC5), 34(DIR2:PC3) to output

}


void loop(){
  
  if(*portPinA &= 0b01000000){
    if(start == false){
      start = true;
    }
    else{
      start = false;
    }
  }
  if(*portPinA &= 0b00010000){
    if(reset == false){
      reset = true;
    }
    else{
      reset = false;
    }
  }
  DateTime timeNow = RTC.now();

  if(currentState!=DISABLED){
      error = DHT.read11(DHT11_PIN);
      temp = DHT.temperature;
      humidity = DHT.humidity;
      *portB |= 0b00001000; //turn the water sensor on
      my_delay(10);
      waterLevel = adc_read(WATER_SENSOR_PIN);
      *portB &= 0b11110111; //turn the water sensor off
    
      if(currentState!=ERROR){
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temp);
        lcd.print((char)223);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Humidity: ");
        lcd.print(humidity);
        lcd.print("%");
      }
      else{
        lcd.setCursor(0, 0);
        lcd.print("Water level too low.");
        lcd.setCursor(0, 1);
        lcd.print("Level: ");
        lcd.print(waterLevel);
      } 
  }

  //changing and updating the state
  if(start == false){
    currentState = DISABLED;
  }
  else{
    if(reset == true){ 
      currentState = IDLE;
    }
    else if(temp <= TEMP_THRESHOLD){
      currentState = IDLE;
    }
    else if(temp > TEMP_THRESHOLD && waterLevel >= WATER_LEVEL_THRESHOLD){
      currentState = RUNNING;
    }
    else if(waterLevel < WATER_LEVEL_THRESHOLD){
      currentState = ERROR;
    }
    else{}
  }
  
  //execute the instructions for the state
  switch(currentState) {
    case DISABLED:
      disabled_state();
      break;
    case IDLE:
      idle_state();
      break;
    case ERROR:
      error_state();
      break;
    case RUNNING:
      running_state();
      break;
    default:
      break;
  }

  if(*portPinA &= 0b00000100){
    if(currentState != ERROR){
      myStepper.setSpeed(motorSpeed);
      myStepper.step(-1);
    }
  }
  if(*portPinA &= 0b00000001){
    if(currentState != ERROR){
      myStepper.setSpeed(motorSpeed);
      myStepper.step(1);
    }
  }
}
//End of loop

void disabled_state(){
  *portB |= 0b10000000; //turn on yellow LED on for disabled state
  *portB &= 0b10001111; //turn off all other LEDs
  turnOffFan(); //turn off fan
}

void idle_state(){
  *portB |= 0b01000000; //turn on green LED on for idle state
  *portB &= 0b01001111; //turn off all other LEDs
  turnOffFan(); //turn off fan
}

void error_state(){
  *portB |= 0b00100000; //turn on red LED on for error state
  *portB &= 0b00101111; //turn off all other LEDs
  turnOffFan(); //turn off fan

 ///no stepper motor
}

void running_state(){
  *portB |= 0b00010000; //turn on blue LED on for running state
  *portB &= 0b00011111; //turn off all other LEDs
  turnOnFan(); //turn on fan
}

void moveVent(bool left, bool right){
  if(left == true){
    myStepper.setSpeed(motorSpeed);
    myStepper.step(-1);
  }
  if(right == true){
    myStepper.setSpeed(motorSpeed);
    myStepper.step(1);
  }
}

void turnOnFan(){
  for(int i = 255; i>100;i--){
    *portC &= 0b11011111;
    *portC |= 0b00001000;
    analogWrite(FAN_ENABLE, i*2);
    my_delay(25);
  }
}

void turnOffFan(){
  *portC &= 0b11011111; // set pin 5 low
  *portC &= 0b11110111; // set pin 3 low
  analogWrite(FAN_ENABLE, 0); // set fan enable pin to 0 (turn off)
  my_delay(25);
}

//Start of UART functions
void U0init(unsigned long U0baud)
{
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  
  tbaud = (FCPU / 16 / U0baud - 1);

  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;
}


unsigned char U0kbhit()
{
  return (*myUCSR0A & RDA);
}

unsigned char U0getchar()
{
  return *myUDR0;
}

void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A &= TBE) == 0){};
  *myUDR0 = U0pdata;
}
//End of UART functions


//Start of ADC functions
void adc_init()
{
  ADCSRA = 0x80;
  ADCSRB = 0X00;
  ADMUX = 0x40;
}


unsigned int adc_read(unsigned char adc_channel){
     ADCSRB &= 0xF7; //Reset MUX5.
     ADCSRB |= (adc_channel & 0x08); //Set MUX5.
     ADMUX &= 0xF8; //Reset MUX2:0.
     ADMUX |= (adc_channel & 0x07); //Set MUX2:0.
     ADCSRA |= 0x40; //Start the conversion.
     while (ADCSRA & 0x40) {} //Wait for conversion to complete
     return ADC; //Return the converted number.
}
//End of ADC functions


//My delay function
void my_delay(unsigned int freq)
{
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  * myTCCR1B |= 0b00000001;
  // wait for overflow
  while((*myTIFR1 & 0x01)==0); // 0b00000000
  // stop the timer
  *myTCCR1B &= 0xF8;   //0b00000000
  // reset TOV           
  *myTIFR1 |= 0x01;
}
