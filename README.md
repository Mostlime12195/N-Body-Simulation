# C++ N-Body Simulation
This N-Body Simulation is compiled in C++ and Raylib. It does not use the Barnes-Hut model, so it can only handle several hundred, maybe a thousand, particles at a smooth 60 fps.

Feel free to use this repo for your own use, but don't expect the best quality.

## Features:
- Adjustable simulation area (must change the minX, maxX, minY, maxY, variables within the code)
- Pleasing visuals (dynamically coloured particles, grid with dark borders)
- Inelastic collisions (modifiable via code)
- Particles coloured based on their their kinetic + potential energy (Blue -> Red)
- Spawn particles anywhere
- Adjustable number of particles to spawn on start
- Useful stats

![image](Assets/thumbnail.png)

# Important notes

- This project does NOT use the Barnes-Hut model at the moment. I understand that this simulation would be tremendously quicker and would be able to simulate millions more particles with the Barnes-Hut model, but I am not that talented at this time. I will experiment with the Barnes-Hut model, but I will not implement it here until I feel ready.
- This is an amateur project. I'm a 16 year old with no formal education in programming, I have only ever been taught through the internet in my free-time. Do not expect quality.
- Particles in the centre of clumps may start glitching out, this is normal. I understand what the issue is, but given that the simulation runs mostly fine without the fix, I'm not keen on patching it at the moment unless I have a burst of motivation. For your knowledge, the fix *should* be to increase the amount of collision checks per frame, decreasing the FPS, but making far more accurate collision interactions.

# How to run

## For anything other than Notepad++:
I did not create this project for tools like VSCode, so you would have to look for Raylib's own documentation (or others' documentation on the internet) to figure out how to use Raylib on VSCode, or any other IDE/tool.

## For Notepad++:

### If you use Raylib's given Notepad++:
You can find Raylib's Notepad++ inside of the raylib installation directory. Within the raylib installation dir, you can find notepad++.exe under the 'npp' folder.

#### Steps:
1. Open main.cpp (with Raylib's version of NP++) 
2. Press f6 
3. You'll see a list of commands. One of them should be 'SET CC=gcc'. Replace 'gcc' with 'g++'. If it's already like that, continue to the next step.
4. Click on 'OK'.
5. You're done!

#### Notes:
- 'gcc' must be replaced with 'g++' because the project does not work with a C compiler, and thus you must compile it with a C++ compiler instead.

### If you use any other edition of Notepad++:
I have not compiled this project under any other version of NP++, so I suggest that you either use Raylib's NP++, or you search for help elsewhere online.


# How to use

- Left click anywhere to spawn a particle.
- Left click and drag to spawn a particle with a direction and velocity (direction will be where you move your mouse).
- Hold right click to pan the camera.
- You can also use the arrow keys to pan the camera.
- Use the scroll wheel to zoom in and out with the camera.