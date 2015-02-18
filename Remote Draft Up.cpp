// Stuff to use for remote (possibly)

#include <SPI.h>	// Need to look up exactly what this is but whatevs for now.
#include "nRF24L01.h"
#include "RF24.h"
// #include <Wire.h>	Not used now, but if we used an I2C communicating LCD display, this library is used to handle the I2C protocol communication.
// #include <LiquidCrystal_I2C.h>	// The LCD display over I2C library.
#include <Bounce.h>	// Helps with push buttons to avoid "bouncing" aka multiple low high reads (resulting in it reading multiple pushes) when the button was only pressed one time by the user.
// The bounce may just be for pushing down the joystick on the dude's remote... Not something we need to worry about.

#define runEvery(t) for (static typeof(t) _lasttime;(typeof(t))((typeof(t))millis() - _lasttime) > (t);_lasttime += (t))

typedef struct{
  // int X;
  // int Y;
  // int Z;
  int A;
  // int B;
  // int C;
  // int D;
}
struct1_t;

typedef struct {
  int led_mode;       // Just ideas for now.
  int is_alive;       // Just ideas for now.
}
struct2_t;

struct1_t remote;
struct2_t hoop;
// struct2_t rover;

// nRF24L01 radio
// RF24 radio(_cepin, _cspin);
RF24 radio(8,9);
// _cepin  The pin attached to Chip Enable on the RF module
// _cspin  The pin attached to Chip Select
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

const int aButton = 4;
// const int bButton = 5;

void setup()
{
  Serial.begin(57600);
  pinMode(aButton , INPUT);
  digitalWrite(aButton, HIGH);
  // pinMode(bButton , INPUT);
  // digitalWrite(bButton, HIGH);
  radio.begin();
  radio.setPALevel( RF24_PA_MAX ) ; 
  //radio.setDataRate(RF24_250KBPS);
  //radio.setAutoAck( true ) ;
  radio.setDataRate(RF24_1MBPS);
  //radio.setPayloadSize(14);
  //radio.enableDynamicPayloads() ;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
}

void loop(void)
{
  // analogRead(A3);
  // batRemote = analogRead(A3) / 102.3;
  analogRead(A0);
  remote.X = analogRead(A0);
  // analogRead(A1);
  // remote.Y = analogRead(A1);
  // remote.A = digitalRead(clawOpen);    
  // remote.B = digitalRead(clawClose);
  // remote.C = digitalRead(topButton);  
  // remote.D = digitalRead(bottomButton);
  
  // Note: this may be for the joystick z axis push down and may not be necessary in our usage. 
  // if ( bouncerz.update() ) {
  //   if ( bouncerz.read() == LOW) {
  //     if ( remote.Z == HIGH ) {
  //       remote.Z = LOW;
  //     } 
  //     else {
  //       remote.Z = HIGH;
  //     }
  //   }
  // }

  // radio stuff
  if ( radio.available() )
  {
    bool done = false;
    while (!done)
    {
      // done = radio.read( &rover, sizeof(rover) );
      done = radio.read( &hoop, sizeof(hoop) );
    }
    radio.stopListening();
    radio.write( &remote, sizeof(remote) );
    radio.startListening();
  }
  // end of radio stuff

  //lcd stuff
  // runEvery(200)
  // {
  // lcd.setCursor(8,0);
  // lcd.print(rover.motors);
  // lcd.setCursor(13,2);
  // lcd.print(batRemote);
  // lcd.setCursor(12,3);
  // lcd.print(rover.batRover);
  // }
  // end of lcd stuff
}
