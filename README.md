# Tetris - Purdue Proton

This project implements a multiplayer Tetris variant on the Raspberry Pi 2350B, also found on the Raspberry Pi Pico 2.

## Overview

Battle Tetris runs on a 64×64 RGB LED matrix and supports head-to-head multiplayer using UART communication between two Proton boards.  
Gameplay uses an NES controller for input, includes custom audio playback over PWM, and features a full UI rendered with a custom bitmap font.

## Core Features

- **LED Matrix Graphics**  
  Rendering to a 64×64 Adafruit matrix using PIO and two framebuffers. Display driver written from scratch, art drawn by hand.

- **NES Controller Input**  
  GPIO-based polling and detection of an original NES controller.

- **Multiplayer Support**  
  Board-to-board communication over UART via a USB-A cable for real-time competitive gameplay. Advanced sync logic ensures game states don't desynchronize.

- **Audio System**
  PWM-based playback of music and sound effects through a filtered line-level output, with double-buffered DMA streaming.

## Team Roles

- **Andrei** – game logic, graphics, art, music, display driver, input driver, multiplayer hardware and software
- **Raahil** – display hardware
- **Amy** – audio hardware and driver
- **Davis** – input hardware

## References

- [Tetris Wiki: Guidelines Overview](https://tetris.wiki/Tetris_Guideline)
- [Leaked 2009 Tetris internal design guidelines](https://dn720004.ca.archive.org/0/items/2009-tetris-variant-concepts_202201/2009%20Tetris%20Design%20Guideline.pdf)
