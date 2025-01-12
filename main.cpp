#include "raylib.h"
#include "raymath.h"

#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color = BLACK;
    float potEnergy; // Potential Energy
    float kinEnergy; // Kinetic Energy
};

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
        
        float minX = -2000.0f;   // Minimum X-coordinate (left boundary)
        float maxX = 2000.0f;  // Maximum X-coordinate (right boundary)
        float minY = -2000.0f;   // Minimum Y-coordinate (top boundary)
        float maxY = 2000.0f;  // Maximum Y-coordinate (bottom boundary)
        
        bool spawn_on_load = true; // Load particles
            
            int stepSizeX = 150;
            int stepSizeY = 150;
        
        
    
    
    
    // Other variables:
    
        
        const float gravConst = 0.000000000067;
        
        std::vector<Particle> parts;
        
        Camera2D camera = { 0 };
        camera.zoom = 0.30f;
        camera.target = {-3000, -1500};
        
        Vector2 init_pos;
        Vector2 fin_pos;
    
    
    
    
    if(spawn_on_load) {
        for(int x = minX + stepSizeX; x < maxX; x += stepSizeX)  {
            for(int y = minY + stepSizeY; y < maxY; y += stepSizeY) {
                Particle p;
                p.position = {x, y};
                p.velocity = Vector2Zero();
                
                parts.push_back(p);
            }
        }
    }
    

    InitWindow(screenWidth, screenHeight, "N-Body Simulation");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    
    const Texture2D texture = LoadTexture("Assets/particle.png");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            init_pos = GetScreenToWorld2D(GetMousePosition(), camera);
        }
        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            fin_pos = GetScreenToWorld2D(GetMousePosition(), camera);
            
            float dist = Vector2Distance(init_pos, fin_pos);
            float angle = atan2(fin_pos.y - init_pos.y, fin_pos.x - init_pos.x);
            
            Particle p;
            p.position.x = init_pos.x;
            p.position.y = init_pos.y;
            
            // Reset the velocity. Without this, all particles
            // will have the same velocity as the first spawned
            // particle upon being spawned.
            p.velocity = Vector2Zero();
            
            if(dist > 0.25f) {
                p.velocity.x = dist * cos(angle);
                p.velocity.y = dist * sin(angle);
            }
            
            parts.push_back(p);
        }
        
        if (GetMouseWheelMove() != 0) {
            
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.offset = GetMousePosition();
            
            camera.target = mouseWorldPos;

            // Update zoom level
            camera.zoom += GetMouseWheelMove() / 20.0f;
            camera.zoom = Clamp(camera.zoom, 0.10f, 5.0f);
            
            float scaleFactor = 1.0f + (0.25f * fabsf(GetMouseWheelMove()));
            
            if (GetMouseWheelMove() < 0) scaleFactor = 1.0f/scaleFactor;
            
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }
        
        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }
        
        camera.target.x += (IsKeyDown(KEY_LEFT) - IsKeyDown(KEY_RIGHT)) * 10;
        camera.target.y += (IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)) * 10;
        
        for (int i = 0; i < parts.size(); i++) {
            Particle &p1 = parts[i];
            
            p1.kinEnergy = 0.5 * pow(Vector2Length(p1.velocity), 2);
            
            // X boundaries
            if (p1.position.x < minX + 10) {
                p1.position.x = minX + 10;  // Stop or reflect the particle at the left boundary
                p1.velocity.x = -p1.velocity.x;  // Reflect velocity if you want bouncing effect
            }
            else if (p1.position.x > maxX - 10) {
                p1.position.x = maxX - 10;  // Stop or reflect the particle at the right boundary
                p1.velocity.x = -p1.velocity.x;  // Reflect velocity if you want bouncing effect
            }

            // Y boundaries
            if (p1.position.y < minY + 10) {
                p1.position.y = minY + 10;  // Stop or reflect the particle at the top boundary
                p1.velocity.y = -p1.velocity.y;  // Reflect velocity if you want bouncing effect
            }
            else if (p1.position.y > maxY - 10) {
                p1.position.y = maxY - 10;  // Stop or reflect the particle at the bottom boundary
                p1.velocity.y = -p1.velocity.y;  // Reflect velocity if you want bouncing effect
            }
            
            for (int d = i + 1; d < parts.size(); d++) {
                Particle &p2 = parts[d];
                
                float dist = Vector2Distance(p1.position, p2.position);
                
                    dist = Vector2Distance(p1.position, p2.position);
                    if (dist < 20.0f) {
                        Vector2 normal = Vector2Normalize(Vector2Subtract(p2.position, p1.position));
                        float overlap = 20.0f - dist;

                        // Resolve overlap by moving particles apart
                        Vector2 resolution = Vector2Scale(normal, overlap / 2.0f);
                        p1.position = Vector2Subtract(p1.position, resolution);
                        p2.position = Vector2Add(p2.position, resolution);

                        // Calculate relative velocity
                        Vector2 relative_velocity = Vector2Subtract(p2.velocity, p1.velocity);
                        float speed = Vector2DotProduct(relative_velocity, normal);

                        if (speed < 0) { // Only resolve collisions if particles are moving toward each other
                            float restitution = 0.8f; // Slightly inelastic collision
                            float impulse = (-(1.0f + restitution) * speed) / 2.0f;

                            // Apply impulse to velocities
                            Vector2 impulseVector = Vector2Scale(normal, impulse);
                            p1.velocity = Vector2Subtract(p1.velocity, impulseVector);
                            p2.velocity = Vector2Add(p2.velocity, impulseVector);
                        }
                    }
                
                float force = (gravConst * 25000000000000) / pow(dist, 2.0);
                
                // Gravitational force calculation
                if (dist > 20.0f) { // Only apply gravity if particles are not colliding
                    float angle = atan2(p2.position.y - p1.position.y, p2.position.x - p1.position.x);
                    
                    p1.velocity.x += force * cos(angle);
                    p1.velocity.y += force * sin(angle);
                    p2.velocity.x -= force * cos(angle);
                    p2.velocity.y -= force * sin(angle);
                }
                
                p1.potEnergy += force;
                p2.potEnergy += force;
            }
        
        // 1.3, 1500, and 200 are arbitrary numbers, tweaked to perfection.
        float normalizedEnergy = Normalize((p1.potEnergy * 1.3) + (p1.kinEnergy / 1500), 0, 200.0f);
        normalizedEnergy = Clamp(normalizedEnergy, 0.0f, 1.0f);
        
        p1.potEnergy = 0.0f; // Resets it for the next frame.
        
        int r = (int)(normalizedEnergy * 255);
        int b = (int)((1.0f - normalizedEnergy) * 255);
        
        p1.color = Color{r, 12, b, 255};

        // Update position after resolving collisions and applying forces
        p1.position.x += p1.velocity.x * GetFrameTime();
        p1.position.y += p1.velocity.y * GetFrameTime();
        }

        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            BeginMode2D(camera);
            
            for (float x = minX; x <= maxX; x += 100) {
                if(x == minX or x == maxX) {
                    DrawLine(x, minY, x, maxY, DARKGRAY);
                    continue;
                }
                DrawLine(x, minY, x, maxY, LIGHTGRAY);  // Vertical lines across the play area
            }

            // Draw horizontal lines (for y-axis, covering the range from -2000 to 2000)
            for (float y = minY; y <= maxY; y += 100) {
                if(y == minY or y == maxY) {
                    DrawLine(minX, y, maxX, y, DARKGRAY);
                    continue;
                }
                DrawLine(minX, y, maxX, y, LIGHTGRAY);  // Horizontal lines across the play area
            }
            
            for(Particle &p : parts) {
                DrawTexture(texture,
                    p.position.x - 10,
                    p.position.y - 10,
                    p.color);
            }
            
            EndMode2D();
            
            DrawText(TextFormat("Camera Zoom: %.2f", camera.zoom), 40, 0, 20, BLACK);
            DrawText(TextFormat("Camera Position: (%.0f, %.0f)", camera.target.x, camera.target.y), 40, 20, 20, BLACK);
            DrawFPS(40, 40);
            DrawText(TextFormat("Particles: %i", parts.size()), 40, 60, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}