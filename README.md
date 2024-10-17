# Endless World Generation ( Using Level Streaming )

[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## Description

This project load dynamic level streaming around the player. As the player moves through the environment, levels are loaded and unloaded in real-time, ensuring optimal performance. This system allows for large, open-world experiences where only the nearby areas are active, similar to how Minecraft dynamically manages chunks.

## Project Setup
To set up the project, follow these steps:

Navigate to the project folder and find the **EndlessWorld.uproject file**.

Right-click on the **EndlessWorld.uproject** file and select **Generate Visual Studio Project Files**. This will create all the necessary files for compiling the project.

Once the Visual Studio project files are generated, you can either:
Open the solution in Visual Studio for code editing and building.
Or, double-click the .uproject file to open the project directly in Unreal Engine.


## Endless World Setup
The *EndlessWorld** component is attached to the PlayerController. It dynamically handles the generation of the world around the player.

If you want to add your own levels, you can easily do so that by adding levels to the **Levels** arrays in the **World Generation** Category.

![image](https://github.com/user-attachments/assets/f41ec5a1-7978-4236-b5ca-64a68ff23d2f)

![image](https://github.com/user-attachments/assets/c6209831-3f47-40fe-88bd-99bce0844af1)
