# Player Script to have input for the tank duel game

## Requirements

- Python 3
- Visual Studio Code (or any Python-capable editor)

## Getting Started

1. Download the Python script.
2. Open the file in Visual Studio Code.
3. Press **Run** to start the script.
4. The game will guide you through the available options once it starts.

No additional configuration is required to begin.

## Configuration Options

### IP Address

- The default IP address is `192.168.0.113`.
- This can be changed within the script interface.
- After entering a new IP address, press **Update IP** to apply the change.

### Player Selection

- You can choose to play as **Player 1** or **Player 2**.
- The UDP ports are hard-coded in the Vitis project and are selected automatically based on the player choice.

### Keyboard Layout

- Multiple keyboard layouts are supported.
- This option exists because the game was developed by Belgian developers.
- Switching the keyboard layout will automatically update the key bindings to match the selected layout.

## Controls and Gameplay

- Movement keys can be held down to move the tank continuously.
- Shooting is limited by a fixed delay to prevent continuous firing.

<img src="C:\Github\TankDuelGame\resources\pictures\playerGUI.jpg" alt="gui" style="zoom:100%;" />

## Notes

- This script is intentionally minimal and focused on player input.
- All network communication relies on predefined UDP ports configured in the Vitis project.

