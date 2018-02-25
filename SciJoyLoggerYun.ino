//libraries
#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h"

//Values for Temboo to append to spreadsheet
#define CLIENT_ID "xxxx"
#define CLIENT_SECRET "xxx"
#define REFRESH_TOKEN "xxx"
#define SPREADSHEET_ID "xxx"

//Setting up e-mail for Temboo
#define GMAIL_USER_NAME "xxxx"
#define GMAIL_APP_PASSWORD "xxxx"
#define TO_EMAIL_ADDRESS "xxxx"
boolean attempted = false; 

//Pin constants
#define ledPin 13
#define solenoidPin 4
#define motorPin 2
#define sensorLDR 0
#define sensorForce 1
#define sensorTemperature 2

//for tracking durations
unsigned long previousMillis0=0;
unsigned long previousMillis1=0;
unsigned long previousMillis2=0;
unsigned long previousMillis3=0;
unsigned long previousMillis4=0;
unsigned long previousMillis5=0;

//the serial vaules from the pi
//sensor, SampleRate, sensorDuration, thresholdMinMax, threshold, Actuator, ActuatorDuration, Trigger, TriggerMinMax
//0-6, mills, mills, min-0/max-1,int,0-6,mills,min-0/max-1,int
int deltaShadow [9] = {0,0,0,0,0,0,0,0,0};

//Rows are the type of sensor and columns are the otehr 8 values from deltaShadow
int deltaArray [6][8]={{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0}};


//Returns from sensors functions that trigger actuators to turn on
int actuatorReturn[6] = {0,0,0,0,0,0};
//Duration of actuator
int actuatorDuration[6] = {0,0,0,0,0,0};
//If actuator is being used. Sensors can be logged with no actuator
int actuatorEnabled[6] = {0,0,0,0,0,0};

int actLDR;
int actTemp;
int actForce;

int counts[] = {0,0,0,0,0,0};

void setup() 
{
//Start serial communitcation with Pi
  Serial.begin(9600);
  while (!Serial); 
  Serial.print(F("Initializing the bridge...")); //Email and google sheets
  Bridge.begin();
  Serial.println(F("Done"));
  Serial.print(TEMBOO_ACCOUNT);

//initialize actuator pins
  pinMode(motorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
}

void loop() 
{
  long unsigned startTime[] = {0,0,0,0,0,0};
  long unsigned currentTime; //something that tracks time in the main function and then sends to sensor functions
  long unsigned lastTime[] = {0,0,0,0,0,0};
  long unsigned elapsedTime[] = {0,0,0,0,0,0};
 
  
  static unsigned long previousMillis1;
  static unsigned long previousMillis2;
  static unsigned long previousMillis3;

  char piArray[60];
  //Read from Pi  - look up while and look up readString
  boolean stringComplete = false;
  byte arrayPostion =0;
  ///the Pi must only send the delta when the logging needs to start!!
  while (Serial.available() && !stringComplete) 
  {
    char inChar = (char)Serial.read();
    piArray[arrayPostion] = inChar;
    arrayPostion++;
    Serial.print("The array is coming: ");
    Serial.print(arrayPostion);
    Serial.println(piArray[arrayPostion]);
    if (inChar == '\n') 
    {
      stringComplete = true;
    }
  }

  //Parse the int that identifies the sensor 0-temp, 1-force....

  byte i;
  char * pch;
  //  piString.toCharArray(piArray, 50);
  pch = strtok (piArray,",");
  while (pch != NULL)
  {
    deltaShadow [i] = atoi(pch);
    pch = strtok (NULL, ",");
    Serial.print("Delta Shadow: ");
    Serial.println(deltaShadow[i]);
    i++;
  }

  byte index = deltaShadow[0]; //determines what sensor is being used and choses the "row" of the delta array
  //fill column values for that row
  deltaArray[index][0] = deltaShadow[1];
  deltaArray[index][1] = deltaShadow[2];
  deltaArray[index][2] = deltaShadow[3];
  deltaArray[index][3] = deltaShadow[4];
  deltaArray[index][4] = deltaShadow[5];
  deltaArray[index][5] = deltaShadow[6];
  deltaArray[index][6] = deltaShadow[7];
  deltaArray[index][7] = deltaShadow[8];  

counts[index] = (deltaArray[index][1] / deltaArray[index][0]);

  for (byte i = 0; i < 1; i++) 
  {
    currentTime = millis(); 
    elapsedTime[i] = currentTime - lastTime[i] ;
    //Looks at the last time each sensor was sampled
    if (elapsedTime[i]  >= deltaArray[i][0] && deltaArray[i][0] != 0) 
    {  //[i][0] is sample rate in milliseconds
      switch (i) 
      {
        case 0: //LDR
          actLDR = deltaArray[i][4];  //[i][4] is the type of actuator
          actuatorEnabled[actLDR] = 1;
          actuatorDuration[actLDR] = deltaArray[i][5]; //[i][5] is acutator duration
          actuatorReturn[actLDR] = LDRFunction(); //actuator return for actuator type is equal to return from tempFunc - 0 is off and 1 is off
          Serial.println(F("Actuator Return")); 
          Serial.println(actuatorReturn[i]);
          lastTime[i] = currentTime;
          break;
        case 1: //force
          actForce = deltaArray[i][4];  //[i][4] is the type of actuator
          actuatorEnabled[actForce] = 1;
          actuatorDuration[actForce] = deltaArray[i][5]; //[i][5] is acutator duration
          actuatorReturn[actForce] = ForceFunction(); //actuator return for actuator type is equal to return from tempFunc - 0 is off and 1 is off
          Serial.println(F("Actuator Return")); 
          Serial.println(actuatorReturn[i]);
          lastTime[i] = currentTime;
          break;
        case 2: //temperature
          actTemp = deltaArray[i][4];  //[i][4] is the type of actuator
          actuatorEnabled[actTemp] = 1;
          actuatorDuration[actTemp] = deltaArray[i][5]; //[i][5] is acutator duration
          actuatorReturn[actForce] = temperatureFunction(); //actuator return for actuator type is equal to return from tempFunc - 0 is off and 1 is off
          Serial.println(F("Actuator Return")); 
          Serial.println(actuatorReturn[i]);
          lastTime[i] = currentTime;
          break;
          
      }
    }
    else if (counts[i] == 0)
    {
      startTime[i] = currentTime;
      lastTime[i] = currentTime;
      counts[i]++; //Add a loop just to kick the counting into an upward state so it doesn't read teh time to current time
    } 
    else 
    {
     //lastTime[i] = currentTime;
      Serial.println(F("Am I in else?"));
    }
  }
  if(motor(previousMillis1, actuatorDuration[2]) && (actuatorEnabled[2] == 1)); //actuator function
  if(solenoid(previousMillis2, actuatorDuration[1]) && (actuatorEnabled[1] == 1)); //actuator function
  if(LED(previousMillis3, actuatorDuration[0]) && (actuatorEnabled[0] == 1)); //actuator function
  delay(5000); //remove when human is out of loop
};//end loop

int LDRFunction()
{
  //look up varaible type, maybe do uint32_t
  int rawLDR = analogRead(sensorLDR); //raw analog read
  Serial.print("LDR: ");
  Serial.println(rawLDR);
  //sends calibrated values to be logged in google sheets
  spreadsheet(rawLDR); 

  //Check thresholds and send e-mail if it crosses
  if (deltaArray[0][2] == 0) //if threshold for temp is == 0, then the threshold is min
  { 
    //if log value is less than min threshold, then e-mail
    if (rawLDR < deltaArray[0][3]) 
    { 
      email();
      Serial.println(F("min"));
    }
  }
  //else if log value is more than threshold, then e-mail
  else if(rawLDR > deltaArray[0][3]) 
  {
    email();
    Serial.println(F("max"));
  }

  counts[0]++;
  //Check if raw is above the actuator trigger values then it starts the actuator
  if ((deltaArray[0][6] == 0) && (rawLDR < deltaArray[0][7])) 
  {
    
    Serial.println(F("Min trigger Actuator"));
    return (1); //turn on
  } 
  else if ((deltaArray[0][6] == 1) && (rawLDR > deltaArray[0][7]))
{
    Serial.println(F("Max trigger Actuator"));
    return (1); //turn on
  } 
  else
  {
    return (0); //turn off
  }
}; // end LDR function


int temperatureFunction()
{
  //look up varaible type, maybe do uint32_t
  int rawTemperature = analogRead(sensorTemperature); //raw analog read
  Serial.print("temp: ");
  Serial.println(rawTemperature);
  //sends the raw and calibrated values to be logged in google sheets
  spreadsheet(rawTemperature); 

  //Check thresholds and send e-mail if it crosses
  if (deltaArray[0][2] == 0) //if threshold for temp is == 0, then the threshold is min
  { 
    //if log value is less than min threshold, then e-mail
    if (rawTemperature < deltaArray[0][3]) 
    { 
   //   email();
      Serial.println(F("min"));
    }
  }
  //else if log value is more than threshold, then e-mail
  else if(rawTemperature > deltaArray[0][3]) 
  {
 //   email();
    Serial.println(F("max"));
  }

  counts[0]++;
  //Check if raw is above the actuator trigger values then it starts the actuator
  if ((deltaArray[0][6] == 0) && (rawTemperature < deltaArray[0][7])) 
  {
    Serial.println(F("Min trigger Actuator"));
    return (1); //turn on
  } 
  else if ((deltaArray[0][6] == 1) && (rawTemperature > deltaArray[0][7]))
{
    Serial.println(F("Max trigger Actuator"));
    return (1); //turn on
  } 
  else
  {
    return (0); //turn off
  }
}; // end tempature function


int ForceFunction()
{
  int rawForce = analogRead(sensorForce); //raw analog read
  Serial.print("Force: ");
  Serial.println(rawForce);
  //sends calibrated values to be logged in google sheets
  spreadsheet(rawForce); 

  //Check thresholds and send e-mail if it crosses
  if (deltaArray[0][2] == 0) //if threshold for temp is == 0, then the threshold is min
  { 
    //if log value is less than min threshold, then e-mail
    if (rawForce < deltaArray[0][3]) 
    { 
      email();
      Serial.println(F("min"));
    }
  }
  //else if log value is more than threshold, then e-mail
  else if(rawForce > deltaArray[0][3]) 
  {
    email();
    Serial.println(F("max"));
  }

  counts[0]++;
  //Check if raw is above the actuator trigger values then it starts the actuator
  if ((deltaArray[0][6] == 0) && (rawForce < deltaArray[0][7])) 
  {
    
    Serial.println(F("Min trigger Actuator"));
    return (1); //turn on
  } 
  else if ((deltaArray[0][6] == 1) && (rawForce > deltaArray[0][7]))
{
    Serial.println(F("Max trigger Actuator"));
    return (1); //turn on
  } 
  else
  {
    return (0); //turn off
  }
}; // end Force function


bool motor(unsigned long &last_time, unsigned long period)
{
  unsigned long now = millis();



//  int digitalState;
  if(actuatorReturn[2] == 1)
  {
    actuatorReturn[2] = 2;
    last_time = now;   // Remember the time
    digitalWrite(motorPin, LOW);    // Update the motor
    return true;
  }
  else if ((now - last_time >= period))
  {
    //digitalState = LOW;  // Turn it off
    actuatorReturn[2] = 0;
    digitalWrite(motorPin, HIGH);  // Update the motor
    return false;
  }

};



bool solenoid(unsigned long &last_time, unsigned long period)
{
  unsigned long now = millis();

//  int digitalState;
  if(actuatorReturn[1] == 1)
  {
    actuatorReturn[1] = 2;
    last_time = now;   // Remember the time
    digitalWrite(solenoidPin, LOW);    // Update the solenoid
    return true;
  }
  else if ((now - last_time >= period))
  {
    //digitalState = LOW;  // Turn it off
    actuatorReturn[1] = 0;
    digitalWrite(solenoidPin, HIGH);  // Update the solenoid
    return false;
  }

};


bool LED(unsigned long &last_time, unsigned long period)
{
  unsigned long now = millis();

//  int digitalState;
  if(actuatorReturn[0] == 1)
  {
    actuatorReturn[0] = 2;
    last_time = now;   // Remember the time
    digitalWrite(ledPin, HIGH);    // Update the LED
    return true;
  }
  else if ((now - last_time >= period))
  {
    //digitalState = LOW;  // Turn it off
    actuatorReturn[0] = 0;
    digitalWrite(ledPin, LOW);  // Update the LED
    return false;
  }

};

void spreadsheet(int raw) 
{

  Serial.println(TEMBOO_ACCOUNT);
  int sensorRaw = raw;
  unsigned long now = millis();

  Serial.println(F("Appending value to spreadsheet..."));

  // we need a Process object to send a Choreo request to Temboo
  TembooChoreo AppendValuesChoreo;

  // invoke the Temboo client
  // NOTE that the client must be reinvoked and repopulated with
  // appropriate arguments each time its run() method is called.
  AppendValuesChoreo.begin();
  
  // set Temboo account credentials
  AppendValuesChoreo.setAccountName(TEMBOO_ACCOUNT);
  AppendValuesChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  AppendValuesChoreo.setAppKey(TEMBOO_APP_KEY);
  
  // identify the Temboo Library choreo to run (Google > Sheets > AppendValues)
  AppendValuesChoreo.setChoreo("/Library/Google/Sheets/AppendValues");
  
  // set the required Choreo inputs
  // see https://www.temboo.com/library/Library/Google/Sheets/AppendValues/ 
  // for complete details about the inputs for this Choreo
  
  // your Google application client ID
  AppendValuesChoreo.addInput("ClientID", CLIENT_ID);
  // your Google application client secret
  AppendValuesChoreo.addInput("ClientSecret", CLIENT_SECRET);
  // your Google OAuth refresh token
  AppendValuesChoreo.addInput("RefreshToken", REFRESH_TOKEN);

  // the ID of the spreadsheet you want to append to
  AppendValuesChoreo.addInput("SpreadsheetID", SPREADSHEET_ID);

  // convert the time and sensor values to a comma separated string
  String rowData = "[[\"" + String(now) + "\", \"" + String(sensorRaw) + "\"]]";
  Serial.print(F("RAW: "));
  Serial.println(rowData);

  // add the RowData input item
  AppendValuesChoreo.addInput("Values", rowData);

  // run the Choreo and wait for the results
  // The return code (returnCode) will indicate success or failure 
  unsigned int returnCode = AppendValuesChoreo.run();

  // return code of zero (0) means success
  if (returnCode == 0) 
  {
    Serial.print(F("Success! Appended "));
    Serial.println(rowData);
  } else 
  {
    // return code of anything other than zero means failure  
    // read and display any error messages
    while (AppendValuesChoreo.available()) 
    {
      char c = AppendValuesChoreo.read();
      Serial.print(c);
    }
  }
  AppendValuesChoreo.close();
}; // end spreadsheet 

void email()
{
  if (!attempted) 
  {
    Serial.println(F("Running SendAnEmail..."));
    TembooChoreo SendEmailChoreo;
    // invoke the Temboo client
    // NOTE that the client must be reinvoked, and repopulated with
    // appropriate arguments, each time its run() method is called.
    SendEmailChoreo.begin();

    // set Temboo account credentials
    SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
    SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);

    // identify the Temboo Library choreo to run (Google > Gmail > SendEmail)
    SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");


    // set the required choreo inputs
    // see https://www.temboo.com/library/Library/Google/Gmail/SendEmail/ 
    // for complete details about the inputs for this Choreo

    // the first input is your Gmail email address.     
    SendEmailChoreo.addInput("Username", GMAIL_USER_NAME);
    // next is your application specific password
    SendEmailChoreo.addInput("Password", GMAIL_APP_PASSWORD);
    // who to send the email to
    SendEmailChoreo.addInput("ToAddress", TO_EMAIL_ADDRESS);
    // then a subject line
    SendEmailChoreo.addInput("Subject", "ALERT: Greenhouse Temperature");

     // next comes the message body, the main content of the email   
    SendEmailChoreo.addInput("MessageBody", "Hey! The greenhouse is too cold!");

    // tell the Choreo to run and wait for the results. The 
    // return code (returnCode) will tell us whether the Temboo client 
    // was able to send our request to the Temboo servers
    unsigned int returnCode = SendEmailChoreo.run();

    // a return code of zero (0) means everything worked
    if (returnCode == 0) 
    {
      Serial.println(F("Success! Email sent!"));
    } 
    else 
    {
      // a non-zero return code means there was an error
      // read and print the error message
      while (SendEmailChoreo.available()) 
      {
        char c = SendEmailChoreo.read();
        Serial.print(c);
      }
    } 
    SendEmailChoreo.close();

    // set the flag showing we've tried
    attempted = true;
  }// end if attempted 
}; //edn email function 

