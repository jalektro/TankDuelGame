# TankDuelGame

TankDuelGame is a two-player tank combat game inspired by the classic *Wii Play – Tanks!* game.  
The game features two tanks starting in opposite corners of the map (top-left and bottom-right) and supports multiple platforms, including PC and an embedded FPGA-based system.

---

## Features

- Two-player competitive gameplay
- Multiple platform support (PC and Embedded FPGA)
- Real-time player input
- HDMI-based video output for embedded version
- Modular hardware/software design

---

## PC Version

The PC version allows the game to be played directly on a desktop or laptop environment.

### Getting Started

Please follow the instructions provided in the PC-specific documentation:

[PC Manual](./Game_PC/README.md)

---

## Embedded Version

The embedded version runs on an FPGA-based platform and uses HDMI output for display. Player input is provided via external Python scripts.

Working video : [Youtube](https://youtu.be/zoKySX43RjM)

### Hardware Block Design

The hardware design is created using **Vivado 2025.1**.  
The complete block design is shown below:

![Block Design](/resources/pictures/blockdesign.jpg)

A detailed PDF of the block design can be found here:

[HDMI Block Design PDF](./Hardware_vivado/HDMI/HDMI_RTOS.pdf)

From this block design, an **XSA file** is generated and imported into **Vitis Unified IDE 2025.1**.

### Software Development

- **Vitis Unified IDE Version:** 2025.1  
- **Vitis Project Location:**  
  [Vitis Project](./Vitis_project)

### Player Input

The game is controlled by two players using Python scripts that send commands to the embedded system.

- Player control scripts and usage instructions:  
  [Player Script Documentation](./player_script/README.md)

---

## Requirements

### Software

- Vivado 2025.1
- Vitis Unified IDE 2025.1
- Python 3.x (for player input scripts)

### Hardware

- PYNQ-Z2 FPGA board (Zynq-7020)
- HDMI-compatible display
- Host PC or laptop for running Python input scripts

---

## Installation and Execution

1. Open the hardware design in Vivado 2025.1.
2. Generate the XSA file from the block design.
3. Import the XSA into Vitis Unified IDE 2025.1.
4. Build and program the FPGA.
5. Connect the HDMI display to the board.
6. Run the Python player scripts from a host PC.
7. Start debugging or execution in Vitis and begin gameplay.

## Easy start 

To have an easy set up to the game, you can also just download the [boot.bin](./Vitis_project/boot) file. copy and paste this on an SD-Card. 

The game will start from SD-card directly. 

## Repository Structure

```
TankDuelGame/
├── Game_PC/ # PC version of the game
├── Hardware_vivado/ # Vivado hardware design
├── Vitis_project/ # Embedded software project
├── player_script/ # Python scripts for player input
├── pictures/ # Images and diagrams
└── README.md
```

