/********************************************************************************************************************

Copyright 2020 (c) Maximilian Wenkebach <max.wenkebach@gmail.com>

Arduino Fire, Smoke and Gas alarm can not be copied and/or distributed without the express
permission of Maximilian Wenkebach

Code for the Master Arduino Managing the Collection of Sensordata and forwarding of that data to the Slave arduino

Title: Arduino Fire, Smoke and Gas alarm - Sensor Control script
Project: Arduino Fire, Smoke and Gas alarm 

Author: Maximilian Wenkebach

Date of Initial Version: 2020-04-21
Date of Current Version: 2020-04-23
Current Version: 0.1
Currant Revision: 3
Version Code: v0.1-3

TODO List:
- package LCD control functions into seperate function

********************************************************************************************************************/

//liabry to control the comunication via I2C
#include <Wire.h>

int alerttemp = 60; //initialise the variable to store the temperature the alarm will be set of at and set it to 60
int temp = 0; //initialise the variable to store the currently recorded temperature
int gas_conc = 0; //initialise the variable to store the currently level of gas concentration
int code_warn = 200; //initialise the variable to store the warning code and set it to 200 (alarn active)
int code_all_clear = 255; //initialise the variable to store the all clear code and set it to 255 (no alarm)
bool alarmstate = false; //initialise the variable to store the alarmstarte
 
int pin_gas_sig_out = 6; //set the pin for the gas warn signal line
int pin_deadmansignal_master_power_loss = 7; //set the pin to transmit the deadman signal on
/**
Function to trigger the alarm on the connected slave devices
*/
void alert()
{
  Wire.beginTransmission(4); //Start the wire teansmision on channel 4
  Wire.write(temp); //transmit the currently recorded temperature to all connectet slave devices
  Serial.println("Temperature: "+String(temp)); //debug output to the Serial Monitor of the currently recorded temperature
  Wire.endTransmission(); //terminate the wire transmision
  
  delay(500); //delay the the transmision of the code variable by 0.5s to avoid complications that could arise from transmitting the variables to quickly after each other 
  
  Wire.beginTransmission(4); //Start the wire teansmision on channel 4
  Wire.write(code_warn); //transmit the current status code to all connectet slave devices
  Serial.println("Status Code: "+String(code_warn)); //debug output to the Serial Monitor of the current Status code
  Wire.endTransmission(); //terminate the Wire transmision
}

/**
Function to deactivate the alarm on all connected slave devices
*/
void alert_stop()
{
  Wire.beginTransmission(4); //Start the wire teansmision on channel 4
  Wire.write(temp); //transmit the currently recorded temperature to all connectet slave devices
  Serial.println("Temperature: "+String(temp)); //debug output to the Serial Monitor of the currently recorded temperature
  Wire.endTransmission(); //terminate the wire transmision
  
  delay(500); //delay the the transmision of the code variable by 0.5s to avoid complications that could arise from transmitting the variables to quickly after each other 
  
  Wire.beginTransmission(4); //Start the wire teansmision on channel 4
  Wire.write(code_all_clear); //transmit the current status code to all connectet slave devices
  Serial.println("Status Code: "+String(code_all_clear)); //debug output to the Serial Monitor of the current Status code
  Wire.endTransmission(); //terminate the Wire transmision
}

/**
Arduino Setup Function
*/
void setup()
{
  Wire.begin(); //Start Wire on as Master device
  Serial.begin(9600); //Start the Serial output  
  
  //Setting up the pins
  pinMode(pin_deadmansignal_master_power_loss, OUTPUT);
  pinMode(A0, INPUT); //Gas sensor
  pinMode(A1, INPUT); //Temperature Sensor
  pinMode(pin_gas_sig_out, OUTPUT);
  
  digitalWrite(pin_deadmansignal_master_power_loss, HIGH); //start to send the deadman signal to all slave devices
}

/**
Arduino Loop Function
*/
void loop()
{
  temp = map(((analogRead(A1) - 20) * 3.04), 0, 1023, -40, 125); //Read in and Convert the output from the Temperature sensor to Celsius
  gas_conc = analogRead(A0); //Read in the Output from the Gas sensor
  
  Serial.println("====="); //print a spacer to the Serial Monitor to make the debug output more readable
  
  if(gas_conc >= 750) //excec if the value representing the current gas concentration reportet by the gas-sensor is over 750
  {
    digitalWrite(pin_gas_sig_out, HIGH); //write the gas warn signal to the gas warning line
    Serial.println("Gas Detectet"); //debug output to the Serial Monitor that gas was detectet
  }
  else
  {
  	digitalWrite(pin_gas_sig_out, LOW); //write the gas all clear to the gas warning line
  }
  
  if (temp >= alerttemp || gas_conc >= 750) //if the temperature exceeds the perset alert temperature or if the reportet gas concentration exceeds 750
  {
    alert(); //call the function to trigger the alarm on all connectet slave devices
  }
  else //excec if none of the alert conditions are met.
  {
    alert_stop(); //call the function to dissable the alarm on all connectet slave devices
  }
}
