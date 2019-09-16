/*
  Using the SparkFun Qwiic 12 Bit ADC - 4 Channel ADS1015
  By: Pete Lewis, Original flex-glove library by: Andy England
  SparkFun Electronics
  Date: May 9, 2019
  License: This code is public domain but you can buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Please buy a board from SparkFun!
  https://www.sparkfun.com/products/15334

  This example shows how to output ADC values on one differential input between A0 and A1.
   at default gain setting of 1 (and 3.3V VCC), 0-2V will read 0-2047.
   anything greater than 2V will read 2047.

  Hardware Connections and initial setup:
  Plug in your controller board (e.g. Redboard Qwiic) into your computer with USB cable.
  Connect your Qwiic 12 Bit ADC board to your controller board via a qwiic cable.
  Connect your first voltage source to A0
  Connect your second voltage source to A1
  Select TOOLS>>BOARD>>"Arduino/Genuino Uno"
  Select TOOLS>>PORT>> "COM 3" (note, yours may be different)
  Click upload, and watch streaming data over serial monitor at 9600.
  It will show the voltage difference between A0 and A1 (also showing negative differences (if present).

*/

#include <SparkFun_ADS1015_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ADS1015
#include <Wire.h>

#include <Servo.h>
Servo myservo;  // create servo object to control a servo

uint16_t positionAdcReading[181] = {};
void sweepCalibrate(boolean debug = false);

ADS1015 adcSensor;

uint16_t maxReading;
uint16_t minReading;

uint16_t inputPrevious = 876; // for smoothing
uint16_t input = 876; // this is what I'm seeing on ADC for servo 500 position

void setup() {
  Wire.setClock(400000);
  Wire.begin();
  Serial.begin(115200);
  if (adcSensor.begin() == true)
  {
    Serial.println("Device found. I2C connections are good.");
  }
  else
  {
    Serial.println("Device not found. Check wiring.");
    while (1); // stall out forever
  }

  adcSensor.setGain(ADS1015_CONFIG_PGA_1); // PGA gain set to 4

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  //sweepCalibrate(true);
  //myservo.write(180);

  setAdcReadingsMap();
  //printAdcReadings();
  //while(1);

  //  cabilbrateInputStart();
}

void loop() {
  followForce();
  input = adcSensor.getSingleEnded(0); // default (i.e. no arguments) is A0 and A1
  // Optional "commented out" examples below show how to read differential readings between other pins
  // int16_t input = adcSensor.getDifferential(ADS1015_CONFIG_MUX_DIFF_P0_N3);
  // int16_t input = adcSensor.getDifferential(ADS1015_CONFIG_MUX_DIFF_P1_N3);
  // int16_t input = adcSensor.getDifferential(ADS1015_CONFIG_MUX_DIFF_P2_N3);

  //uint16_t input = getAverage(50);

  //  Serial.print(input);
  //  Serial.print(",");
  Serial.println(smoothData(input));
  //  Serial.print("\tm: ");
  //  Serial.print(minReading);
  //  Serial.print("\tM:");
  //  Serial.println(maxReading);
  delay(10); // avoid bogging up serial monitor
  inputPrevious = input;
}

uint16_t getAverage(int readings)
{
  uint16_t total = 0;
  for (int i = 0 ; i < readings ; i++)
  {
    total += adcSensor.getSingleEnded(0);
    setMaxMin(total);
    delayMicroseconds(10);
  }
  total /= readings;
  return total;
}


void setMaxMin(uint16_t total)
{
  if (total > maxReading) maxReading = total;
  if (total < minReading) minReading = total;
}

uint16_t smoothData(uint16_t input)
{
  if (input > (inputPrevious + 5)) return 876;
  else if (input < (inputPrevious - 5)) return 876;
  else return input;
}

void sweepCalibrate(boolean debug)
{
  myservo.write(0); // get to start if it was left elsewhere on previous use
  delay(1000);

  for (int pos = 0 ; pos <= 180 ; pos++ )
  {
    myservo.write(pos);
    delay(50);
    positionAdcReading[pos] = adcSensor.getSingleEnded(0);
    if (debug)
    {
      Serial.print(pos);
      Serial.print(", ");
      Serial.println(positionAdcReading[pos]);
    }
  }
}

void printAdcReadings()
{
  for (int pos = 0 ; pos <= 180 ; pos++ )
  {
    Serial.print(pos);
    Serial.print(", ");
    Serial.println(positionAdcReading[pos]);
  }
}

void setAdcReadingsMap()
{
  for (int pos = 0 ; pos <= 180 ; pos++ )
  {
    positionAdcReading[pos] = map(pos, 0 , 180 , 189 , 872);
  }
}

void followForce()
{
  int pos = 180;
  
  myservo.write(pos); // start position
  delay(1000);
  int previousReading = adcSensor.getSingleEnded(0);
  while (1)
  {
    int currentLocation = adcSensor.getSingleEnded(0);
    if (currentLocation > (previousReading + 40))
    {
      // ignore noise spikes
    }
    else
    {
      if (currentLocation > (previousReading + 1))
      {
        pos += 2;
      }
      else if (currentLocation < (previousReading - 1))
      {
        pos -= 2;
      }
      pos = constrain(pos, 0, 180);

      //pos = map(constrain(currentLocation, 189, 872), 189, 872, 0, 180);

      //      Serial.print(pos);
      //      Serial.print(", ");
      //      Serial.println(currentLocation);

      myservo.write(pos);
      Serial.print(pos);
      Serial.print(", ");
      Serial.println(currentLocation);
      previousReading = currentLocation;
    }
    
  }
}
