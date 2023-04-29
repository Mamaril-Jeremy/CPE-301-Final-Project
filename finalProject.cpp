//Author: Jeremy Mamaril and Brandon (Ri) Yu

#include <LiquidCrystal.h>
#include <Stepper.h>
#include <dht.h>

#define WATER_SENSOR_PIN 5 //ADC channel 5
#define FAN_ENABLE 30
#define WATER_LEVEL_THRESHOLD 150
#define TEMP_THRESHOLD 25

#define DHT11_PIN 34

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
volatile unsigned char *portC = (unsigned char *) 0x28; //pins 30, 32

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


LiquidCrystal lcd(16, 17, 18, 19, 20, 21); //creates lcd object - pins 16-21 taken
dht DHT;
const int stepsPerRev = 2038;
Stepper myStepper = Stepper(stepsPerRev, 2, 3, 4, 5); //pins 2-5 taken
bool moveLeft = false, moveRight = false, start;
int error, temp, humidity, waterLevel;


enum States {
  DISABLED = 1, //yellow LED
  IDLE = 2, //green LED
  ERROR = 3, //red LED
  RUNNING = 4, //blue LED
};

enum States currentState = DISABLED;

void setup(){
  U0init(9600);
  adc_init(); //initializes the ADC
  
  *portDDRB |= 0b11111000; //using pwm pins 10-13 for LEDs, pin 50 for power to water sensor
  *portB &= 0b11110111; //turn the sensor off

  lcd.begin(16, 2); //starts the lcd

  *portDDRA &= 0b10101010; //set all port A to input
  *portB |= 0b10000000; //turn on yellow Led on for disabled state
  *portDDRC |= 0b00101000; //initializes pins 32 (DIR1:PC5), 34(DIR2:PC3) to output

  start = false;
}


void loop(){
  //start stop button
  
  if(*portPinA &= 0b01000000){
    if(start){
      start = false;
    }
    else{
      start = true;
    }
  }

  //Reset Button
  bool reset = false;
  if(*portPinA &= 0b00010000){
    reset = true;
  }

  *portB |= 0b00001000; //turn the water sensor on
  my_delay(10);

  error = DHT.read11(DHT11_PIN);
  temp = DHT.temperature;
  humidity = DHT.humidity;
  waterLevel = adc_read(WATER_SENSOR_PIN); 

  *portB &= 0b11110111; //turn the sensor off


  Serial.print( "T = " );
  Serial.print(DHT.temperature, 1);
  Serial.print(" deg. C, H = ");
  Serial.print(DHT.humidity, 1);
  Serial.println("%");
  Serial.print("Sensor value: ");
  Serial.println(waterLevel);
  Serial.println(currentState);

  //changing and updating the state
  if(start == false){
    currentState = DISABLED;
  }
  else{
    if(reset == true || temp <= TEMP_THRESHOLD){ 
      currentState = IDLE;
    }
    if(temp > TEMP_THRESHOLD){
      currentState = RUNNING;
    }
    if(waterLevel <= WATER_LEVEL_THRESHOLD){
      currentState = ERROR;
    }
  }

  //execute the instructions for the state
  switch(currentState) {
    case DISABLED:
      Serial.println("Disabled State"); //for now
      disabled_state();
      break;
    case IDLE:
      Serial.println("Idle State");
      idle_state();
      break;
    case ERROR:
      Serial.println("Error State");
      error_state();
      break;
    case RUNNING:
      Serial.println("Running State");
      running_state();
      break;
    default:
      break;
  }
}
//End of loop

void disabled_state(){
  *portB |= 0b10000000; //turn on yellow LED on for disabled state
  *portB &= 0b10001111; //turn off all other LEDs
  writeToLCD(); //display stuff to LCD
  turnOffFan(); //turn off fan

  //working with stepper motor
  if(*portPinA &= 0b00000100){
    moveLeft = true;
  }
  if(*portPinA &= 0b00000001){
    moveRight = true;
  }
  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
  my_delay(150);
}

void idle_state(){
  *portB |= 0b01000000; //turn on green LED on for disabled state
  *portB &= 0b01001111; //turn off all other LEDs
  writeToLCD();
  turnOffFan(); //turn off fan

  //working with stepper motor
  if(*portPinA &= 0b00000100){
    moveLeft = true;
  }

  if(*portPinA &= 0b00000001){
    moveRight = true;
  }

  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
  my_delay(150);
}

void error_state(){
  *portB |= 0b00100000; //turn on red LED on for disabled state
  *portB &= 0b00101111; //turn off all other LEDs
  writeToLCD();
  turnOffFan(); //turn off fan

 ///no stepper motor
  my_delay(150);
}

void running_state(){
  *portB |= 0b00010000; //turn on yellow LED on for disabled state
  *portB &= 0b00011111; //turn off all other LEDs
  writeToLCD();
  turnOnFan(); //turn on fan

  //working with stepper motor
  if(*portPinA &= 0b00000100){
    moveLeft = true;
  }
  if(*portPinA &= 0b00000001){
    moveRight = true;
  }
  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
  my_delay(150);
}

void writeToLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(DHT.temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(DHT.humidity);
  lcd.print("%");
}


void moveVent(bool left, bool right){
  if(left == true){
    myStepper.setSpeed(5);
    myStepper.step(-stepsPerRev);
    my_delay(10);
  }
  if(right == true){
    myStepper.setSpeed(5);
    myStepper.step(stepsPerRev);
    my_delay(10);
  }
}

void turnOnFan(){
  *portC &= 0b11011111;
  *portC |= 0b00001000;
  analogWrite(FAN_ENABLE, 255);
  my_delay(25);
  analogWrite(FAN_ENABLE, 90);
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
