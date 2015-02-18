/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "LPD8806.h"
#include <avr/sleep.h>
#include "hearts.h"
// #include <printf.h>  // Printf is used for debug.
//
// Initialization & Hardware Configuration
//
int demoCaseNumber = 0;

int intervalDemo = 10000;     // Looking like this is 10 seconds.

int powerPin = 4;
int upModePin = 3;
int upColorPin = 2;

int upModeButtonState = HIGH;
int upModeButtonCycles = 0;
int upColorButtonState = HIGH;
int upColorButtonCycles = 0;

int CYCLES_DEBOUNCE = 2; //check the button for X ticks to see if it is bouncing
int MAX_COLORS = 19;
int MAX_MODES = 17;
int MAX_STRIPES = 5;

unsigned long tick = 0;

int mode = 1;
int color = 1;

long previousMillis = 0;
long interval = 500;

uint16_t i, j, x, y;
uint32_t c, d;

// Since data is on 11 and clock is on 13, we can use hardware SPI
// This will now go through the wireless which is acting on the SPI bus,
// ultimately allowing us to send over the SPI bus wirelessly.
LPD8806 strip = LPD8806(160);
//
// Set up nRF24L01 radio on SPI bus plus pins 7 & 8.
RF24 radio(7,8);
// Sets the role of this unit in hardware.
// @HARDWARE: Connect to GND to be the 'pong' receiver.
// @HARDWARE: Leave open to be the 'ping' transmitter.

const int role_pin = 5;     // @TODO: Need to figure out which pin would be
                            // best for our usage.
//
// Topology.
//
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
    0xF0F0F0F0E1LL,
    0xF0F0F0F0D2LL
};
//
// Role management.
//
// Set up role.
// This sketch uses the same software for all the nodes in this system.
// Doing so greatly simplifies testing.
// The hardware itself specifies which node it is.
//
// This is done through the 'role_pin.'
//
// The various roles supported by this sketch.
typedef enum {
    role_ping_out = 1,
    role_pong_back
} role_e;
// The debug-friendly names of those roles.
const char* role_friendly_name[] = {
    "invalid",
    "Ping out",
    "Pong back"
};
// The role of the current running sketch.
role_e role;
//
// Payload.
//
const int min_payload_size = 4;
const int max_payload_size = 32;
const int payload_size_increments_by = 1;
int next_payload_size = min_payload_size;
char receive_payload[max_payload_size + 1]; // +1 to allow room for a
                                            // terminating NULL char.
// Set the first variable to the NUMBER of pixels. 32 = 32 pixels in a row
// The LED strips are 32 LEDs per meter but you can extend/cut the strip

void ISR_Wake() {
    detachInterrupt(0);
    detachInterrupt(1);
}

void blackout() {
    for(int i = 0; i < strip.numPixels() + 1; i++) {
        strip.setPixelColor(i, strip.Color(0,0,0));
    }
    strip.show();
}


void triggerSleep() {
    blackout();

    attachInterrupt(0, ISR_Wake, LOW); //pin 2
    attachInterrupt(1, ISR_Wake, LOW); //pin 3

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    //sleeping, until rudely interrupted
    sleep_disable();
}

void triggerModeUp() {
    ++mode;
    blackout();
}

void triggerColorUp() {
    color++;
    blackout();
}

void handleButtons() {
    if(digitalRead(powerPin) == LOW) {
        triggerSleep();
    }
    // software debounce
    if(digitalRead(upModePin) != upModeButtonState) {
        upModeButtonCycles++;
        if(upModeButtonCycles > CYCLES_DEBOUNCE) {
            upModeButtonCycles = 0;
            upModeButtonState = digitalRead(upModePin);
            if(upModeButtonState == LOW) {
                triggerModeUp();
            }
        }
    }
    // software debounce
    if(digitalRead(upColorPin) != upColorButtonState) {
        upColorButtonCycles++;
        if(upColorButtonCycles > CYCLES_DEBOUNCE) {
            upColorButtonCycles = 0;
            upColorButtonState = digitalRead(upColorPin);
            if(upColorButtonState == LOW) {
                triggerColorUp();
            }
        }
    }
}

void handleStrip(int case_num) {
    // switch(mode%MAX_MODES) {
    switch (case_num){
    case 0: //solid
    c = GetColor(color % MAX_COLORS);
    for(i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    break;
    case 1:  //every other led 
    c = GetColor((tick % 3 + color) % MAX_COLORS);
    for(i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    break;
    case 2:
    if(tick % 50 == 0) {
        c = GetColor(color % MAX_COLORS);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    if(tick % 50 == 25) {
        c = strip.Color(0, 0, 0);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    break;
    case 3:  //strobe 2 color
    if(tick % 30 == 0) {
        c = GetColor(color % MAX_COLORS);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    if(tick % 30 == 15) {
        c = GetColor(color % MAX_COLORS + 2);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    break; 
    case 4:  //strobe 3 color
    if(tick % 60 == 20) {
        c = GetColor(color % MAX_COLORS);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    if(tick % 60 == 40) {
        c = GetColor(color % MAX_COLORS + random(5));
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    if(tick % 60 == 60) {
        c = GetColor(color % MAX_COLORS + random(2));
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
        }
    }
    break; 
    case 5: //chasers
        d = (color / MAX_COLORS) % MAX_STRIPES + 1; //chaser
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels() / d);
        for(i = 0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels() / d) == j) {
                strip.setPixelColor(i, c);
            }
            else {
                strip.setPixelColor(i, strip.Color(0, 0, 0));
            }
        }
        break;
    case 6: //chasers + statics
        d = (color / MAX_COLORS) % MAX_STRIPES + 1; //chaser
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels() / d);
        for(i = 0; i < strip.numPixels(); i++) {
            x = i % (strip.numPixels() / d);
            if((x == j) || (x == 0)) {
                strip.setPixelColor(i, c);
            }
            else {
                strip.setPixelColor(i, strip.Color(0, 0, 0));
            }
        }
        break;
    case 7: //fuckin' rainbows
    j = tick % 384;
    for(i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + j) % 384));
    }
    break;
    case 8:  //rainbow dither
        d = (color / MAX_COLORS) % MAX_STRIPES + 1; //chaser
        c = tick % 384;       
        j = tick % (strip.numPixels() / d);
        for(i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));        //first pixel
            strip.setPixelColor(i + 1, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));    //second pixel
            strip.setPixelColor(i + 2, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));    //third pixel
            strip.setPixelColor(i + 32, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));        //first pixel
            strip.setPixelColor(i + 33, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));    //second pixel
            strip.setPixelColor(i + 34, Wheel(((i * 384 / strip.numPixels() * (color % MAX_COLORS)) + c) % 384));
            strip.show();
            strip.setPixelColor(i, 0);
            strip.setPixelColor(i + 32, 0);

        }
        break;
    case 9: //rainbow chaser
        //d = random(3); //chaser
    d = 3;
    x = tick % 384;
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels()/d) == j) {
                strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + x) % 384));        //first pixel
                strip.setPixelColor(i + 1, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + x) % 384));    //second pixel
                strip.setPixelColor(i + 2, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + x) % 384));
            }
            else {
                strip.setPixelColor(i, strip.Color(0,0,0));
            }
        }
        break;
        /*case 8:  //rainbow chaser (3 pixel chaser)
        d = (color / MAX_COLORS) % MAX_STRIPES + 1; //chaser
        c = tick % 384;       
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
        if(i % (strip.numPixels()/d) == c) {
        strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));        //first pixel
        strip.setPixelColor(i + 1, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));    //second pixel
        strip.setPixelColor(i + 2, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));    //third pixel
        }
        else {
        strip.setPixelColor(i, strip.Color(0,0,0));
        }
        }
        break;*/
        /*case 10: //hands POV
        j = tick % 150;
        d = hands[j];
        //green                     //orange
        c = (j < 14)?GetColor((color+2)%MAX_COLORS):GetColor((color+3)%MAX_COLORS);
        for(i=0;i<32;i++) {
        //adding 32 to the index makes it appear on the side opposite the controller
        if(d & 0x00000001) {
        strip.setPixelColor(i+32, c);
        }
        else {
        strip.setPixelColor(i+32, strip.Color(0,0,0));
        }
        d >>= 1;
        }
        break;
        case 11: //tanner POV
        j = tick % 150;
        d = tanner[j];
        //green                     //orange
        c = GetColor(color % MAX_COLORS);
        for(i=0;i<32;i++) {
        //adding 32 to the index makes it appear on the side opposite the controller
        if(d & 0x00000001) {
        strip.setPixelColor(i+32, c);
        }
        else {
        strip.setPixelColor(i+32, strip.Color(0,0,0));
        }
        d >>= 1;
        }
        break;
        case 12: //tanner2 POV
        j = tick % 150;
        d = tanner2[j];
        //green                     //orange
        c = GetColor(color % MAX_COLORS);
        for(i=0;i<32;i++) {
        //adding 32 to the index makes it appear on the side opposite the controller
        if(d & 0x00000001) {
        strip.setPixelColor(i+32, c);
        }
        else {
        strip.setPixelColor(i+32, strip.Color(0,0,0));
        }
        d >>= 1;
        }
        break;*/
    case 10:  //hearts, entire strip
    j = tick % 149;
    d = hearts[j];
        //green                     //orange
    c = GetColor(color % MAX_COLORS);
    for(i=0;i<32;i++) {
            //adding 32 to the index makes it appear on the side opposite the controller
        if(d & 0x00000001) {
            strip.setPixelColor(i, c);
            strip.setPixelColor(i+32, c);
            strip.setPixelColor(i+64, c);
        }
        else {
            strip.setPixelColor(i, strip.Color(0,0,0));
            strip.setPixelColor(i+32, strip.Color(0,0,0));
            strip.setPixelColor(i+64, strip.Color(0,0,0));
        }
        d >>= 1;
    }
    break;  
    case 11:  //stripes
    c = GetColor(color % MAX_COLORS);
    d = GetColor((color +1) % MAX_COLORS);
    for(i = 0; i < 4; i++) {
        strip.setPixelColor(i, c);
        strip.setPixelColor(i+9, c);
        strip.setPixelColor(i+18, c);
        strip.setPixelColor(i+27, c);
        strip.setPixelColor(i+36, c);
        strip.setPixelColor(i+45, c);
        strip.setPixelColor(i+54, c);
        strip.setPixelColor(i+63, c);
        strip.setPixelColor(i+72, c);
        strip.setPixelColor(i+81, c);
        strip.setPixelColor(i+89, c);
    }
    for(i = 4; i < 9; i++) {
        strip.setPixelColor(i, d);
        strip.setPixelColor(i+9, d);
        strip.setPixelColor(i+18, d);
        strip.setPixelColor(i+27, d);
        strip.setPixelColor(i+36, d);
        strip.setPixelColor(i+45, d);
        strip.setPixelColor(i+54, d);
        strip.setPixelColor(i+63, d);
        strip.setPixelColor(i+72, d);
        strip.setPixelColor(i+81, d);
        strip.setPixelColor(i+89, d);
    }
    break;
    case 12:  //stripes rainbow color
    c = GetColor(color % MAX_COLORS);
    j = tick % 384;
    for(i = 0; i < 4; i++) {
        strip.setPixelColor(i, c);
        strip.setPixelColor(i+9, c);
        strip.setPixelColor(i+18, c);
        strip.setPixelColor(i+27, c);
        strip.setPixelColor(i+36, c);
        strip.setPixelColor(i+45, c);
        strip.setPixelColor(i+54, c);
        strip.setPixelColor(i+63, c);
        strip.setPixelColor(i+72, c);
        strip.setPixelColor(i+81, c);
        strip.setPixelColor(i+89, c);
    }
    for(i = 4; i < 9; i++) {
        strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+9, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+18, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+27, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+36, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+45, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+54, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+63, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+72, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+81, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
        strip.setPixelColor(i+89, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + j) % 384));
    }
    break;
    case 13:  //stripes changing color
    c = GetColor(color % MAX_COLORS);
        //d = GetColor((color + random(11) % MAX_COLORS));
    j = tick % 384;
    for(i = 0; i < 4; i++) {
        strip.setPixelColor(i, c);
        strip.setPixelColor(i+9, c);
        strip.setPixelColor(i+18, c);
        strip.setPixelColor(i+27, c);
        strip.setPixelColor(i+36, c);
        strip.setPixelColor(i+45, c);
        strip.setPixelColor(i+54, c);
        strip.setPixelColor(i+63, c);
        strip.setPixelColor(i+72, c);
        strip.setPixelColor(i+81, c);
        strip.setPixelColor(i+89, c);
    }
    for(i = 4; i < 9; i++) {
        d = GetColor((color + random(11)) % MAX_COLORS);
        strip.setPixelColor(i, d);
        strip.setPixelColor(i+9, d);
        strip.setPixelColor(i+18, d);
        strip.setPixelColor(i+27, d);
        strip.setPixelColor(i+36, d);
        strip.setPixelColor(i+45, d);
        strip.setPixelColor(i+54, d);
        strip.setPixelColor(i+63, d);
        strip.setPixelColor(i+72, d);
        strip.setPixelColor(i+81, d);
        strip.setPixelColor(i+89, d);
    }
    break;
    case 14:
    for(i = 0; i < 4; i++) {
        d = GetColor((color + random(3)) % MAX_COLORS);
        strip.setPixelColor(i, d);
        strip.setPixelColor(i+9, d);
        strip.setPixelColor(i+18, d);
        strip.setPixelColor(i+27, d);
        strip.setPixelColor(i+36, d);
        strip.setPixelColor(i+45, d);
        strip.setPixelColor(i+54, d);
        strip.setPixelColor(i+63, d);
        strip.setPixelColor(i+72, d);
        strip.setPixelColor(i+81, d);
        strip.setPixelColor(i+89, d);
    }
    for(i = 4; i < 9; i++) {
        d = GetColor((color + random(6)) % MAX_COLORS);
        strip.setPixelColor(i, d);
        strip.setPixelColor(i+9, d);
        strip.setPixelColor(i+18, d);
        strip.setPixelColor(i+27, d);
        strip.setPixelColor(i+36, d);
        strip.setPixelColor(i+45, d);
        strip.setPixelColor(i+54, d);
        strip.setPixelColor(i+63, d);
        strip.setPixelColor(i+72, d);
        strip.setPixelColor(i+81, d);
        strip.setPixelColor(i+89, d);
    }
    break;
    case 15: //solid
    c = GetColor(color%MAX_COLORS);
    for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    break;
    case 16:  //rainbow chaser (3 pixel chaser) --------------DAVIS
        d = (color / MAX_COLORS) % MAX_STRIPES +3; //chaser
        //c = tick % 384;       
        //c = tick % 96;
        c = tick % 80;
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels()/d) == c) {
                strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));        //first pixel
                strip.setPixelColor(i + 1, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));    //second pixel
                strip.setPixelColor(i + 2, Wheel(((i * 384 / strip.numPixels() * (color%MAX_COLORS)) + c) % 384));    //third pixel
            }
            else {
                strip.setPixelColor(i, strip.Color(0,0,0));
            }
        }
        break;
    case 17: //3 spaced chasers --------------DAVIS
        d = (color / MAX_COLORS) % MAX_STRIPES + 3; //chaser
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels()/d) == j) {
                strip.setPixelColor(i, c);
            }
            else {
                strip.setPixelColor(i, strip.Color(0,0,0));
            }
        }
        break;
    case 18: //3 spaced chasers, with filled in gaps --------------DAVIS
        d = (color / MAX_COLORS) % MAX_STRIPES + 3; //chaser
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels()/d) == j) {
                strip.setPixelColor(i, strip.Color(127,0,127));
            }
            else {
                strip.setPixelColor(i, strip.Color(0,0,0));
            }
        }
        break;     
    case 19: //3 spaced chasers, with filled in gaps --------------DAVIS
        d = (color / MAX_COLORS) % MAX_STRIPES + 4; //chaser
        c = GetColor(color % MAX_COLORS);       //color
        j = tick % (strip.numPixels()/d);
        for(i=0; i < strip.numPixels(); i++) {
            if(i % (strip.numPixels()/d) == j) {
                strip.setPixelColor(i, strip.Color(0,95,121));
            }
            else {
                strip.setPixelColor(i, strip.Color(50,10,100));
            }
        }
        break; 
//  case 20:  //random static colors
//      unsigned long currentMillis = millis();
//      if(currentMillis - previousMillis > interval) {
//          previousMillis = currentMillis;
//          for(int i = 0; i < strip.numPixels(); i++) {
//              c = GetColor((color % MAX_COLORS) + random(6));
//              strip.setPixelColor(i, c);
//          }
//          break;
//          strip.show();
//      }
//      break;
    }
    strip.setPixelColor(strip.numPixels()-1, strip.Color(0,0,0)); //set that last LED off because it overlaps
    strip.show();
}


void setup() {  // The previous was 'void setup(void)' which is unnecessary as leaving it empty implies 'void'
    //
    // Role.
    //
    // Set up the role pin.
    pinMode(role_pin, INPUT);
    digitalWrite(role_pin, HIGH);
    delay(20);  // Just to get a solid reading on the role pin.

    // Start up the LED strip
    strip.begin();

    pinMode(powerPin, INPUT);    // declare pushbutton as input
    pinMode(upModePin, INPUT);    // declare pushbutton as input
    pinMode(upColorPin, INPUT);    // declare pushbutton as input
    digitalWrite(powerPin, HIGH);
    digitalWrite(upModePin, HIGH);
    digitalWrite(upColorPin, HIGH);

    // Read the address pin, establish our role.
    if(digitalRead(role_pin))
        role = role_ping_out;
    else
        role = role_pong_back;
    //
    // Print preamble.
    //
    Serial.begin(115200);
    //printf_begin();   // Printf is used for debug.
  
    Serial.println(F("RF24/examples/pingpair_dyn/"));
    Serial.print(F("ROLE: "));
    Serial.println(role_friendly_name[role]);
    //
    // Setup and configure rf radio.
    //
    radio.begin();
    // Enable dynamic payloads.
    radio.enableDynamicPayloads();
    // Optionally, increase the delay between retries & # of retries.
    radio.setRetries(5,15);
    //
    // Open pipes to other nodes for communication.
    //
    // This simple sketch opens two pipes for these two nodes to communicate
    // back and forth.
    // Open 'our' pipe for writing.
    // Open the 'other' pipe for reading, in position #1 (we can have up to 5
    // pipes open for reading).
    if (role == role_ping_out) {
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
    } else {
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1,pipes[0]);
    }
    //
    // Start listening.
    //
    radio.startListening();
    //
    // Dump the configuration of the rf unit for debugging.
    //
    radio.printDetails();
}
void loop() {   // The previous was 'void setup(void)' which is unnecessary as leaving it empty implies 'void'

    tick++;
    handleStrip(demoLoopSeconds());
    handleButtons();

    //
    // Ping out role.
    // Repeatedly send the current time.
    //
    if (role == role_ping_out) {
        // The payload will always be the same, what will change is how much
        // of it we send.
        static char send_payload[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ789012";

        // First, stop listening so we can talk.
        radio.stopListening();
        // Take the time, and send it.
        // This will block until complete.
        // Serial.print(F("Now sending length "));
        // Serial.println(next_payload_size);
        radio.write(send_payload, next_payload_size);
        // Now, continue listening.
        radio.startListening();
        // Wait here until we get a response, or timeout.
        unsigned long started_waiting_at = millis();
        bool timeout = false;
        while (!radio.available() && !timeout)
            if (millis() - started_waiting_at > 500)
                timeout = true;
        // Describe the results.
        if (timeout) {
            Serial.println(F("Failed, response timed out."));
        } else {
            // Grab the response, compare, and send to debugging spew.
            uint8_t len = radio.getDynamicPayloadSize();

            // If a corrupt dynamic payload is received, it will be flushed.
            if(!len){
                return; 
            }
        
            radio.read(receive_payload, len);
            // Put a zero at the end for easy printing.
            receive_payload[len] = 0;
            // Spew it.
            // Serial.print(F("Got response size="));
            // Serial.print(len);
            // Serial.print(F(" value="));
            // Serial.println(receive_payload);
        }
    
        // Update size for next time.
        next_payload_size += payload_size_increments_by;
        if (next_payload_size > max_payload_size)
            next_payload_size = min_payload_size;
        // Try again one second later.
        delay(100);
    }
    //
    // Pong back role.
    // Receive each packet, dump it out, and send it back.
    //
    if (role == role_pong_back) {
        // If there is data ready.
        while (radio.available()) {
            // Fetch the payload, and see if this was the last one.
            uint8_t len = radio.getDynamicPayloadSize();

            // If a corrupt dynamic payload is received, it will be flushed.
            if(!len){
                continue;
            }
      
            radio.read(receive_payload, len);
            // Put a zero at the end for easy printing.
            receive_payload[len] = 0;
            // Spew it.
            Serial.print(F("Got response size="));
            Serial.print(len);
            Serial.print(F(" value="));
            Serial.println(receive_payload);
            // First, stop listening so we can talk.
            radio.stopListening();
            // Send the final one back.
            radio.write(receive_payload, len);
            Serial.println(F("Sent response."));
            // Now, resume listening so we catch the next packets.
            radio.startListening();
        }
    }
}

int demoLoopSeconds() {
    unsigned long currentMillis = millis();
    if((currentMillis - previousMillis) > intervalDemo) {
        previousMillis = currentMillis;
        demoCaseNumber++;
    }
    return (demoCaseNumber % 20);
}



    /* Helper functions */

    //Input a value 0 to 384 to get a color value.
    //The colours are a transition r - g - b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
    byte r, g, b;
    switch(WheelPos / 128)
    {
        case 0:
        r = 127 - WheelPos % 128; // red down
        g = WheelPos % 128;       // green up
        b = 0;                    // blue off
        break;
        case 1:
        g = 127 - WheelPos % 128; // green down
        b = WheelPos % 128;       // blue up
        r = 0;                    // red off
        break;
        case 2:
        b = 127 - WheelPos % 128; // blue down
        r = WheelPos % 128;       // red up
        g = 0;                    // green off
        break;
    }
    return(strip.Color(r,g,b));
}


uint32_t GetColor(int c)
{
    switch(c) {
        case 0:
        return strip.Color(127,0,0);  //red
        case 1:
        return strip.Color(127,0,60);  
        case 2:
        return strip.Color(127,0,127); 
        case 3:  //orange
        return strip.Color(127,60,0);
        case 4:  //yellow
        return strip.Color(127,127,0);
        case 5:
        return strip.Color(127,127,60);
        case 6:
        return strip.Color(60,127,0);
        case 7:
        return strip.Color(60,127,127);
        case 8:
        return strip.Color(60,60,127);
        case 9:
        return strip.Color(60,90,60);
        case 10:
        return strip.Color(60,60,0);
        case 11:
        return strip.Color(0,0,127);  //blue
        case 12:
        return strip.Color(0,60,127);
        case 13:
        return strip.Color(0,127,127);
        case 14:
        return strip.Color(0,127,0);  //green
        case 15:
        return strip.Color(127,30,10);
        case 16:
        return strip.Color(25,80,100);
        case 17:
        return strip.Color(127,127,127);  //White
        case 18:
        int r, g, b;
        r = random(0, 50);
        g = random(40, 90);
        b = random(80, 128);
        return strip.Color(r,g,b);
        default:
        return strip.Color(0,0,0);
    }
}

// vim:cin:ai:sts=2 sw=2 ft=cpp
