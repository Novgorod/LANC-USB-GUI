# Arduino LANC to USB with GUI

This project emulates a simple service remote for LANC enabled camcorders on a PC using an Arduino Nano Every to translate between a LANC device and the Arduino's USB-serial interface as well as a simple GUI resembling the functions of a service remote for status monitoring and control.

## Background

As magnetic tape video media and recording/playback devices from the last millennium and early 2000s are becoming increasingly obscure historic artefacts, the issue of long-term conservation before those tapes turn into dust is now more pressing than ever. Functioning hardware from that era is becoming rare and professional service tools are collecting dust in some retired VCR technicians' basements never to be seen or used again. Sony's ancient LANC protocol enables professional functions such as automation, monitoring and low-level service mode access on most of their camcorders, which is a valuable tool for any professional or even enthusiast hobbyist conservation efforts, giving the old hardware one last and very important purpose before it can be retired to the museum forever. Although the tech has been long forgotten, enough documentation is preserved to rebuild the necessary tools from scratch with modern inexpensive parts.

## Hardware

This project uses an [Arduino Nano Every](https://store.arduino.cc/products/arduino-nano-every) (ATMega4809) since it is currently the cheapest Arduino with an onbaord USB interface. The previous-generation Nano 3.x (ATmega328) should also work with the code but requires adjusting the register names for the digital read/write pins.

### Wiring

![Image](https://user-images.githubusercontent.com/13183195/238089976-93a7ba03-fa65-449f-9aee-c33160807e74.png)

This simple wiring diagram is used in most 5V microcontroller-based LANC implementations. Note that the Arduino is USB-powered. The 5.1V Zener diode protects the Arduino because the LANC supply/signaling voltage can go to up to +8V as per spec and can be omitted (at your own risk!) if your camera uses not more than +5V. An even simpler 1-wire implementation (LANC line directly to a digital pin) is also possible with a 5V device, but generally the LANC supply voltage should be used for signaling (via the transistor) and the code uses dedicated input and output pins (though it can be easily adapted for 1-wire communication).

Example wiring:

![Image](https://user-images.githubusercontent.com/13183195/238100367-774f6af9-bf39-4e46-b4cd-33d24a9e1dfc.png) ![Image](https://user-images.githubusercontent.com/13183195/238100414-08e91210-accd-443c-8317-0ae796bee540.png)

## Arduino code

Use the [Arduino IDE](https://www.arduino.cc/en/software) to compile and deploy the sketch `arduino_lanc_nano-every.ino` to the Arduino Nano Every. Make sure you select the Nano Every board and use native ATMega4809 registers, **not** ATmega328 register emulation (Tools menu). The correct USB serial port driver should be automatically installed through the IDE. By default, pin 3 is LANC in and pin 4 is LANC out (as in the schematic), which can be changed in the #define section in the code (see the Arduino Nano Every documentation for port assignments of the pins). Pin 13 (parallel to the onboard LED) is high during a LANC frame and can be used as a trigger/sync.

The code follows the [protocol documentation](http://www.boehmel.de/lanc.htm) and is loosely based on the Arduino Nano LANC implementation by [L. Rosén](https://projecthub.arduino.cc/L-Rosen/9b5d02d4-f885-41ee-bba7-6b18d3dfe47d) with some important changes:

- The original code only returned a single LANC frame after a command has been sent. The new code reads every LANC frame (at 50Hz for PAL or 60Hz for NTSC) and sends it as raw data (8 bytes followed by a line feed) to the serial port. The terminal program has to convert the code into hex characters to be human readable if required. The data is sent over the serial port at 115200kbps as soon as one byte is received ftom the LANC line.
- The background functions provided by the Arduino transparently to the user (such as timers) make use of interrupts and introduce unpredictable ~10µs jitter a few percent of the time. Interrupts are disabled in the new code during the critical bit-banging period, elimitating all timing jitter.
- The LANC command from the user is read over the serial port between the LANC frames. The commands are 2 bytes formatted as hex characters (i.e., 4 ASCII characters representing hex digits) followed by a line feed.

## Remote emulator GUI

![Image](https://user-images.githubusercontent.com/13183195/238098564-2eec6d55-677c-4667-8e20-94b751ad3fd1.png)

The GUI software `Sony LANC remote.vi` mimics some basic functions of a wired remote (such as the RM-95) including EEPROM read/write functions of a service remote and monitoring of camera status and time codes. The implementation follows roughly the available [documentation](http://www.boehmel.de/lanc.htm) and advanced commands can be sent through the drop-down menu or a command code can be entered manually as 4 hexadecimal characters (2 bytes). The LANC interface (including EEPROM functions) is completely exposed, so use at your own risk! More functions can be explicitly implemented in the future if the documentation becomes available.

### Requirements

The GUI software is written in National Instruments Labview 2022, which requires the development environment (including NI-VISA) to view and edit the source code. A [compiled executable](https://github.com/Novgorod/LANC-USB-GUI/releases/tag/LANC) is available for Windows (under Releases), which requires the free [Labview Runtime Engine 2022](https://download.ni.com/support/nipkg/products/ni-l/ni-labview-2022-runtime-engine-x86/22.3/online/ni-labview-2022-runtime-engine-x86_22.3_online.exe) or later.

### Usage

Run the .vi or the compiled executable with the Arduino connected to the PC and the LANC device. Select the Arduino's COM port in the prompt at startup and click Ok. Once the GUI is connected and showing the camera status, you can send commands with the buttons, from the drop-down menu or as manual hexadecimal input.
