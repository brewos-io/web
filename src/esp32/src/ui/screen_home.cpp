/**
 * BrewOS Home Screen Implementation
 * 
 * Optimized for 480x480 round display
 */

#include <Arduino.h>
#include "ui/screen_home.h"
#include "display/theme.h"
#include "display/display_config.h"
#include "config.h"

// Static elements
static screen_home_t elements = {0};

// Forward declarations
static void create_temp_display(lv_obj_t* parent, bool is_steam);
static void create_pressure_display(lv_obj_t* parent);
static void create_status_display(lv_obj_t* parent);

// =============================================================================
// Screen Creation
// =============================================================================

lv_obj_t* screen_home_create(void) {
    LOG_I("Creating home screen...");
    
    // Create screen with dark background
    elements.screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(elements.screen, COLOR_BG_DARK, 0);
    
    // Create main container (circular area)
    lv_obj_t* container = lv_obj_create(elements.screen);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // === Status indicator at top ===
    create_status_display(container);
    
    // === Brew Temperature (upper half) ===
    lv_obj_t* brew_section = lv_obj_create(container);
    lv_obj_remove_style_all(brew_section);
    lv_obj_set_size(brew_section, 280, 140);
    lv_obj_align(brew_section, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_bg_opa(brew_section, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(brew_section, LV_OBJ_FLAG_SCROLLABLE);
    
    // Brew icon
    lv_obj_t* brew_icon = lv_label_create(brew_section);
    lv_label_set_text(brew_icon, LV_SYMBOL_CHARGE);  // Using charge as brew icon
    lv_obj_set_style_text_font(brew_icon, FONT_LARGE, 0);
    lv_obj_set_style_text_color(brew_icon, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(brew_icon, LV_ALIGN_TOP_MID, 0, 0);
    
    // Brew temperature label (large)
    elements.brew_temp_label = lv_label_create(brew_section);
    lv_label_set_text(elements.brew_temp_label, "--.-°C");
    lv_obj_set_style_text_font(elements.brew_temp_label, FONT_TEMP, 0);
    lv_obj_set_style_text_color(elements.brew_temp_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(elements.brew_temp_label, LV_ALIGN_CENTER, 0, 10);
    
    // Brew label
    lv_obj_t* brew_label = lv_label_create(brew_section);
    lv_label_set_text(brew_label, "BREW");
    lv_obj_set_style_text_font(brew_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(brew_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(brew_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // === Steam Temperature (lower section) ===
    lv_obj_t* steam_section = lv_obj_create(container);
    lv_obj_remove_style_all(steam_section);
    lv_obj_set_size(steam_section, 200, 80);
    lv_obj_align(steam_section, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_bg_opa(steam_section, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(steam_section, LV_OBJ_FLAG_SCROLLABLE);
    
    // Steam icon
    lv_obj_t* steam_icon = lv_label_create(steam_section);
    lv_label_set_text(steam_icon, LV_SYMBOL_WARNING);  // Steam warning/hot icon
    lv_obj_set_style_text_font(steam_icon, FONT_MEDIUM, 0);
    lv_obj_set_style_text_color(steam_icon, COLOR_TEMP_HOT, 0);
    lv_obj_align(steam_icon, LV_ALIGN_LEFT_MID, 10, 0);
    
    // Steam temperature label
    elements.steam_temp_label = lv_label_create(steam_section);
    lv_label_set_text(elements.steam_temp_label, "---°C");
    lv_obj_set_style_text_font(elements.steam_temp_label, FONT_XLARGE, 0);
    lv_obj_set_style_text_color(elements.steam_temp_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_align(elements.steam_temp_label, LV_ALIGN_CENTER, 20, 0);
    
    // Steam label
    lv_obj_t* steam_label = lv_label_create(steam_section);
    lv_label_set_text(steam_label, "STEAM");
    lv_obj_set_style_text_font(steam_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(steam_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(steam_label, LV_ALIGN_BOTTOM_MID, 20, 0);
    
    // === Pressure Bar (bottom) ===
    create_pressure_display(container);
    
    // === Outer arc for temperature progress ===
    elements.brew_temp_arc = lv_arc_create(container);
    lv_obj_set_size(elements.brew_temp_arc, 440, 440);
    lv_obj_center(elements.brew_temp_arc);
    lv_arc_set_range(elements.brew_temp_arc, 0, 100);
    lv_arc_set_value(elements.brew_temp_arc, 0);
    lv_arc_set_bg_angles(elements.brew_temp_arc, 150, 30);  // Leave gap at bottom
    lv_arc_set_rotation(elements.brew_temp_arc, 0);
    
    // Arc background
    lv_obj_set_style_arc_color(elements.brew_temp_arc, COLOR_ARC_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(elements.brew_temp_arc, 10, LV_PART_MAIN);
    
    // Arc indicator
    lv_obj_set_style_arc_color(elements.brew_temp_arc, COLOR_ACCENT_AMBER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(elements.brew_temp_arc, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(elements.brew_temp_arc, true, LV_PART_INDICATOR);
    
    // Hide knob
    lv_obj_set_style_bg_opa(elements.brew_temp_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    
    // Make arc non-interactive (display only)
    lv_obj_clear_flag(elements.brew_temp_arc, LV_OBJ_FLAG_CLICKABLE);
    
    LOG_I("Home screen created");
    return elements.screen;
}

static void create_status_display(lv_obj_t* parent) {
    // Status container at top
    lv_obj_t* status_container = lv_obj_create(parent);
    lv_obj_remove_style_all(status_container);
    lv_obj_set_size(status_container, 200, 40);
    lv_obj_align(status_container, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_bg_opa(status_container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(status_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // LED indicator
    elements.status_led = lv_led_create(status_container);
    lv_obj_set_size(elements.status_led, 12, 12);
    lv_obj_align(elements.status_led, LV_ALIGN_LEFT_MID, 0, 0);
    lv_led_set_color(elements.status_led, COLOR_SUCCESS);
    lv_led_on(elements.status_led);
    
    // Status text
    elements.status_label = lv_label_create(status_container);
    lv_label_set_text(elements.status_label, "READY");
    lv_obj_set_style_text_font(elements.status_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(elements.status_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(elements.status_label, LV_ALIGN_LEFT_MID, 20, 0);
}

static void create_pressure_display(lv_obj_t* parent) {
    // Pressure container at bottom
    lv_obj_t* pressure_container = lv_obj_create(parent);
    lv_obj_remove_style_all(pressure_container);
    lv_obj_set_size(pressure_container, 280, 50);
    lv_obj_align(pressure_container, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_bg_opa(pressure_container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(pressure_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Pressure bar
    elements.pressure_bar = lv_bar_create(pressure_container);
    lv_obj_set_size(elements.pressure_bar, 200, 8);
    lv_obj_align(elements.pressure_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_bar_set_range(elements.pressure_bar, 0, 15);  // 0-15 bar
    lv_bar_set_value(elements.pressure_bar, 0, LV_ANIM_OFF);
    
    // Bar background
    lv_obj_set_style_bg_color(elements.pressure_bar, COLOR_ARC_BG, LV_PART_MAIN);
    lv_obj_set_style_radius(elements.pressure_bar, 4, LV_PART_MAIN);
    
    // Bar indicator
    lv_obj_set_style_bg_color(elements.pressure_bar, COLOR_PRESSURE_OPTIMAL, LV_PART_INDICATOR);
    lv_obj_set_style_radius(elements.pressure_bar, 4, LV_PART_INDICATOR);
    
    // Pressure label
    elements.pressure_label = lv_label_create(pressure_container);
    lv_label_set_text(elements.pressure_label, "0.0 bar");
    lv_obj_set_style_text_font(elements.pressure_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(elements.pressure_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_align(elements.pressure_label, LV_ALIGN_BOTTOM_MID, 0, 0);
}

// =============================================================================
// Screen Update
// =============================================================================

void screen_home_update(lv_obj_t* screen, const ui_state_t* state) {
    if (!state || !elements.screen) return;
    
    // Update brew temperature
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.1f°C", state->brew_temp);
    lv_label_set_text(elements.brew_temp_label, temp_str);
    
    // Update brew arc (percentage of setpoint)
    if (state->brew_setpoint > 0) {
        int pct = (int)((state->brew_temp / state->brew_setpoint) * 100);
        pct = LV_CLAMP(0, pct, 100);
        lv_arc_set_value(elements.brew_temp_arc, pct);
        
        // Update arc color based on temperature state
        lv_color_t temp_color = theme_get_temp_color(state->brew_temp, state->brew_setpoint);
        lv_obj_set_style_arc_color(elements.brew_temp_arc, temp_color, LV_PART_INDICATOR);
    }
    
    // Update steam temperature
    snprintf(temp_str, sizeof(temp_str), "%.0f°C", state->steam_temp);
    lv_label_set_text(elements.steam_temp_label, temp_str);
    
    // Update pressure
    snprintf(temp_str, sizeof(temp_str), "%.1f bar", state->pressure);
    lv_label_set_text(elements.pressure_label, temp_str);
    lv_bar_set_value(elements.pressure_bar, (int)(state->pressure * 10) / 10, LV_ANIM_ON);
    
    // Update pressure bar color
    lv_color_t pressure_color = theme_get_pressure_color(state->pressure);
    lv_obj_set_style_bg_color(elements.pressure_bar, pressure_color, LV_PART_INDICATOR);
    
    // Update status
    const char* status_text = "READY";
    lv_color_t status_color = COLOR_SUCCESS;
    
    if (state->alarm_active) {
        status_text = "ALARM";
        status_color = COLOR_ERROR;
    } else if (state->water_low) {
        status_text = "LOW WATER";
        status_color = COLOR_WARNING;
    } else if (state->is_brewing) {
        status_text = "BREWING";
        status_color = COLOR_ACCENT_ORANGE;
    } else if (state->is_heating) {
        status_text = "HEATING";
        status_color = COLOR_WARNING;
    } else if (!state->pico_connected) {
        status_text = "DISCONNECTED";
        status_color = COLOR_ERROR;
    }
    
    lv_label_set_text(elements.status_label, status_text);
    lv_led_set_color(elements.status_led, status_color);
}

screen_home_t* screen_home_get_elements(void) {
    return &elements;
}

