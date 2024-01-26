You have to upload the "sleep" and "wake" sketches to 2 atmega328p chips. You can use a bootloader
or flash the program directly with an ISP programmer. The "sleep" module consumes 0.1mA(mean) depending on the WoR settings. This can be many years on battery power.
The clock must be 8MHz as ATmega CANNOT run (reliably) @16MHz and 3.3Volts
For wiring look at the source code of the sketches.
