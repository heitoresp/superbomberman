Mini Bomberman - C Implementation with Raylib

Description
Mini Bomberman is a simplified implementation of the classic Bomberman game, developed in C using the Raylib graphics library. This project was created as a practical assignment for the Algorithms and Programming course at UFRGS's Informatics Institute.

The player controls the Bomberman character in a maze of blocks, strategically placing bombs to eliminate enemies, destroy blocks to find hidden exits, and collect power-ups to enhance abilities.

Key Features
🎮 Procedural level generation

💣 Complete bomb and explosion system

⚡ Bomb count and explosion range power-ups

👾 Enemy AI with basic behavior

📈 Progressive scoring and level system

🖥️ Main menu with multiple options

💾 Save/load system between levels

🗺️ Custom map loading

🔄 Smooth character movement

💥 Visual explosion effects

🎯 Win/lose conditions

Prerequisites
C compiler (GCC recommended)

Raylib (version 4.5 or higher)

Windows, Linux, or macOS

Build and Run
Clone the repository:

bash
git clone https://github.com/heitoresp/superbomberman
cd mini-bomberman
Compile the project:

bash
# Linux/macOS
gcc -o mini_bomberman main.c -lraylib -lm

# Windows
gcc -o mini_bomberman.exe main.c -lraylib -lopengl32 -lgdi32 -lwinmm
Run the game:

bash
# Linux/macOS
./mini_bomberman

# Windows
mini_bomberman.exe
Gameplay
Controls
Movement: W/A/S/D or ARROW KEYS

Plant bomb: SPACEBAR

Menu: Number keys (1, 2, 3, 4) and ESC

Objective
Eliminate all enemies using bombs

Find the exit hidden under a destructible wall

Advance to the next level

Game Elements
Element	Symbol	Description
Player	🟥	Character you control

Enemies	🟪	Touch them and you die

Bombs	💣	Explode after 3 seconds

Explosions	🔥	Destroy blocks and kill enemies

Destructible Walls	🧱	Hide power-ups and exits

Indestructible Walls	⬛	Permanent barriers

Bomb Power-up	💙	Increases bomb capacity

Range Power-up	💛	Increases explosion range

Exit	🟩	Complete level after killing enemies

Project Structure
plaintext
mini-bomberman/
├── src/
│   └── main.c             # Main source code
├── assets/                # Resource directory (images/sounds)
├── maps/                  # Sample custom maps
├── save.bin               # Generated save file
├── LICENSE                # MIT License file
└── README.md              # This file
Game Mechanics Explained
Level Generation:

Grid-based maps (15x15 tiles)

Procedurally placed walls and items

Increasing difficulty with each level

Bomb System:

Timed explosions (3-second fuse)

Chain reactions

Directional blast propagation

Character Movement:

Grid-based with smooth interpolation

Collision detection with walls and bombs

Power-up System:

Persistent upgrades between levels

Hidden under destructible blocks

Enemy AI:

Random movement patterns

Simple obstacle avoidance

Distance-based spawning from player

Developers
Your Name

Partner's Name (if applicable)

License
This project is licensed under the MIT License - see the LICENSE file for details.

Developed for educational purposes at UFRGS - 2025/1
