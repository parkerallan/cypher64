#include "animation.h"
#include <malloc.h>
#include <string.h>

static float get_time_s() {
    return (float)((double)get_ticks_us() / 1000000.0);
}

void animation_system_init(AnimationSystem* anim_sys, T3DModel* model) {
    // Check for null pointers first
    if (anim_sys == NULL || model == NULL) {
        //debugf("Error: null animation system or model pointer\n");
        return;
    }
    
    // Initialize animation system
    anim_sys->anim_count = t3d_model_get_animation_count(model);
    anim_sys->current_anim = -1;
    anim_sys->walk_anim_index = -1;
    anim_sys->run_anim_index = -1;
    anim_sys->idle_anim_index = -1;
    anim_sys->jump_anim_index = -1;
    anim_sys->last_anim_time = get_time_s();
    anim_sys->is_moving = false;
    anim_sys->was_moving = false;
    anim_sys->is_jumping = false;
    anim_sys->was_jumping = false;
    
    //debugf("Player model has %lu animations\n", anim_sys->anim_count);
    
    if (anim_sys->anim_count > 0) {
        anim_sys->anims = malloc(anim_sys->anim_count * sizeof(void*));
        t3d_model_get_animations(model, anim_sys->anims);
        
        // Create animation instances
        anim_sys->anim_instances = malloc(anim_sys->anim_count * sizeof(T3DAnim));
        for (int i = 0; i < anim_sys->anim_count; i++) {
            anim_sys->anim_instances[i] = t3d_anim_create(model, anim_sys->anims[i]->name);
            //debugf("Animation %d: %s\n", i, anim_sys->anims[i]->name);
            
            // Verify animation was created successfully
            if (anim_sys->anim_instances[i].animRef == NULL) {
                //debugf("Failed to create animation %d: %s\n", i, anim_sys->anims[i]->name);
                continue;
            }
            
            // Find the walk animation
            if (strcmp(anim_sys->anims[i]->name, "Walk") == 0) {
                anim_sys->walk_anim_index = i;
                //debugf("Found Walk animation at index %d\n", i);
            }
            // Find the run animation
            else if (strcmp(anim_sys->anims[i]->name, "Run") == 0) {
                anim_sys->run_anim_index = i;
                //debugf("Found Run animation at index %d\n", i);
            }
            // Find the idle animation
            else if (strcmp(anim_sys->anims[i]->name, "Idle") == 0) {
                anim_sys->idle_anim_index = i;
                //debugf("Found Idle animation at index %d\n", i);
            }
            // Find the jump animation
            else if (strcmp(anim_sys->anims[i]->name, "Jump") == 0) {
                anim_sys->jump_anim_index = i;
                //debugf("Found Jump animation at index %d\n", i);
            }
        }
        
        // Don't automatically start any animation during init
        // Let the update function handle initial animation setup
        //debugf("Animation system initialized, will start appropriate animation on first update\n");
    } else {
        //debugf("No animations found in player model\n");
        anim_sys->anims = NULL;
        anim_sys->anim_instances = NULL;
    }
}

void animation_system_update(AnimationSystem* anim_sys, T3DSkeleton* skeleton, bool is_moving, bool is_jumping, bool debug_menu_active) {
    // Check for null pointers first
    if (anim_sys == NULL || skeleton == NULL) {
        //debugf("Error: null animation system or skeleton pointer\n");
        return;
    }
    
    // Handle animation state changes - only if debug menu is not active
    if (!debug_menu_active && anim_sys->anim_count > 0 && anim_sys->anim_instances != NULL) {
        float new_time = get_time_s();
        float delta_time = new_time - anim_sys->last_anim_time;
        anim_sys->last_anim_time = new_time;
        
        // Update movement state
        anim_sys->was_moving = anim_sys->is_moving;
        anim_sys->is_moving = is_moving;
        anim_sys->was_jumping = anim_sys->is_jumping;
        anim_sys->is_jumping = is_jumping;
        
        // Jump has highest priority - check for jump first
        if (anim_sys->is_jumping && !anim_sys->was_jumping) {
            // Started jumping - switch to jump animation
            if (anim_sys->jump_anim_index >= 0 && anim_sys->jump_anim_index < anim_sys->anim_count &&
                anim_sys->anim_instances[anim_sys->jump_anim_index].animRef != NULL) {
                // Switch to jump animation (don't loop - jump is a one-shot animation)
                anim_sys->current_anim = anim_sys->jump_anim_index;
                t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], false); // Don't loop jump
                //debugf("Started Jump animation\n");
            } else {
                //debugf("Jump animation not available or invalid\n");
            }
        }
        // Only handle movement animations if not jumping
        else if (!anim_sys->is_jumping) {
            // Movement animation logic (only when not jumping)
            // If running, play Run animation
            extern bool g_is_running; // Will be set in player.c
            if (g_is_running && anim_sys->run_anim_index >= 0 && anim_sys->run_anim_index < anim_sys->anim_count &&
                anim_sys->anim_instances[anim_sys->run_anim_index].animRef != NULL) {
                if (anim_sys->current_anim != anim_sys->run_anim_index) {
                    anim_sys->current_anim = anim_sys->run_anim_index;
                    t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                    t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    //debugf("Started Run animation\n");
                }
                // Ensure run animation keeps playing and restart if it finished
                else if (!anim_sys->anim_instances[anim_sys->current_anim].isPlaying) {
                    t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    //debugf("Restarted Run animation\n");
                }
            } else if (anim_sys->is_moving && !anim_sys->was_moving) {
                // Started moving - switch to walk animation
                if (anim_sys->walk_anim_index >= 0 && anim_sys->walk_anim_index < anim_sys->anim_count &&
                    anim_sys->anim_instances[anim_sys->walk_anim_index].animRef != NULL) {
                    // Switch to walk animation
                    anim_sys->current_anim = anim_sys->walk_anim_index;
                    t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                    t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    //debugf("Started Walk animation\n");
                } else {
                    //debugf("Walk animation not available or invalid\n");
                }
            } else if (!anim_sys->is_moving && anim_sys->was_moving) {
                // Stopped moving - switch to idle animation
                //debugf("Stopping movement, switching to idle\n");
                if (anim_sys->idle_anim_index >= 0 && anim_sys->idle_anim_index < anim_sys->anim_count &&
                    anim_sys->anim_instances[anim_sys->idle_anim_index].animRef != NULL) {
                    // Switch to idle animation
                    anim_sys->current_anim = anim_sys->idle_anim_index;
                    t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                    t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                    //debugf("Started Idle animation\n");
                } else {
                    // No idle animation available, just stop current animation
                    if (anim_sys->current_anim >= 0 && anim_sys->current_anim < anim_sys->anim_count) {
                        t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], false);
                        anim_sys->current_anim = -1;
                    }
                    //debugf("No idle animation available, stopped current animation\n");
                }
            }
            // Initialize idle animation on first update if no movement and no current animation
            if (anim_sys->current_anim == -1 && !anim_sys->is_moving && 
                anim_sys->idle_anim_index >= 0 && anim_sys->idle_anim_index < anim_sys->anim_count &&
                anim_sys->anim_instances[anim_sys->idle_anim_index].animRef != NULL) {
                anim_sys->current_anim = anim_sys->idle_anim_index;
                t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                //debugf("Initialized with Idle animation\n");
            }
        }
        
        // Check if jump animation finished (only for non-looping animations)
        if (anim_sys->current_anim == anim_sys->jump_anim_index && 
            anim_sys->current_anim >= 0 && anim_sys->current_anim < anim_sys->anim_count &&
            anim_sys->anim_instances != NULL &&
            !anim_sys->anim_instances[anim_sys->current_anim].isPlaying) {
            // Jump animation finished, return to appropriate state
            //debugf("Jump animation finished, returning to movement state\n");
            if (anim_sys->is_moving && anim_sys->walk_anim_index >= 0) {
                // Return to walking
                anim_sys->current_anim = anim_sys->walk_anim_index;
                t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                //debugf("Returned to Walk animation after jump\n");
            } else if (anim_sys->idle_anim_index >= 0) {
                // Return to idle
                anim_sys->current_anim = anim_sys->idle_anim_index;
                t3d_anim_attach(&anim_sys->anim_instances[anim_sys->current_anim], skeleton);
                t3d_anim_set_playing(&anim_sys->anim_instances[anim_sys->current_anim], true);
                t3d_anim_set_looping(&anim_sys->anim_instances[anim_sys->current_anim], true);
                //debugf("Returned to Idle animation after jump\n");
            }
        }
        
        // Update current animation - only if we have a valid active animation
        if (anim_sys->current_anim >= 0 && anim_sys->current_anim < anim_sys->anim_count && 
            anim_sys->anim_instances != NULL && 
            anim_sys->anim_instances[anim_sys->current_anim].isPlaying) {
            t3d_anim_update(&anim_sys->anim_instances[anim_sys->current_anim], delta_time);
        }
        
        // Always update skeleton
        t3d_skeleton_update(skeleton);
    } else {
        // No animations available or debug menu is active, just update skeleton
        t3d_skeleton_update(skeleton);
    }
}

void animation_system_cleanup(AnimationSystem* anim_sys) {
    // Check for null pointer first
    if (anim_sys == NULL) {
        //debugf("Error: null animation system pointer\n");
        return;
    }
    
    // Cleanup animation system
    if (anim_sys->anim_instances) {
        for (int i = 0; i < anim_sys->anim_count; i++) {
            t3d_anim_destroy(&anim_sys->anim_instances[i]);
        }
        free(anim_sys->anim_instances);
        anim_sys->anim_instances = NULL;
    }
    
    if (anim_sys->anims) {
        free(anim_sys->anims);
        anim_sys->anims = NULL;
    }
    
    anim_sys->anim_count = 0;
    anim_sys->current_anim = -1;
    anim_sys->walk_anim_index = -1;
    anim_sys->run_anim_index = -1;
    anim_sys->idle_anim_index = -1;
    anim_sys->jump_anim_index = -1;
}