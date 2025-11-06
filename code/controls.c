#include "controls.h"
#include <math.h>

void controls_init(void) {
    // Initialize any control-related settings here
    // Could be used for control remapping in the future
}

PlayerInput controls_get_player_input(joypad_buttons_t buttons, joypad_inputs_t inputs) {
    PlayerInput input = {0}; // Initialize all values to 0/false
    
    // Primary movement: Analog stick
    // Forward/backward movement (stick Y-axis)
    if (inputs.stick_y != 0) {
        // stick_y: up is negative, down is positive
        // We want: up = forward (+1.0), down = backward (-1.0)
        float stick_forward = inputs.stick_y / 127.0f;
        if (stick_forward > 1.0f) stick_forward = 1.0f;
        if (stick_forward < -1.0f) stick_forward = -1.0f;
        input.move_forward = stick_forward;
    }
    
    // Turning (stick X-axis)
    if (inputs.stick_x != 0) {
        float stick_turn = inputs.stick_x / 127.0f;
        // Clamp to -1.0 to 1.0 range
        if (stick_turn > 1.0f) stick_turn = 1.0f;
        if (stick_turn < -1.0f) stick_turn = -1.0f;
        input.turn_rate = stick_turn;
    }
    
    // Backup movement: D-pad (for when analog stick isn't available)
    // Only use D-pad if analog stick isn't being used
    if (fabs(input.move_forward) < 0.1f) {
        if (buttons.d_up) {
            input.move_forward = 1.0f;
        } else if (buttons.d_down) {
            input.move_forward = -1.0f;
        }
    }
    
    if (fabs(input.turn_rate) < 0.1f) {
        if (buttons.d_left) {
            input.turn_rate = -1.0f;
        } else if (buttons.d_right) {
            input.turn_rate = 1.0f;
        }
    }
    
    // Action buttons
    input.jump = buttons.a;
    input.action = buttons.b;
    input.secondary_action = buttons.c_up;
    input.run = buttons.z;
    
    // Camera controls (C buttons)
    if (buttons.c_left) {
        input.camera_x = -1.0f;
    } else if (buttons.c_right) {
        input.camera_x = 1.0f;
    }
    
    if (buttons.c_up) {
        input.camera_y = 1.0f;
    } else if (buttons.c_down) {
        input.camera_y = -1.0f;
    }
    
    input.camera_reset = buttons.l;
    
    // Menu controls
    input.pause = buttons.start;
    input.select = buttons.a;
    input.cancel = buttons.b;
    
    return input;
}
