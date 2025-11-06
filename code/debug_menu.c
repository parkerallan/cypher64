#include "debug_menu.h"
#include <malloc.h>
#include <math.h>

#define STRINGIFY(x) #x
#define STYLE(id) "^0" STRINGIFY(id)
#define STYLE_TITLE 1
#define STYLE_GREY 2
#define STYLE_GREEN 3

static float get_time_s() {
    return (float)((double)get_ticks_us() / 1000000.0);
}

void debug_menu_init(DebugMenu* menu, Player* player) {
    menu->is_active = false;  // Start with debug menu visible by default
    menu->active_anim = 0;
    menu->active_blend_anim = -1;
    menu->blend_factor = 0.0f;
    menu->time_cursor = 0.5f;
    menu->last_time = get_time_s() - (1.0f / 60.0f);
    menu->show_help = true;
    
    // Load and register the builtin debug font with a unique ID
    menu->debug_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    // Use a unique font ID that won't conflict
    rdpq_text_register_font(10, menu->debug_font);
    
    // Get animation data from player model
    menu->anim_count = t3d_model_get_animation_count(player->model);
    if (menu->anim_count > 0) {
        menu->anims = malloc(menu->anim_count * sizeof(void*));
        t3d_model_get_animations(player->model, menu->anims);
        
        // Create animation instances that work with the player's skeleton
        menu->anim_instances = malloc(menu->anim_count * sizeof(T3DAnim));
        for (int i = 0; i < menu->anim_count; i++) {
            menu->anim_instances[i] = t3d_anim_create(player->model, menu->anims[i]->name);
            // Don't attach here - we'll attach the active one in update
        }
        
        // Create blend skeleton
        menu->skel_blend = t3d_skeleton_clone(&player->skeleton, false);
        
        // Don't automatically attach animations - let player system handle this
        // Only take control when debug menu becomes active
    } else {
        menu->anims = NULL;
        menu->anim_instances = NULL;
    }
}

void debug_menu_update(DebugMenu* menu, Player* player, joypad_buttons_t buttons, joypad_inputs_t inputs) {
    if (!menu->is_active || menu->anim_count == 0) return;
    
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    int last_anim = menu->active_anim;
    
    // Animation selection
    if (btn.c_up) menu->active_anim--;
    if (btn.c_down) menu->active_anim++;
    menu->active_anim = (menu->active_anim + (int)menu->anim_count) % (int)menu->anim_count;
    
    // If animation changed, attach the new one to the player's skeleton
    if (last_anim != menu->active_anim) {
        t3d_skeleton_reset(&player->skeleton);
        t3d_anim_attach(&menu->anim_instances[menu->active_anim], &player->skeleton);
        t3d_anim_set_playing(&menu->anim_instances[menu->active_anim], true);
    }
    
    // Animation control
    if (btn.start) {
        t3d_anim_set_playing(&menu->anim_instances[menu->active_anim], 
                           !menu->anim_instances[menu->active_anim].isPlaying);
    }
    if (btn.z) {
        t3d_anim_set_looping(&menu->anim_instances[menu->active_anim], 
                           !menu->anim_instances[menu->active_anim].isLooping);
    }
    
    // Blend controls
    if (inputs.btn.c_left) menu->blend_factor -= 0.0075f;
    if (inputs.btn.c_right) menu->blend_factor += 0.0075f;
    if (menu->blend_factor > 1.0f) menu->blend_factor = 1.0f;
    if (menu->blend_factor < 0.0f) menu->blend_factor = 0.0f;
    
    // Time cursor
    menu->time_cursor += (float)inputs.stick_x * 0.0001f;
    
    // Blend animation selection
    if (btn.b) {
        if (menu->active_blend_anim >= 0) {
            t3d_anim_attach(&menu->anim_instances[menu->active_blend_anim], &player->skeleton);
        }
        menu->active_blend_anim = (menu->active_blend_anim == menu->active_anim) ? -1 : menu->active_anim;
        
        if (menu->active_blend_anim >= 0) {
            t3d_skeleton_reset(&menu->skel_blend);
            t3d_anim_attach(&menu->anim_instances[menu->active_blend_anim], &menu->skel_blend);
        }
    }
    if (menu->active_blend_anim >= menu->anim_count) menu->active_blend_anim = -1;
    
    // Set time cursor
    if (btn.a) {
        t3d_anim_set_time(&menu->anim_instances[menu->active_anim], menu->time_cursor);
    }
    
    // Animation speed control
    menu->anim_instances[menu->active_anim].speed += (float)inputs.stick_y * 0.0001f;
    if (menu->anim_instances[menu->active_anim].speed < 0.0f) {
        menu->anim_instances[menu->active_anim].speed = 0.0f;
    }
    
    // Time cursor bounds
    if (menu->time_cursor < 0.0f) {
        menu->time_cursor = menu->anim_instances[menu->active_anim].animRef->duration;
    }
    if (menu->time_cursor > menu->anim_instances[menu->active_anim].animRef->duration) {
        menu->time_cursor = 0.0f;
    }
    
    // Update animations and apply to player skeleton
    float new_time = get_time_s();
    float delta_time = new_time - menu->last_time;
    menu->last_time = new_time;
    
    // Update the active animation
    t3d_anim_update(&menu->anim_instances[menu->active_anim], delta_time);
    
    if (menu->active_blend_anim >= 0) {
        t3d_anim_update(&menu->anim_instances[menu->active_blend_anim], delta_time);
        t3d_skeleton_blend(&player->skeleton, &menu->skel_blend, &player->skeleton, menu->blend_factor);
    }
    
    // Apply the animation to the player's skeleton
    t3d_skeleton_update(&player->skeleton);
    
    // Toggle help
    if (btn.c_left && btn.c_right) {
        menu->show_help = !menu->show_help;
    }
}

void debug_menu_render(DebugMenu* menu, Player* player) {
    if (!menu->is_active) return;
    
    rdpq_sync_pipe();
    
    if (menu->anim_count == 0) {
        rdpq_text_printf(NULL, 10, 16, 20, 
                        "No animations found in model");
        return;
    }
    
    float posX = 16;
    float posY = 20;
    
    // Title
    rdpq_set_prim_color(RGBA32(0xAA, 0xAA, 0xFF, 0xFF));
    rdpq_text_printf(NULL, 10, posX, posY, 
                    "Player Animation Debug");
    posY += 20;
    
    // Animation list
    rdpq_set_prim_color(RGBA32(0xAA, 0xAA, 0xFF, 0xFF));
    rdpq_text_printf(NULL, 10, posX, posY, 
                    "[C-U/D] Animations (%lu total):", menu->anim_count);
    posY += 10;
    
    for (int i = 0; i < menu->anim_count; i++) {
        const T3DChunkAnim *anim = menu->anims[i];
        
        if (menu->active_blend_anim == i) {
            rdpq_set_prim_color(RGBA32(0x39, 0xBF, 0x1F, 0xFF)); // Green
            rdpq_text_printf(NULL, 10, posX, posY, 
                           "%d: %s: %.2fs (%d%%)", i, anim->name, anim->duration, 
                           (int)((1.0f - menu->blend_factor) * 100));
        } else if (menu->active_anim == i) {
            rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF)); // White
            if (menu->active_blend_anim >= 0) {
                rdpq_text_printf(NULL, 10, posX, posY, 
                               "%d: %s: %.2fs (%d%%)", i, anim->name, anim->duration, 
                               (int)(menu->blend_factor * 100));
            } else {
                rdpq_text_printf(NULL, 10, posX, posY, 
                               "%d: %s: %.2fs", i, anim->name, anim->duration);
            }
        } else {
            rdpq_set_prim_color(RGBA32(0x66, 0x66, 0x66, 0xFF)); // Grey
            rdpq_text_printf(NULL, 10, posX, posY, 
                           "%d: %s: %.2fs", i, anim->name, anim->duration);
        }
        posY += 10;
    }
    
    posY += 10;
    
    // Current animation details
    if (menu->active_anim < menu->anim_count) {
        T3DChunkAnim *anim = menu->anims[menu->active_anim];
        rdpq_set_prim_color(RGBA32(0xAA, 0xAA, 0xFF, 0xFF));
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Animation: %s", anim->name);
        posY += 10;
        
        rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Duration: %.2fs", anim->duration);
        posY += 10;
        
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Keyframes: %ld", anim->keyframeCount);
        posY += 10;
        
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Speed: %.2fx", menu->anim_instances[menu->active_anim].speed);
        posY += 10;
        
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Time: %.2fs / %.2fs %c", 
                        menu->anim_instances[menu->active_anim].time, anim->duration,
                        menu->anim_instances[menu->active_anim].isLooping ? 'L' : '-');
        posY += 20;
        
        // Timeline
        float barWidth = 150.0f;
        float barHeight = 10.0f;
        float timeScale = barWidth / anim->duration;
        
        // Backdrop
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 0xFF));
        rdpq_fill_rectangle(posX-2, posY-2, posX + barWidth+2, posY + barHeight+2);
        
        // Background
        rdpq_set_fill_color(RGBA32(0x33, 0x33, 0x33, 0xFF));
        rdpq_fill_rectangle(posX, posY, posX + barWidth, posY + barHeight);
        
        // Progress bar
        rdpq_set_fill_color(RGBA32(0xAA, 0xAA, 0xAA, 0xFF));
        rdpq_fill_rectangle(posX, posY, 
                          posX + (menu->anim_instances[menu->active_anim].time * timeScale), 
                          posY + barHeight);
        
        // Cursor
        rdpq_set_fill_color(RGBA32(0x55, 0x55, 0xFF, 0xFF));
        rdpq_fill_rectangle(posX + (menu->time_cursor * timeScale), posY-2, 
                          posX + (menu->time_cursor * timeScale) + 2, posY + barHeight+2);
        
        posY += 20;
    }
    
    // Help text
    if (menu->show_help) {
        posY += 10;
        rdpq_set_prim_color(RGBA32(0xAA, 0xAA, 0xFF, 0xFF));
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Controls:");
        posY += 10;
        rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "C-U/D: Switch animation");
        posY += 8;
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Start: Play/Pause");
        posY += 8;
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Z: Toggle loop");
        posY += 8;
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "A: Set time cursor");
        posY += 8;
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "B: Blend animation");
        posY += 8;
        rdpq_text_printf(NULL, 10, posX, posY, 
                        "Stick: Time/Speed");
    }
}

void debug_menu_cleanup(DebugMenu* menu) {
    if (menu->anims) {
        free(menu->anims);
        menu->anims = NULL;
    }
    
    if (menu->anim_instances) {
        free(menu->anim_instances);
        menu->anim_instances = NULL;
    }
    
    if (menu->anim_count > 0) {
        t3d_skeleton_destroy(&menu->skel_blend);
    }
}

bool debug_menu_is_active(DebugMenu* menu) {
    return menu->is_active;
}

void debug_menu_toggle(DebugMenu* menu) {
    menu->is_active = !menu->is_active;
}