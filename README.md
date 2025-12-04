# Tetris - Purdue Proton

This project implements a multiplayer Tetris variant on the Purdue Proton development board, which uses the same 2350b microcontroller as the Raspberry Pi Pico 2.

## Overview

Battle Tetris runs on a 64×64 RGB LED matrix and supports head-to-head multiplayer using UART communication between two Proton boards.  
Gameplay uses an NES controller for input, includes custom audio playback over PWM, and features a full UI rendered with a custom bitmap font.

## Core Features

- **LED Matrix Graphics**  
  Rendering to a 64×64 Adafruit matrix using GPIO and DMA-driven framebuffer updates.

- **NES Controller Input**  
  GPIO-based polling and detection of an original NES controller.

- **Multiplayer Support**  
  Board-to-board communication over UART via a USB-A cable for real-time competitive gameplay.

- **Audio System**
  PWM-based playback of music and sound effects through a filtered line-level output, with double-buffered DMA streaming.

## Team Roles

- **Andrei** – game logic and coordination
- **Raahil** – graphics engine, enclosure design  
- **Amy** – audio system and UI  
- **Davis** – multiplayer communication and input hardware

## References

- [Tetris Wiki: Guidelines Overview](https://tetris.wiki/Tetris_Guideline)
- [Leaked 2009 Tetris internal design guidelines](https://dn720004.ca.archive.org/0/items/2009-tetris-variant-concepts_202201/2009%20Tetris%20Design%20Guideline.pdf)
