# avr_vhf
Channel 3 VHF Broadcasting with an AVR

Broadcast analog video, Channel 3 with your ATTiny85!

I strongly recommend using an analog TV, as it will be much easier to debug your signal.

This is set up for North America NTSC Channel 3 Only.

Do not add an amplifier to this device.  You will likely violate broadcast restrictions.

# Setup

*Plug two wires into PB3 and PB4 of your AVR, PB3 should be shorter than PB4.
*Turn on an analog TV set to channel 3.  If you don't see static, it's unlikely you'll be able to do this project.
*Program your AVR, then change the OSCCAL value until you find the correct frequency for broadcast on your particular AVR.  All AVRs have different clocks.

You now have an AVR Multimeter, Pin PB2 contains the analog input.
