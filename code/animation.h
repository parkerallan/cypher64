#ifndef ANIMATION_H
#define ANIMATION_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

typedef struct {
    uint32_t anim_count;
    T3DChunkAnim **anims;
    T3DAnim *anim_instances;
    int current_anim;
    int walk_anim_index;
    int run_anim_index;
    int idle_anim_index;
    int jump_anim_index;
    float last_anim_time;
    bool is_moving;
    bool was_moving;
    bool is_jumping;
    bool was_jumping;
} AnimationSystem;

// Animation system functions
void animation_system_init(AnimationSystem* anim_sys, T3DModel* model);
void animation_system_update(AnimationSystem* anim_sys, T3DSkeleton* skeleton, bool is_moving, bool is_jumping, bool debug_menu_active);
void animation_system_cleanup(AnimationSystem* anim_sys);

#endif
