/*
Arduino Nano Every LANC to USB-serial interface by Novgorod
Version 1.0
Github: 

Based on Arduino code by L. Rosén: https://projecthub.arduino.cc/L-Rosen/9b5d02d4-f885-41ee-bba7-6b18d3dfe47d
LANC protocol: http://www.boehmel.de/lanc.htm
*/

// LANC in: pin 3, LANC out: pin 4
// Use ATMEGA4809 native registers (direct I/O) on Arduino Nano Every!

#define cmdPinON (VPORTC.OUT = B01000000)    // Set digtal pin 4 (PC6)
#define cmdPinOFF (VPORTC.OUT = B00000000)   // Reset digtal pin 4 (PC6)
#define ledON (VPORTE.OUT = B00000100)       // Set LED pin 13 (PE2)
#define ledOFF (VPORTE.OUT = B00000000)      // Reset LED pin 13 (PE2)
#define lancPinREAD (VPORTF.IN &= B00100000) // Read pin 3 (PF5)
#define lancRepeats 4                        // Repeat LANC command (4 frames default)

int bitDura = 104;           // Duration of one LANC bit in microseconds, 104µs -> 9600bps
int halfbitDura = 52;        // Half bit duration
int repeats = 0;
byte lancByte = 0;
byte strPointer = 0;
char inChar;
char inString[5];
char outString[17];
boolean strComplete = false;
boolean lancCmd[16];
unsigned long time;

void setup() {
 VPORTC.DIR |= B01000000;    // Config cmdPin as output (high)
 VPORTF.DIR &= B11011111;    // Config lancPin as input (low)
 VPORTE.DIR |= B00000100;    // Config ledPin as output (high)
 cmdPinOFF;                  // Reset LANC control pin (LANC line becomes high)
 Serial.begin(115200);       // Start serial port  
 Serial.println("Arduino LANC to USB-serial interface v1.0");
}


void loop() {

 time = micros();                                // Wait for lancPin to be high for at least 5ms
 while (micros()-time<5000) {
   if(!lancPinREAD) { time = micros(); }
   }

 noInterrupts();                                 // Disable interrupts for time-critical jitter-free bit-banging

 while (lancPinREAD) {  }                        // Wait for the falling edge indicating the begin of the start bit
 
 ledON;                                          // LED indicator on = LANC message start
     
 for (int bytenr = 0 ; bytenr<8 ; bytenr++) {    // Process 8-byte frame
  delayMicroseconds(bitDura-4);                  // Wait start bit duration at the beginning of a byte
  for (int bitnr = 0 ; bitnr<8 ; bitnr++) {      // Process 8 bits
    if (bytenr<2 && repeats) {                   // Output data (if available) during the first two bytes
      if (lancCmd[bitnr+bytenr*8]) { cmdPinON; } 
      else { cmdPinOFF; }
    }
    delayMicroseconds(halfbitDura-3);
    bitWrite(lancByte, bitnr, !lancPinREAD);     // Read data line during middle of bit and write the bit to lancByte (LANC is inverted!)
    delayMicroseconds(halfbitDura);
  }
 cmdPinOFF;
 delayMicroseconds(halfbitDura-10);              // Make sure to be in the stop bit before waiting for next byte; small delay adjust for sending serial data
 Serial.write(lancByte);                         // Send lancByte through serial port while waiting for next start bit
 if (bytenr<7) { while (lancPinREAD) {  } }      // Wait as long as the LANC line is high until the next start bit EXCEPT at end of frame
 }
 
 Serial.write(10);                               // Write line feed (0xA) to serial port after frame

 if(repeats>0) { repeats--; }                    // If a LANC command was sent in this frame, decrease the send "queue"; nothing is sent if repeats is 0

 ledOFF;                                         // LED indicator on = LANC message end
 interrupts();                                   // Re-enable interrupts

 while (Serial.available()) {                    // Read serial port input beween frames
   inChar = (char)Serial.read();                 // Get the new byte
   inString[strPointer++] = inChar;                                 // Add it to the input string
   if ((inChar == '\n') || (inChar == '\r') || (strPointer > 4)) {  // If new character is a line feed, carriage return or 4 bytes were received, prepare LANC message
     strPointer = 0;
     if(hexchartobitarray()) { repeats = lancRepeats; }             // Convert input string and set LANC commands "queue" (number of frames to repeat command)
     for (int i=0 ; i<5 ; i++) { inString[i] = 0; }                 // Reset input string (optional cleanup)
   }
 }

}


boolean hexchartobitarray() {
 // This function converts the hex char LANC command and fills the lancCmd array with the bits in LSB-first order

 int byte1, byte2;
 
 for (int i=0 ; i<4 ; i++ ) {
  if (!(isHexadecimalDigit(inString[i]))) { return 0; }
 }

 byte1 = (hexchartoint(inString[0]) << 4) + hexchartoint(inString[1]);
 byte2 = (hexchartoint(inString[2]) << 4) + hexchartoint(inString[3]);

 for (int i=0 ; i<8 ; i++) {  lancCmd[i] = bitRead(byte1,i); }
 for (int i=0 ; i<8 ; i++) {  lancCmd[i + 8] = bitRead(byte2,i); }
 
 return 1;
}


int hexchartoint(char hexchar) {
 switch (hexchar) {
   case 'F':
     return 15;
     break;
   case 'f':
     return 15;
     break;
   case 'E':
     return 14;
     break;
   case 'e':
     return 14;
     break;
   case 'D':
     return 13;    
     break;
   case 'd':
     return 13;    
     break;
   case 'C':
     return 12;
     break;
   case 'c':
     return 12;
     break;
   case 'B':
     return 11;
     break;
   case 'b':
     return 11;
     break;
   case 'A':
     return 10;
     break;
   case 'a':
     return 10;
     break;
   default:
     return (int) (hexchar - 48);
    break;
 }
}