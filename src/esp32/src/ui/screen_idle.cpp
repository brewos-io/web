/**
 * BrewOS Idle Screen Implementation
 * 
 * Power on screen with heating strategy selection
 */

#include <Arduino.h>
#include "ui/screen_idle.h"
#include "display/theme.h"
#include "display/display_config.h"
#include "config.h"

// Strategy names
static const char* strategy_names[] = {
    "Brew Only",
    "Sequential",
    "Steam Priority",
    "Parallel",
    "Smart Stagger"
};

static const char* strategy_descriptions[] = {
    "Heat brew boiler only",
    "Heat brew first, then steam",
    "Heat steam first, then brew",
    "Heat both simultaneously",
    "Stagger for power efficiency"
};

#define STRATEGY_COUNT 5

// Static elements
static lv_obj_t* screen = nullptr;
static lv_obj_t* power_icon = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* strategy_name_label = nullptr;
static lv_obj_t* strategy_desc_label = nullptr;
static lv_obj_t* hint_label = nullptr;
static lv_obj_t* dots_container = nullptr;
static lv_obj_t* strategy_dots[STRATEGY_COUNT] = {nullptr};

// State
static int selected_index = 0;
static idle_turn_on_callback_t turn_on_callback = nullptr;

// =============================================================================
// Screen Creation
// =============================================================================

lv_obj_t* screen_idle_create(void) {
    LOG_I("Creating idle screen...");
    
    // Create screen with dark background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, COLOR_BG_DARK, 0);
    
    // Create main container
    lv_obj_t* container = lv_obj_create(screen);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // === Power Icon (large, pulsing) ===
    power_icon = lv_label_create(container);
    lv_label_set_text(power_icon, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(power_icon, FONT_TEMP, 0);
    lv_obj_set_style_text_color(power_icon, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(power_icon, LV_ALIGN_CENTER, 0, -80);
    
    // Add subtle animation to power icon
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, power_icon);
    lv_anim_set_values(&anim, LV_OPA_50, LV_OPA_COVER);
    lv_anim_set_time(&anim, 1500);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&anim, 1500);
    lv_anim_set_exec_cb(&anim, [](void* obj, int32_t v) {
        lv_obj_set_style_opa((lv_obj_t*)obj, v, 0);
    });
    lv_anim_start(&anim);
    
    // === Title ===
    title_label = lv_label_create(container);
    lv_label_set_text(title_label, "Press to Start");
    lv_obj_set_style_text_font(title_label, FONT_LARGE, 0);
    lv_obj_set_style_text_color(title_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    
    // === Strategy Name ===
    strategy_name_label = lv_label_create(container);
    lv_label_set_text(strategy_name_label, strategy_names[selected_index]);
    lv_obj_set_style_text_font(strategy_name_label, FONT_MEDIUM, 0);
    lv_obj_set_style_text_color(strategy_name_label, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(strategy_name_label, LV_ALIGN_CENTER, 0, 50);
    
    // === Strategy Description ===
    strategy_desc_label = lv_label_create(container);
    lv_label_set_text(strategy_desc_label, strategy_descriptions[selected_index]);
    lv_obj_set_style_text_font(strategy_desc_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(strategy_desc_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(strategy_desc_label, LV_ALIGN_CENTER, 0, 80);
    
    // === Dots indicator ===
    dots_container = lv_obj_create(container);
    lv_obj_remove_style_all(dots_container);
    lv_obj_set_size(dots_container, STRATEGY_COUNT * 20, 20);
    lv_obj_align(dots_container, LV_ALIGN_CENTER, 0, 120);
    lv_obj_set_flex_flow(dots_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dots_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    for (int i = 0; i < STRATEGY_COUNT; i++) {
        strategy_dots[i] = lv_obj_create(dots_container);
        lv_obj_set_size(strategy_dots[i], 8, 8);
        lv_obj_set_style_radius(strategy_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(strategy_dots[i], 0, 0);
        lv_obj_set_style_margin_all(strategy_dots[i], 4, 0);
        
        if (i == selected_index) {
            lv_obj_set_style_bg_color(strategy_dots[i], COLOR_ACCENT_AMBER, 0);
        } else {
            lv_obj_set_style_bg_color(strategy_dots[i], COLOR_BG_ELEVATED, 0);
        }
    }
    
    // === Hint at bottom ===
    hint_label = lv_label_create(container);
    lv_label_set_text(hint_label, LV_SYMBOL_LOOP " Rotate to change strategy");
    lv_obj_set_style_text_font(hint_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(hint_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    LOG_I("Idle screen created");
    return screen;
}

// =============================================================================
// Screen Update
// =============================================================================

void screen_idle_update(const ui_state_t* state) {
    if (!screen || !state) return;
    
    // Update selected strategy from state if not idle (e.g., coming from settings)
    // The current selection is maintained by the screen itself during rotation
}

void screen_idle_select_strategy(int index) {
    // Clamp index
    if (index < 0) index = STRATEGY_COUNT - 1;
    if (index >= STRATEGY_COUNT) index = 0;
    
    selected_index = index;
    
    // Update UI
    if (strategy_name_label) {
        lv_label_set_text(strategy_name_label, strategy_names[selected_index]);
    }
    if (strategy_desc_label) {
        lv_label_set_text(strategy_desc_label, strategy_descriptions[selected_index]);
    }
    
    // Update dots
    for (int i = 0; i < STRATEGY_COUNT; i++) {
        if (strategy_dots[i]) {
            if (i == selected_index) {
                lv_obj_set_style_bg_color(strategy_dots[i], COLOR_ACCENT_AMBER, 0);
            } else {
                lv_obj_set_style_bg_color(strategy_dots[i], COLOR_BG_ELEVATED, 0);
            }
        }
    }
}

heating_strategy_t screen_idle_get_selected_strategy(void) {
    return (heating_strategy_t)selected_index;
}

void screen_idle_set_turn_on_callback(idle_turn_on_callback_t callback) {
    turn_on_callback = callback;
}

