// http://www.myledhoop.com
// LED Hula Hoop
// Wireless remote control with nRF24L01 module
// The 'hoop' is controlled by the "Arduino Remote Control"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// nRF24L01 radio
// RF24 radio(_cepin, _cspin);
RF24 radio(48, 49);
// _cepin  The pin attached to Chip Enable on the RF module
// _cspin  The pin attached to Chip Select
const uint64_t pipes[2] = {
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// Timer
#define runEvery(t) for (static typeof(t) _lasttime;(typeof(t))((typeof(t))millis() - _lasttime) > (t);_lasttime += (t))

// More output pins
// const int redLed = 34; // low battery led
// const int buzzer = 35; // buzzer
// const int orangeLeds = 38; // orange leds
// const int headlight = 39; // white leds
// const int blueLed = 40;   // blue led = wireless link down

// Structs for communication
typedef struct {
  // int X;
  // int Y;
  // int Z;
  int A;
  // int B;
  // int C;
  // int D;
}
struct1_t;

// typedef struct {
//   float motors;
//   float batRover;
// }
// struct2_t;
// struct2_t rover;

typedef struct {
  int led_mode;       // Just ideas for now.
  int is_alive;       // Just ideas for now.
}
struct2_t;

struct1_t remote;
struct2_t hoop;

// Other stuff...
// byte current;
// byte battery;
// byte orangeLedState;
unsigned long startTime;
unsigned long endTime;
int time;

void setup()
{
  Serial.begin(115200);   // Need to see why this number is bigger than the remote's baud rate...
  // Setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_MAX) ; 
  radio.setDataRate(RF24_1MBPS);
  //radio.setDataRate(RF24_250KBPS);
  //radio.enableDynamicPayloads() ;
  //radio.setAutoAck(true);
  //radio.setPayloadSize(14);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();

}

void loop(void)
{
  startTime = millis();

  // Radio 
  runEvery(20)     // running this code too often leads to dropped packets
  {
    radio.stopListening();
    bool ok = radio.write( &rover, sizeof(rover) );
    radio.startListening();

    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      //if (millis() - started_waiting_at > 1+(radio.getMaxTimeout()/1000) )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    if ( timeout )          // no wireless connection
    {
      // digitalWrite(blueLed, HIGH);
      stopped();
      Serial.println("Failed, response timed out.");
    }
    else
    {
      // digitalWrite(blueLed, LOW);
      radio.read( &remote, sizeof(remote) );
    }
  }
  // end of radio stuff

  endTime = millis();
  time = endTime - startTime;

}

void stopped()
{
  // Need to add here code that turns off the hoop.
}

// Lots of work to do here!




