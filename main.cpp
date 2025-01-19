#include "raylib.h"
#include "C:/raylib/raylib/src/raymath.h"

#include <vector>
#include <thread>

struct Particles
{
    std::vector<Vector2> pos;
    std::vector<Vector2> vel;
    std::vector<Color> color;
    std::vector<float> potEnergy; // Potential Energy
    std::vector<float> kinEnergy; // Kinetic Energy
};


void CreateParticle(Particles &parts, Vector2 vel, Vector2 pos);
void SetGravitationalPull(Vector2 &p1Vel, Vector2 &p1Pos, Vector2 &p2Vel, Vector2 &p2Pos, float force);
void DrawGrid();
void SpawnParticleBatch(Particles &parts, float stepSizeX, float stepSizeY);
void UpdateParticles(Particles &parts, int start, int end, float gravConst);
void ComputeForces(Particles &parts, int start, int end, float gravConst, bool collisionsEnabled);

float minX = -2000.0f; // Minimum X-coordinate (left boundary)
float maxX = 2000.0f;  // Maximum X-coordinate (right boundary)
float minY = -2000.0f; // Minimum Y-coordinate (top boundary)
float maxY = 2000.0f;  // Maximum Y-coordinate (bottom boundary)

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

    bool spawn_on_load = true; // Load particles

    int stepSizeX = 150;
    int stepSizeY = 150;

    const int threadCount = 7;

    bool collisionsEnabled = true;

    // Other variables:

    float gravConst = 1600.0f;

    Particles parts;
    parts.pos.reserve(2000);
    parts.vel.reserve(2000);
    parts.color.reserve(2000);
    parts.kinEnergy.reserve(2000);
    parts.potEnergy.reserve(2000);

    Camera2D camera = {0};
    camera.zoom = 0.30f;
    camera.target = {-3000, -1500};

    Vector2 init_pos;
    Vector2 fin_pos;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    if (spawn_on_load)
    {
        SpawnParticleBatch(parts, stepSizeX, stepSizeY);
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);
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

            Vector2 vel = Vector2Zero();

            if (dist > 0.25f)
            {
                vel.x = dist * cos(angle);
                vel.y = dist * sin(angle);
            }

            CreateParticle(parts, vel, init_pos);
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
            parts.pos.clear();
            parts.vel.clear();
            parts.color.clear();
            parts.kinEnergy.clear();
            parts.potEnergy.clear();

            if (spawn_on_load)
            {
                SpawnParticleBatch(parts, stepSizeX, stepSizeY);
            }
        }

        if (IsKeyPressed(KEY_C))
        {
            (collisionsEnabled) ? collisionsEnabled = false : collisionsEnabled = true;
        }

        gravConst = Clamp(gravConst, 0.0f, 409600.0f);

        camera.target.x += (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * 30;
        camera.target.y += (IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * 30;

        int particlesPerThread = parts.kinEnergy.size() / threadCount;
        int remainingParticles = parts.kinEnergy.size() % threadCount;

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

        

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        // Draw Grid, duh
        DrawGrid();

        
        for (int i = 0; i < parts.kinEnergy.size(); i++)
        {
            DrawTextureEx(texture, Vector2SubtractValue(parts.pos[i], 10), 0.0f, 1.0f, parts.color[i]);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            DrawLineEx(init_pos, GetScreenToWorld2D(GetMousePosition(), camera), 5.0f,{Clamp(Normalize(0.5 * pow(Vector2Distance(GetScreenToWorld2D(GetMousePosition(), camera), init_pos), 2.0f), 0.0f, 1000.0f), 0, 255U), 0U, 0U, 255U});
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
        DrawText(TextFormat("%i", parts.kinEnergy.size()), 250, 170, 20, DARKBLUE);
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

void CreateParticle(Particles &parts, Vector2 vel, Vector2 pos)
{
    parts.pos.push_back(pos);
    parts.vel.push_back(vel);
    parts.color.push_back(BLACK);
    parts.kinEnergy.push_back(0.0f);
    parts.potEnergy.push_back(0.0f);
}

void SetGravitationalPull(Vector2 &p1Vel, Vector2 &p1Pos, Vector2 &p2Vel, Vector2 &p2Pos, float force)
{
    float angle = atan2(p2Pos.y - p1Pos.y, p2Pos.x - p1Pos.x);

    p1Vel.x += force * cos(angle);
    p1Vel.y += force * sin(angle);
}

void DrawGrid()
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

void SpawnParticleBatch(Particles &parts, float stepSizeX, float stepSizeY)
{
    for (int x = minX + stepSizeX; x < maxX; x += stepSizeX)
    {
        for (int y = minY + stepSizeY; y < maxY; y += stepSizeY)
        {
            CreateParticle(parts, Vector2Zero(), {(float)x, (float)y});
        }
    }
}

void UpdateParticles(Particles &parts, int start, int end, float gravConst)
{
    for (int i = start; i < end; i++)
    {
        Vector2 &p1Pos = parts.pos[i];
        Vector2 &p1Vel = parts.vel[i];


        // X boundaries
        if (p1Pos.x < minX + 10)
        {
            p1Pos.x = minX + 10;      // Stop or reflect the particle at the left boundary
            p1Vel.x = -p1Vel.x; // Reflect velocity if you want bouncing effect
        }
        else if (p1Pos.x > maxX - 10)
        {
            p1Pos.x = maxX - 10;      // Stop or reflect the particle at the right boundary
            p1Vel.x = -p1Vel.x; // Reflect velocity if you want bouncing effect
        }

        // Y boundaries
        if (p1Pos.y < minY + 10)
        {
            p1Pos.y = minY + 10;      // Stop or reflect the particle at the top boundary
            p1Vel.y = -p1Vel.y; // Reflect velocity if you want bouncing effect
        }
        else if (p1Pos.y > maxY - 10)
        {
            p1Pos.y = maxY - 10;      // Stop or reflect the particle at the bottom boundary
            p1Vel.y = -p1Vel.y; // Reflect velocity if you want bouncing effect
        }

        float normalizedEnergy = Normalize((parts.potEnergy[i] * 1800 / gravConst) + (parts.kinEnergy[i] / 1500), 0, 200.0f);
        normalizedEnergy = Clamp(normalizedEnergy, 0.0f, 1.0f);

        parts.potEnergy[i] = 0.0f; // Resets it for the next frame.

        parts.color[i] = ColorLerp({0U, 12U, 255U, 255U}, {255U, 12U, 0U, 255U}, normalizedEnergy);

        // Update position after resolving collisions and applying forces
        p1Pos.x += p1Vel.x * GetFrameTime();
        p1Pos.y += p1Vel.y * GetFrameTime();
    }
}

void ComputeForces(Particles &parts, int start, int end, float gravConst, bool collisionsEnabled)
{
    for (int i = start; i < end; i++)
    {
        Vector2 &p1Pos = parts.pos[i];
        Vector2 &p1Vel = parts.vel[i];

        parts.kinEnergy[i] = 0.5 * pow(Vector2Length(p1Vel), 2);

        for (int d = 0; d < parts.kinEnergy.size(); d++)
        {
            if (i == d)
            {
                continue;
            }


            Vector2 &p2Pos = parts.pos[d];
            Vector2 &p2Vel = parts.vel[d];

            float dist = Vector2Distance(p1Pos, p2Pos);
            if (dist < 20.0f and collisionsEnabled)
            {
                Vector2 normal = Vector2Normalize(Vector2Subtract(p2Pos, p1Pos));
                float overlap = 20.0f - dist;

                // Resolve overlap by moving particles apart
                Vector2 resolution = Vector2Scale(normal, overlap / 2.0f);
                p1Pos = Vector2Subtract(p1Pos, resolution);
                p2Pos = Vector2Add(p2Pos, resolution);

                // Calculate relative velocity
                Vector2 relative_velocity = Vector2Subtract(p2Vel, p1Vel);
                float speed = Vector2DotProduct(relative_velocity, normal);

                if (speed < 0)
                {                             // Only resolve collisions if particles are moving toward each other
                    float restitution = 0.8f; // Slightly inelastic collision
                    float impulse = (-(1.0f + restitution) * speed) / 2.0f;

                    // Apply impulse to velocities
                    Vector2 impulseVector = Vector2Scale(normal, impulse);
                    p1Vel = Vector2Subtract(p1Vel, impulseVector);
                    p2Vel = Vector2Add(p2Vel, impulseVector);
                }
            }

            float force = (gravConst) / pow(dist, 2.0);

            // Gravitational force calculation
            if (dist > 20.0f)
            { // Only apply gravity if particles are not colliding
                SetGravitationalPull(p1Vel, p1Pos, p2Vel, p2Pos, force);
            }

            parts.potEnergy[i] += force;
        }
    }

    UpdateParticles(parts, start, end, gravConst);
}