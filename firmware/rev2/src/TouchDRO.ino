/* Touch DRO
 *
 * Based on:
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 * http://www.pjrc.com/teensy/td_libs_FreqMeasure.html
 *

 */

#include <Encoder.h>
#include <Metro.h>
#include <FreqMeasure.h>
#include <Bounce.h>
#include <EEPROM.h>

// Frequency measure reserved pins
// For Teensy 3.x  3 & 4
// For Teensy LC  16 & 17

#DEFINE ZSET_PIN 5

#DEFINE X_A_PIN 11
#DEFINE X_B_PIN 12
#DEFINE Y_A_PIN 14
#DEFINE Y_B_PIN 15
#DEFINE Z_A_PIN 22
#DEFINE Z_B_PIN 23

#DEFINE BT_KEY 2

#DEFINE DEVICE_NAME "TouchDRO"

// Set up three variables for receiving encoder information
Encoder encoderX(X_A_PIN, X_B_PIN);
Encoder encoderY(Y_A_PIN, Y_B_PIN);
Encoder encoderZ(Z_A_PIN, Z_B_PIN);

// Periodic 'interrupt' in order to keep TouchDRO connection active
Metro keepAlive = Metro(100);

// Zero button (optional)
Bounce zerobutton = Bounce(ZSET_PIN, 10);

void setup() {
  // initialize the console output
  Serial.begin(9600);

  // initialize the bluetooth output
  Serial1.begin(9600);

  // set the bluetooth module name
  delay(250);
  pinMode(BT_KEY,OUTPUT);
  delay(250);
  digitalWrite(BT_KEY, HIGH);
  delay(250);
  Serial1.print("AT+NAME"+DEVICE_NAME);
  delay(250);
  digitalWrite(BT_KEY, LOW);

  // initialize the frequency library (tach)
  FreqMeasure.begin();

  // initialize the zero-set input
  pinMode(ZSET_PIN, INPUT_PULLUP);
}


// variables for current x, y & z positions
long positionX = -999;
long positionY = -999;
long positionZ = -999;

// variables for tach
double rpm = 0;
int samples = 0;
float frequency = 0;

void loop() {

   if (FreqMeasure.available()) {
    // average every 30 frequency samples
    rpm = rpm + FreqMeasure.read();
    samples = samples + 1;
    if (samples > 30) {
      frequency = FreqMeasure.countToFrequency(rpm / samples) * 60;
      rpm = 0;
      samples = 0;
    }
  }

 // zero-set the encoder values
 if (zerobutton.update()) {
    if (zerobutton.fallingEdge()) {
        encoderX.write(0);
        encoderY.write(0);
        encoderZ.write(0);
    }
 }

  // read the current position of the encoders
  long _positionX = encoderX.read();
  long _positionY = encoderY.read();
  long _positionZ = encoderZ.read();

  // if any of the positions have changed or periodic interupt triggered
  if( (_positionX != positionX) ||
      (_positionY != positionY) ||
      (_positionZ != positionZ) ||
        keepAlive.check() == 1) {

        // set new values to current values
        positionX = _positionX;
        positionY = _positionY;
        positionZ = _positionZ;

        // send current position to TouchDRO app via bluetooth serial interface
        Serial1.print(F("x")); Serial1.print((long)positionX); Serial1.print(F(";"));
        Serial1.print(F("y")); Serial1.print((long)positionY); Serial1.print(F(";"));
        Serial1.print(F("z")); Serial1.print((long)positionZ); Serial1.print(F(";"));

        // uncomment to debug via console
        //Serial.print(F("x")); Serial.print((long)positionX); Serial.print(F(";"));
        //Serial.print(F("y")); Serial.print((long)positionY); Serial.print(F(";"));

        // send current tach reading to TouchDRO app via bluetooth serial interface
        Serial1.print(F("t")); Serial1.print((long)frequency); Serial1.print(F(";"));

        // reset the periodic interupt
        keepAlive.reset();

      }

}