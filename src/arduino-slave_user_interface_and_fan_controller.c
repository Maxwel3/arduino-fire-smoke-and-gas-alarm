/********************************************************************************************************************

Copyright 2020 (c) Maximilian Wenkebach <max.wenkebach@gmail.com>

Arduino Fire, Smoke and Gas alarm can not be copied and/or distributed without the express
permission of Maximilian Wenkebach

Code for the Slave Arduino Managing the Display of information to the user using the LCD Screen as well as RGB LEDs

Title: Arduino Fire, Smoke and Gas alarm - User interface and Fan control script
Project: Arduino Fire, Smoke and Gas alarm 

Author: Maximilian Wenkebach

Date of Initial Version: 2020-04-21
Date of Current Version: 2020-04-23
Current Version: 0.1
Currant Revision: 3
Version Code: v0.1-3

TODO List:
- package LCD control functions for temperature display into seperate function

********************************************************************************************************************/

//liabry to control the comunication via I2C
#include <Wire.h>
//libary to control the LCD screen
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //initialise the pins for the LCD

int pin_R = 9; //set the pin for the Red led channel
int pin_G = 10; //set the pin for the Green led channel
int pin_B = 13; //set the pin for the Blue led channel
int pin_Button = 14; //set the pin for Button
int pin_relay = 7; //set the pin for the relay
int pin_master_power_fault = 8; //set the pin for the master power fault channel
int pin_piezo = 6; //set the pin for the piezo

bool alarmstate = false; //initialise the bool variable for the alarm state
bool gas_detect = false; //initialise the bool variable for the gas detection state
bool sensor_display_state = false; //initialise the bool variable to store the state of the sensor display

int btn_state; //intitalise the variable to store the state of the button read from the pin

int temp; //intitalise the variable to store the temperature recived from the master via I2C
int code; //intitalise the variable to store the Status code recived from the master via I2C

/**
Function to set the color of the two status LEDs
@param int r: Red value (0-255)
@param int g: Green value (0-255)
@param bool b: toggle the blue LED to full or of (portlimitation)
*/
void set_led(int r, int g, bool b)
{
  analogWrite(pin_R, r); //set the red value
  analogWrite(pin_G, g); //set the green value
  if (b) //excec if the variable for blue is true
  {
    digitalWrite(pin_B, HIGH); //if the variable is true set the digital output for blue to high
  }
  else //if the variable is not true
  {
    digitalWrite(pin_B, LOW); //if the variable is not true set the digital output for blue to low
  }
}

/**
Toggle the Display of Sensor data on The LCD Dispaly
*/
void sensor_display()
{
  while (btn_state == LOW) //excec as long as the button is held down
  {
    sensor_display_state = true; //set the sensor display state variable to true
    if (gas_detect) //excec if the variable gas_detect is true
    {
      //set the text on the LCD*/
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Temp: "+String(temp)+" C");
      lcd.setCursor(0,1);
      lcd.print("Gas Detected");
    }
    else //if the variable gas_detect is not true
    {
      //set the text on the LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Temp: "+String(temp)+" C");
      lcd.setCursor(0,1);
      lcd.print("No Gas Detected");
    }
    delay(5000); //delay the end of the function by 5 seconds (5000ms) to give the user time to read the display
    
    btn_state = digitalRead(pin_Button); //check if the button is still being held before exiting the loop
  } //end of while loop
  
  if (alarmstate && sensor_display_state) //excec if the alarmstate variable is true
  {
  	lcd.clear(); //clear the LCD
    lcd_control("Fans: ON", "Alarm: ON"); //set the text on the LCD
  } 
  else if (sensor_display_state) //excec if the alarmstate variable is not true
  {
    lcd.clear(); //clear the LCD
	lcd_control("Fans: OFF", "Alarm: OFF"); //set the text on the LCD
  }
  
  sensor_display_state = false; //set the sensor display state variable to false
}

/**
Fuction to set the text on the display
@param char *l1: String for the first line of the display
@param char *l1: String for the second line of the display
*/

void lcd_control(char *l1, char *l2)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(l1);
  lcd.setCursor(0,1);
  lcd.print(l2);
}

/**
Arduino Setup Function
*/
void setup()
{
  Wire.begin(4); //Start Wire on Channel 4
  Wire.onReceive(receiveEvent); //Call the Function receiveEvent on wire recive
  Serial.begin(9600); //Start the Serial output  
  lcd.begin(16, 2); //Initialise the LCD Display
  
  lcd_control("Fans: OFF", "Alarm: OFF"); //Set the Default text for the LCD
  
  //Setting up the pins
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_master_power_fault, INPUT);
  pinMode(A3, INPUT);
  pinMode(pin_R, OUTPUT);
  pinMode(pin_G, OUTPUT);
  pinMode(pin_B, OUTPUT);
  pinMode(pin_Button, INPUT);
  
  set_led(0,255,false);	//Setting the LEDs to the Starting Values
}

/**
Arduino Loop Function
*/
void loop()
{
  int val_master_power = digitalRead(pin_master_power_fault); //check if the Master Arduino has lost power
  int val_gas = digitalRead(A3); //check if the Master is Outputting the gas warn signal
  
  Serial.println(val_gas);
  
  btn_state = digitalRead(pin_Button); //check if the Button is pressed
  
  if (val_master_power != HIGH) //excec if the Master Arduino has lost power
  {
    digitalWrite(pin_relay, HIGH); //open the Relay to start the fans
    
 	set_led(255,255,false);	//set the LEDs to red
    
    lcd_control("Fans: ON", "Alarm: OFF"); //Set the Text on the LCD
  }
  if (val_gas == HIGH) //excec if the Master Arduino is Outputting the gas warn signal
  {
  	gas_detect = true; //set the variable gas_detect to true
  }
  else 
  {
  	gas_detect = false; //set the variable gas_detect to false
  }
  

  sensor_display(); //run the fuction to display the data
  
  delay(1000); //wait 1 Second (1000ms) to prevent the loop from running every milisecond
}

/**
Function to handle the data Transfered via the Wire.h libary using I2C
@param int howMany: number of bytes recived
*/
void receiveEvent(int howMany) 
{
  int x = Wire.read();    // receive byte as an integer
  
  if (x < 199) //as the temperature sensor cant register anything above ~122 °C any value below 200 has to be the temperature 
  {
 	temp = x; //asigning the value recived to the temp (for temperature) variable
    Serial.println("Temp: "+String(x)); //debug output to the Serial Monitor of the temperature variable
  } 
  else if (x > 199 && x < 300) //as the temperature sensors output will always be below 200, anything above has to be the code
  {
  	code = x; //assing the value recived to the variable storing the current status code
    Serial.println("code: "+String(x)); //debug output to the Serial Monitor of the code variable
  }
  
  if (code == 200) //excec if the code is equal to 200 (sensor alert)
  {
    lcd_control("Fans: ON", "Alarm: ON"); //Set the Text on the LCD
    
    digitalWrite(pin_relay, HIGH); //open the relay to start the ventilation fans
    analogWrite(pin_piezo, 100); //play the alarm sound using the piezo
    set_led(255,0,false); //set the LEDs to red
    alarmstate = true; //set the alarmstate variable to true
  }
  else if (code == 255 && digitalRead(A3) == HIGH) // excec if the code is not 200 but 255 (no alert / clear) and if the master power failure is not detected
  {
    lcd_control("Fans: OFF", "Alarm: OFF"); //Set the Text on the LCD
    
    digitalWrite(pin_relay, LOW); //close the relay to stop the fans and prevent excess power useage 
    analogWrite(pin_piezo, 0); //stop the piezo alarm
    set_led(0,255,false); //set the LEDs to Green
    alarmstate = false; //set the alarmstate variable to false
  }
}
