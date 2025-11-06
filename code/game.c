#include "game.h"
#include <rdpq_tex.h>
#include <malloc.h>
#include <t3d/t3dskeleton.h>

TunnelScene tunnel_scene;

sprite_t* tunnelTexture = NULL;

void tunnel_scene_init() {
    // Initialize lighting vectors
    tunnel_scene.lightDirVec = (T3DVec3){{0.3f, -0.8f, 0.5f}};
    tunnel_scene.lightDirVec2 = (T3DVec3){{-0.5f, 0.2f, -0.3f}};
    t3d_vec3_norm(&tunnel_scene.lightDirVec);
    t3d_vec3_norm(&tunnel_scene.lightDirVec2);
    
    // Load tunnel model
    tunnel_scene.tunnel_model = t3d_model_load("rom:/tunnel2.t3dm");
    tunnelTexture = NULL;  // Not needed for T3D models
    
    // Create display list for tunnel
    rspq_block_begin();
    t3d_model_draw(tunnel_scene.tunnel_model);
    tunnel_scene.tunnelDpl = rspq_block_end();
    
    // Initialize player
    player_init(&tunnel_scene.player);
    
    // Debug menu disabled - commented out to avoid conflicts
    // debug_menu_init(&tunnel_scene.debug_menu, &tunnel_scene.player);
    
    // Initialize third-person camera settings - Raised camera for better player centering
    tunnel_scene.camDistance = 200.0f;  // Further back for better view
    tunnel_scene.camHeight = 200.0f;    // Raised camera height to show less ground
    tunnel_scene.camPos = (T3DVec3){{0.0f, tunnel_scene.camHeight, tunnel_scene.camDistance}};
    tunnel_scene.camTarget = (T3DVec3){{0.0f, 0.0f, 0.0f}};
    
    // Initialize lighting colors - neutral/warm dungeon lighting
    tunnel_scene.colorAmbient[0] = 40;   // R - slightly warmer ambient
    tunnel_scene.colorAmbient[1] = 35;   // G  
    tunnel_scene.colorAmbient[2] = 25;   // B - reduced blue
    tunnel_scene.colorAmbient[3] = 0xFF; // A
    
    tunnel_scene.colorDir[0] = 220;      // R - warm main light
    tunnel_scene.colorDir[1] = 200;      // G
    tunnel_scene.colorDir[2] = 160;      // B - reduced blue
    tunnel_scene.colorDir[3] = 0xFF;     // A
    
    tunnel_scene.colorDir2[0] = 80;      // R - warmer secondary light
    tunnel_scene.colorDir2[1] = 70;      // G
    tunnel_scene.colorDir2[2] = 50;      // B - much less blue
    tunnel_scene.colorDir2[3] = 0xFF;    // A
    
    // Create viewport like T3D examples
    tunnel_scene.viewport = malloc(sizeof(T3DViewport));
    *tunnel_scene.viewport = t3d_viewport_create();
}

void tunnel_scene_update(joypad_buttons_t button, joypad_inputs_t inputs) {
    // Debug menu disabled - commented out to avoid conflicts
    // Check for debug menu toggle (Z button)
    // joypad_buttons_t btn_pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    // if (btn_pressed.z) {
    //     debug_menu_toggle(&tunnel_scene.debug_menu);
    // }
    
    // Always update debug menu if active
    // if (debug_menu_is_active(&tunnel_scene.debug_menu)) {
    //     debug_menu_update(&tunnel_scene.debug_menu, &tunnel_scene.player, button, inputs);
    // }
    
    // Always update player movement (debug menu disabled, so always pass false)
    player_update(&tunnel_scene.player, button, inputs, false);
    
    // Update camera to follow player
    tunnel_scene.camPos = player_get_camera_position(&tunnel_scene.player, tunnel_scene.camDistance, tunnel_scene.camHeight);
    tunnel_scene.camTarget = player_get_camera_target(&tunnel_scene.player, 100.0f, 125.0f);  // Look up 50 units to center player better
}

void tunnel_scene_render() {
    // Use T3D example values for better Z-buffer precision and avoid clipping
    t3d_viewport_set_projection(tunnel_scene.viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 500.0f);
    t3d_viewport_look_at(tunnel_scene.viewport, &tunnel_scene.camPos, &tunnel_scene.camTarget, &(T3DVec3){{0,1,0}});

    rdpq_attach(display_get(), display_get_zbuf());

    t3d_frame_start();
    t3d_viewport_attach(tunnel_scene.viewport);

    // Dark atmosphere for dungeon
    t3d_screen_clear_color(RGBA32(10, 10, 20, 0xFF));
    t3d_screen_clear_depth();
    
    // Set render state flags like T3D examples - add depth bias to prevent Z-fighting
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH);
    t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
    
    // Set up atmospheric lighting (KEEP YOUR LIGHTING)
    t3d_light_set_ambient(tunnel_scene.colorAmbient);
    t3d_light_set_directional(0, tunnel_scene.colorDir, &tunnel_scene.lightDirVec);
    t3d_light_set_directional(1, tunnel_scene.colorDir2, &tunnel_scene.lightDirVec2);
    t3d_light_set_count(2);

    // Draw the tunnel using display list
    rspq_block_run(tunnel_scene.tunnelDpl);
    
    // Draw the player using skinned rendering
    player_render(&tunnel_scene.player);

    // Debug menu disabled - commented out to avoid conflicts
    // debug_menu_render(&tunnel_scene.debug_menu, &tunnel_scene.player);

    rdpq_detach_show();
}

void tunnel_scene_cleanup() {
    if (tunnel_scene.tunnel_model) {
        t3d_model_free(tunnel_scene.tunnel_model);
        tunnel_scene.tunnel_model = NULL;
    }
    

    
    // No need to free textures - T3D handles this internally
    
    // Cleanup player
    player_cleanup(&tunnel_scene.player);
    
    // Debug menu disabled - commented out to avoid conflicts
    // debug_menu_cleanup(&tunnel_scene.debug_menu);
    
    if (tunnel_scene.viewport) {
        free(tunnel_scene.viewport);
        tunnel_scene.viewport = NULL;
    }
}
