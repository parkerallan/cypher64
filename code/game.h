#ifndef GAME_H
#define GAME_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <math.h>
#include "player.h"
#include "debug_menu.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    T3DModel *tunnel_model;
    T3DViewport *viewport;
    Player player;
    DebugMenu debug_menu;
    
    // Display list for tunnel
    rspq_block_t *tunnelDpl;
    
    // Third-person camera
    T3DVec3 camPos;
    T3DVec3 camTarget;
    float camDistance;
    float camHeight;
    
    // Lighting
    uint8_t colorAmbient[4];
    uint8_t colorDir[4];
    uint8_t colorDir2[4];
    T3DVec3 lightDirVec;
    T3DVec3 lightDirVec2;
} TunnelScene;

// External variables
extern TunnelScene tunnel_scene;

// Function declarations
void tunnel_scene_init();
void tunnel_scene_update(joypad_buttons_t button, joypad_inputs_t inputs);
void tunnel_scene_render();
void tunnel_scene_cleanup();

#endif
