#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include "player.h"

typedef struct {
    bool is_active;
    int active_anim;
    int active_blend_anim;
    float blend_factor;
    float time_cursor;
    float last_time;
    uint32_t anim_count;
    T3DChunkAnim **anims;
    T3DAnim *anim_instances;
    T3DSkeleton skel_blend;
    rdpq_font_t* debug_font;
    bool show_help;
} DebugMenu;

// Function declarations
void debug_menu_init(DebugMenu* menu, Player* player);
void debug_menu_update(DebugMenu* menu, Player* player, joypad_buttons_t buttons, joypad_inputs_t inputs);
void debug_menu_render(DebugMenu* menu, Player* player);
void debug_menu_cleanup(DebugMenu* menu);
bool debug_menu_is_active(DebugMenu* menu);
void debug_menu_toggle(DebugMenu* menu);

#endif // DEBUG_MENU_H