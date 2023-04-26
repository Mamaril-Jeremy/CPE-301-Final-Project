//Author: Jeremy Mamaril and Brandon (Ri) Yu

#include <LiquidCrystal.h>
#include <Stepper.h>
#include <DHT.h>

#define WATER_SENSOR_PIN 5 //ADC channel 5
#define WATER_LEVEL_THRESHOLD 150
#define TEMP_THRESHOLD 25

#define DHT11_PIN 25

#define RDA 0x80
#define TBE 0x20 

volatile unsigned char *portPinB = (unsigned char *) 0x23;
volatile unsigned char *portDDRB = (unsigned char *) 0x24; //LEDs 
volatile unsigned char *portB = (unsigned char *) 0x25; //pins 9-13 taken

volatile unsigned char *portPinE = (unsigned char *) 0x2C;
volatile unsigned char *portDDRE = (unsigned char *) 0x2D; //Button
volatile unsigned char *portE = (unsigned char *) 0x2E; //pins taken

volatile unsigned char *portPinC = (unsigned char *) 0x26;
volatile unsigned char *portDDRC = (unsigned char *) 0x27; //For motor 
volatile unsigned char *portC = (unsigned char *) 0x28; //pins 30, 32, 34, and 36 taken

//UART registers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

//ADC registers
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

//Timers
volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;


LiquidCrystal lcd(16, 17, 18, 19, 20, 21); //creates lcd object - pins 16-21 taken
dht DHT;
Stepper myStepper = Stepper(stepsPerRev, 2, 3, 4, 5); //pins 2-5 taken
int temp, waterLevel;
int stepsPerRev = 2038;

char sensorValue[14] = “Sensor value: “;

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

  *portDDRE &= 0b11010011; //set all port E to input
  *portB |= 0b10000000; //turn on yellow Led on for disabled state
  *portDDRC |= 0b10101010; //initializes pins 30, 32, 34, and 36 to output for fan
}


void loop(){
  //start stop button
  bool start = false;
  if(*portPinE &= 0b00100000){
    if(start){
      start = false;
    }
    else{
      start = true;
    }
  }

  //Reset Button
  bool reset = false;
  if(*portPinE &= 0b00010000){
    reset = true;
  }

  *portB |= 0b00001000; //turn the water sensor on
  my_delay(10);

  waterLevel = adc_read(WATER_SENSOR_PIN);

  *portB &= 0b11110111; //turn the sensor off

  for(int i = 0; i < 15; i++){U0putchar(sensorValue[i]);}

  U0putchar((char) waterLevel);
  U0putchar(\n);

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

  //working with stepper motor
  bool moveLeft = false, moveRight = false;

  if(*portPinE &= 0b00000001){
    moveLeft = true;
  }
  if(*portPinE &= 0b00000010){
    moveRight = true;
  }
  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
}

void idle_state(){
  *portB |= 0b01000000; //turn on green LED on for disabled state
  *portB &= 0b01001111; //turn off all other LEDs
  writeToLCD();

  //working with stepper motor

  bool moveLeft = false, moveRight = false;

  if(*portPinE &= 0b00000001){
    moveLeft = true;
  }

  if(*portPinE &= 0b00000010){
    moveRight = true;
  }

  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
}

void error_state(){
  *portB |= 0b00100000; //turn on red LED on for disabled state
  *portB &= 0b00101111; //turn off all other LEDs
  writeToLCD();

 ///no stepper motor
}

void running_state(){
  *portB |= 0b00010000; //turn on yellow LED on for disabled state
  *portB &= 0b00011111; //turn off all other LEDs
  writeToLCD();

  //working with stepper motor

  bool moveLeft = false, moveRight = false;

  if(*portPinE &= 0b00000001){
    moveLeft = true;
  }

  if(*portPinE &= 0b00000010){
    moveRight = true;
  }

  if(moveLeft == true  || moveRight == true){
    moveVent(moveLeft, moveRight);
  }
}

void writeToLCD() {
  int error = DHT.read11(DHT11_PIN);
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
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b10111111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to set prescaler selection to slow reading

  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode

  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}



unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;

  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;

  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }

  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;

  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;

  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);

  // return the result in the ADC data register
  return *my_ADC_DATA;
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
