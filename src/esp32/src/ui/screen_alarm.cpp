/**
 * BrewOS Alarm Screen Implementation
 * 
 * Full-screen alarm display
 */

#include "platform/platform.h"
#include "ui/screen_alarm.h"
#include "display/theme.h"
#include "display/display_config.h"

// Static elements
static lv_obj_t* screen = nullptr;
static lv_obj_t* alarm_icon = nullptr;
static lv_obj_t* code_label = nullptr;
static lv_obj_t* message_label = nullptr;
static lv_obj_t* instruction_label = nullptr;
static lv_anim_t icon_anim;

// =============================================================================
// Screen Creation
// =============================================================================

lv_obj_t* screen_alarm_create(void) {
    LOG_I("Creating alarm screen...");
    
    // Create screen with dark red tint
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A0505), 0);
    
    // Create main container
    lv_obj_t* container = lv_obj_create(screen);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // === Pulsing red ring ===
    lv_obj_t* ring = lv_arc_create(container);
    lv_obj_set_size(ring, 400, 400);
    lv_obj_center(ring);
    lv_arc_set_range(ring, 0, 100);
    lv_arc_set_value(ring, 100);
    lv_arc_set_bg_angles(ring, 0, 360);
    lv_obj_set_style_arc_color(ring, COLOR_ERROR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ring, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ring, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    
    // Animate ring opacity
    lv_anim_t ring_anim;
    lv_anim_init(&ring_anim);
    lv_anim_set_var(&ring_anim, ring);
    lv_anim_set_values(&ring_anim, LV_OPA_30, LV_OPA_COVER);
    lv_anim_set_time(&ring_anim, 800);
    lv_anim_set_repeat_count(&ring_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&ring_anim, 800);
    lv_anim_set_exec_cb(&ring_anim, [](void* obj, int32_t v) {
        lv_obj_set_style_arc_opa((lv_obj_t*)obj, v, LV_PART_INDICATOR);
    });
    lv_anim_start(&ring_anim);
    
    // === Alarm Icon ===
    alarm_icon = lv_label_create(container);
    lv_label_set_text(alarm_icon, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(alarm_icon, FONT_TEMP, 0);
    lv_obj_set_style_text_color(alarm_icon, COLOR_ERROR, 0);
    lv_obj_align(alarm_icon, LV_ALIGN_CENTER, 0, -80);
    
    // Animate icon
    lv_anim_init(&icon_anim);
    lv_anim_set_var(&icon_anim, alarm_icon);
    lv_anim_set_values(&icon_anim, LV_OPA_50, LV_OPA_COVER);
    lv_anim_set_time(&icon_anim, 500);
    lv_anim_set_repeat_count(&icon_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&icon_anim, 500);
    lv_anim_set_exec_cb(&icon_anim, [](void* obj, int32_t v) {
        lv_obj_set_style_opa((lv_obj_t*)obj, v, 0);
    });
    lv_anim_start(&icon_anim);
    
    // === ALARM title ===
    lv_obj_t* title = lv_label_create(container);
    lv_label_set_text(title, "ALARM");
    lv_obj_set_style_text_font(title, FONT_HUGE, 0);
    lv_obj_set_style_text_color(title, COLOR_ERROR, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -10);
    
    // === Code ===
    code_label = lv_label_create(container);
    lv_label_set_text(code_label, "Code: 0x00");
    lv_obj_set_style_text_font(code_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(code_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(code_label, LV_ALIGN_CENTER, 0, 30);
    
    // === Message ===
    message_label = lv_label_create(container);
    lv_label_set_text(message_label, "Unknown error");
    lv_obj_set_style_text_font(message_label, FONT_MEDIUM, 0);
    lv_obj_set_style_text_color(message_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 70);
    
    // === Instructions ===
    instruction_label = lv_label_create(container);
    lv_label_set_text(instruction_label, "Turn off the machine and check water/power.\nPress knob to acknowledge.");
    lv_obj_set_style_text_font(instruction_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(instruction_label, COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -60);
    
    LOG_I("Alarm screen created");
    return screen;
}

// =============================================================================
// Screen Update
// =============================================================================

void screen_alarm_set(uint8_t code, const char* message) {
    if (!screen) return;
    
    // Update code
    char code_str[16];
    snprintf(code_str, sizeof(code_str), "Code: 0x%02X", code);
    lv_label_set_text(code_label, code_str);
    
    // Update message
    if (message && strlen(message) > 0) {
        lv_label_set_text(message_label, message);
    } else {
        // Default messages based on code
        switch (code) {
            case 0x01:
                lv_label_set_text(message_label, "Brew boiler overtemp");
                break;
            case 0x02:
                lv_label_set_text(message_label, "Steam boiler overtemp");
                break;
            case 0x03:
                lv_label_set_text(message_label, "Water tank empty");
                break;
            case 0x04:
                lv_label_set_text(message_label, "Sensor failure");
                break;
            case 0x05:
                lv_label_set_text(message_label, "SSR failure");
                break;
            case 0x10:
                lv_label_set_text(message_label, "Watchdog timeout");
                break;
            default:
                lv_label_set_text(message_label, "Unknown error");
                break;
        }
    }
}

void screen_alarm_clear(void) {
    // Just reset labels to default
    if (code_label) {
        lv_label_set_text(code_label, "Code: 0x00");
    }
    if (message_label) {
        lv_label_set_text(message_label, "Alarm cleared");
    }
}

