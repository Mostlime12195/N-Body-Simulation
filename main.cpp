#include "raylib.h"
#include "C:/raylib/raylib/src/raymath.h"

#include <vector>
#include <thread>

#include <iostream>

#include <mutex>


struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Color color = BLACK;
    float potEnergy; // Potential Energy
    float kinEnergy; // Kinetic Energy
};

void CreateParticle(std::vector<Particle> &particles, Vector2 velocity, Vector2 position);
void SetGravitationalPull(Particle &p1, Particle &p2, float force);
void DrawGrid(float minX, float minY, float maxX, float maxY);
void SpawnParticleBatch(std::vector<Particle> &parts, float minX, float maxX, float minY, float maxY, float stepSizeX, float stepSizeY);
void UpdateParticles(std::vector<Particle> &parts, int start, int end, float minX, float minY, float maxX, float maxY, float gravConst);
void ComputeForces(std::vector<Particle> &parts, int start, int end, float gravConst, bool collisionsEnabled);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    // Settings:
    const int screenWidth = 1400;
    const int screenHeight = 800;

    float minX = -2000.0f; // Minimum X-coordinate (left boundary)
    float maxX = 2000.0f;  // Maximum X-coordinate (right boundary)
    float minY = -2000.0f; // Minimum Y-coordinate (top boundary)
    float maxY = 2000.0f;  // Maximum Y-coordinate (bottom boundary)

    bool spawn_on_load = true; // Load particles

    int stepSizeX = 150;
    int stepSizeY = 150;

    const int threadCount = 7;

    bool collisionsEnabled = true;

    // Other variables:

    float gravConst = 3200.0f;

    std::vector<Particle> parts;
    parts.reserve(1000); // Preallocate space for up to 1,000 particles, so that it does not have to dynamically, this is a performance improvement.

    Camera2D camera = {0};
    camera.zoom = 0.30f;
    camera.target = {-3000, -1500};

    Vector2 init_pos;
    Vector2 fin_pos;

    std::vector<std::thread> threads;

    if (spawn_on_load)
    {
        SpawnParticleBatch(parts, minX, maxX, minY, maxY, stepSizeX, stepSizeY);
    }

    InitWindow(screenWidth, screenHeight, "N-Body Simulation");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    const Texture2D texture = LoadTexture("Assets/particle.png");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here

        threads.clear();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            init_pos = GetScreenToWorld2D(GetMousePosition(), camera);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            fin_pos = GetScreenToWorld2D(GetMousePosition(), camera);

            float dist = Vector2Distance(init_pos, fin_pos);
            float angle = atan2(fin_pos.y - init_pos.y, fin_pos.x - init_pos.x);

            Vector2 velocity = Vector2Zero();

            if (dist > 0.25f)
            {
                velocity.x = dist * cos(angle);
                velocity.y = dist * sin(angle);
            }

            CreateParticle(parts, velocity, init_pos);
        }

        if (GetMouseWheelMove() != 0)
        {

            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.offset = GetMousePosition();

            camera.target = mouseWorldPos;

            // Update zoom level
            camera.zoom += GetMouseWheelMove() / 20.0f;
            camera.zoom = Clamp(camera.zoom, 0.10f, 5.0f);

            float scaleFactor = 1.0f + (0.25f * fabsf(GetMouseWheelMove()));

            if (GetMouseWheelMove() < 0)
                scaleFactor = 1.0f / scaleFactor;

            camera.zoom = Clamp(camera.zoom * scaleFactor, 0.125f, 64.0f);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            gravConst = gravConst / 2.0f;
        }

        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            gravConst *= 2.0f;
        }

        if (IsKeyPressed(KEY_P))
        {
            (spawn_on_load) ? spawn_on_load = false : spawn_on_load = true;
        }

        if (IsKeyPressed(KEY_R))
        {
            parts.clear();
            if (spawn_on_load)
            {
                SpawnParticleBatch(parts, minX, maxX, minY, maxY, stepSizeX, stepSizeY);
            }
        }

        if (IsKeyPressed(KEY_C))
        {
            (collisionsEnabled) ? collisionsEnabled = false : collisionsEnabled = true;
        }

        gravConst = Clamp(gravConst, 0.0f, 409600.0f);

        camera.target.x += (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * 30;
        camera.target.y += (IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * 30;

        int particlesPerThread = parts.size() / threadCount;
        int remainingParticles = parts.size() % threadCount;
        
        for (int t = 0; t < threadCount; t++)
        {
            int start = t * particlesPerThread + std::min(t, remainingParticles);
            int end = (t + 1) * particlesPerThread + std::min(t + 1, remainingParticles);

            
            threads.emplace_back(ComputeForces, std::ref(parts), start, end, gravConst, collisionsEnabled);
        }
        for (auto &t : threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        } 
        
        UpdateParticles(parts, 0, parts.size(), minX, minY, maxX, maxY, gravConst);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        // Draw Grid, duh
        DrawGrid(minX, minY, maxX, maxY);

        
        for (Particle &p : parts)
        {
            DrawTextureEx(texture, Vector2SubtractValue(p.position, 10), 0.0f, 1.0f, p.color);
        }
        
        EndMode2D();

        DrawText("N-Body Simulation |", 20, 10, 20, BLACK);
        DrawFPS(220, 10);

        DrawText("Camera Position:", 20, 50, 20, BLACK);
        DrawText(TextFormat("(%.0f, %.0f)", camera.target.x, camera.target.y), 250, 50, 20, DARKBLUE);
        DrawText("Hold", 20, 70, 20, GRAY);
        DrawText("Right Click", 70, 70, 20, RED);
        DrawText("or use the", 185, 70, 20, GRAY);
        DrawText("Arrow Keys", 300, 70, 20, RED);
        DrawText("to pan the camera", 425, 70, 20, GRAY);

        DrawText("Camera Zoom:", 20, 110, 20, BLACK);
        DrawText(TextFormat("%.2f", camera.zoom), 250, 110, 20, DARKBLUE);
        DrawText("Use the", 20, 130, 20, GRAY);
        DrawText("Mouse Wheel", 108, 130, 20, RED);
        DrawText("to zoom in/out", 245, 130, 20, GRAY);

        DrawText("Particles:", 20, 170, 20, BLACK);
        DrawText(TextFormat("%i", parts.size()), 250, 170, 20, DARKBLUE);
        DrawText("Left Click", 20, 190, 20, RED);
        DrawText("to spawn particles (Hold to set velocity and direction)", 130, 190, 20, GRAY);

        DrawText("Strength of Gravity:", 20, 230, 20, BLACK);
        DrawText(TextFormat("%.1f", gravConst), 250, 230, 20, DARKBLUE);
        DrawText("Press", 20, 250, 20, GRAY);
        DrawText("]", 90, 250, 20, RED);
        DrawText("to double Gravity", 105, 250, 20, GRAY);
        DrawText("Press", 20, 270, 20, GRAY);
        DrawText("[", 90, 270, 20, RED);
        DrawText("to halve Gravity", 105, 270, 20, GRAY);

        DrawText("Press", 20, 310, 20, GRAY);
        DrawText("R", 90, 310, 20, RED);
        DrawText("to reset the simulation.", 110, 310, 20, GRAY);

        DrawText("Spawn bundle of particles", 20, 350, 20, BLACK);
        DrawText("on start/restart:", 20, 370, 20, BLACK);
        if (spawn_on_load == true)
        {
            DrawText("ENABLED", 250, 370, 20, GREEN);
        }
        else
        {
            DrawText("DISABLED", 250, 370, 20, RED);
        }
        DrawText("Press", 20, 390, 20, GRAY);
        DrawText("P", 90, 390, 20, RED);
        DrawText("to toggle spawn_on_load", 110, 390, 20, GRAY);

        DrawText("Collisions enabled:", 20, 430, 20, BLACK);
        if (collisionsEnabled == true)
        {
            DrawText("ENABLED", 250, 430, 20, GREEN);
        }
        else
        {
            DrawText("DISABLED", 250, 430, 20, RED);
        }
        DrawText("Press", 20, 450, 20, GRAY);
        DrawText("C", 90, 450, 20, RED);
        DrawText("to toggle collisions", 110, 450, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void CreateParticle(std::vector<Particle> &particles, Vector2 velocity, Vector2 position)
{
    Particle p;
    p.velocity = velocity;
    p.position = position;

    particles.push_back(p);
}

void SetGravitationalPull(Particle &p1, Particle &p2, float force)
{
    float angle = atan2(p2.position.y - p1.position.y, p2.position.x - p1.position.x);

    p1.velocity.x += force * cos(angle);
    p1.velocity.y += force * sin(angle);
    //p2.velocity.x -= force * cos(angle);
    //p2.velocity.y -= force * sin(angle);
}

void DrawGrid(float minX, float minY, float maxX, float maxY)
{
    for (float y = minY; y <= maxY; y += 100)
    {
        if (y == minY or y == maxY)
        {
            DrawLine(minX, y, maxX, y, DARKGRAY);
            continue;
        }
        DrawLine(minX, y, maxX, y, LIGHTGRAY); // Vertical lines across the play area
    }

    for (float x = minX; x <= maxX; x += 100)
    {
        if (x == minX or x == maxX)
        {
            DrawLine(x, minY, x, maxY, DARKGRAY);
            continue;
        }
        DrawLine(x, minY, x, maxY, LIGHTGRAY); // Vertical lines
    }
}

void SpawnParticleBatch(std::vector<Particle> &parts, float minX, float maxX, float minY, float maxY, float stepSizeX, float stepSizeY)
{
    for (int x = minX + stepSizeX; x < maxX; x += stepSizeX)
    {
        for (int y = minY + stepSizeY; y < maxY; y += stepSizeY)
        {
            CreateParticle(parts, Vector2Zero(), {(float)x, (float)y});
        }
    }
}

void UpdateParticles(std::vector<Particle> &parts, int start, int end, float minX, float minY, float maxX, float maxY, float gravConst)
{
    for (int i = start; i < end; i++)
    {
        Particle &p1 = parts[i];

        // X boundaries
        if (p1.position.x < minX + 10)
        {
            p1.position.x = minX + 10;      // Stop or reflect the particle at the left boundary
            p1.velocity.x = -p1.velocity.x; // Reflect velocity if you want bouncing effect
        }
        else if (p1.position.x > maxX - 10)
        {
            p1.position.x = maxX - 10;      // Stop or reflect the particle at the right boundary
            p1.velocity.x = -p1.velocity.x; // Reflect velocity if you want bouncing effect
        }

        // Y boundaries
        if (p1.position.y < minY + 10)
        {
            p1.position.y = minY + 10;      // Stop or reflect the particle at the top boundary
            p1.velocity.y = -p1.velocity.y; // Reflect velocity if you want bouncing effect
        }
        else if (p1.position.y > maxY - 10)
        {
            p1.position.y = maxY - 10;      // Stop or reflect the particle at the bottom boundary
            p1.velocity.y = -p1.velocity.y; // Reflect velocity if you want bouncing effect
        }

        float normalizedEnergy = Normalize((p1.potEnergy * 1800 / gravConst) + (p1.kinEnergy / 1500), 0, 200.0f);
        normalizedEnergy = Clamp(normalizedEnergy, 0.0f, 1.0f);

        p1.potEnergy = 0.0f; // Resets it for the next frame.

        p1.color = ColorLerp({0U, 12U, 255U, 255U}, {255U, 12U, 0U, 225U}, normalizedEnergy);

        // Update position after resolving collisions and applying forces
        p1.position.x += p1.velocity.x * GetFrameTime();
        p1.position.y += p1.velocity.y * GetFrameTime();
    }
}

void ComputeForces(std::vector<Particle> &parts, int start, int end, float gravConst, bool collisionsEnabled)
{
    for (int i = start; i < end; i++)
    {
        Particle &p1 = parts[i];

        p1.kinEnergy = 0.5 * pow(Vector2Length(p1.velocity), 2);

        for (int d = 0; d < parts.size(); d++)
        {
            if (i == d)
            {
                continue;
            }
            

            Particle &p2 = parts[d];

            float dist = Vector2Distance(p1.position, p2.position);
            if (dist < 20.0f and collisionsEnabled)
            {
                Vector2 normal = Vector2Normalize(Vector2Subtract(p2.position, p1.position));
                float overlap = 20.0f - dist;

                // Resolve overlap by moving particles apart
                Vector2 resolution = Vector2Scale(normal, overlap / 2.0f);
                p1.position = Vector2Subtract(p1.position, resolution);
                p2.position = Vector2Add(p2.position, resolution);

                // Calculate relative velocity
                Vector2 relative_velocity = Vector2Subtract(p2.velocity, p1.velocity);
                float speed = Vector2DotProduct(relative_velocity, normal);

                if (speed < 0)
                {                             // Only resolve collisions if particles are moving toward each other
                    float restitution = 0.8f; // Slightly inelastic collision
                    float impulse = (-(1.0f + restitution) * speed) / 2.0f;

                    // Apply impulse to velocities
                    Vector2 impulseVector = Vector2Scale(normal, impulse);
                    p1.velocity = Vector2Subtract(p1.velocity, impulseVector);
                    p2.velocity = Vector2Add(p2.velocity, impulseVector);
                }
            }

            float force = (gravConst) / pow(dist, 2.0);

            // Gravitational force calculation
            if (dist > 20.0f)
            { // Only apply gravity if particles are not colliding
                SetGravitationalPull(p1, p2, force);
            }

            p1.potEnergy += force;
            //p2.potEnergy += force;
        }
    }
}