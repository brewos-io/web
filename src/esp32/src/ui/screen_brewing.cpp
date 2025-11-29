/**
 * BrewOS Brewing Screen Implementation
 * 
 * Active shot display with timer, weight, and flow rate
 */

#include "platform/platform.h"
#include "ui/screen_brewing.h"
#include "display/theme.h"
#include "display/display_config.h"

// Static elements
static lv_obj_t* screen = nullptr;
static lv_obj_t* timer_label = nullptr;
static lv_obj_t* weight_label = nullptr;
static lv_obj_t* weight_target_label = nullptr;
static lv_obj_t* flow_label = nullptr;
static lv_obj_t* progress_arc = nullptr;
static lv_obj_t* status_label = nullptr;
static lv_obj_t* pressure_label = nullptr;
static lv_obj_t* ratio_label = nullptr;

// =============================================================================
// Screen Creation
// =============================================================================

lv_obj_t* screen_brewing_create(void) {
    LOG_I("Creating brewing screen...");
    
    // Create screen with dark background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, COLOR_BG_DARK, 0);
    
    // Create main container
    lv_obj_t* container = lv_obj_create(screen);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // === Progress Arc (outer ring) ===
    progress_arc = lv_arc_create(container);
    lv_obj_set_size(progress_arc, 440, 440);
    lv_obj_center(progress_arc);
    lv_arc_set_range(progress_arc, 0, 100);
    lv_arc_set_value(progress_arc, 0);
    lv_arc_set_bg_angles(progress_arc, 135, 45);
    lv_arc_set_rotation(progress_arc, 0);
    
    // Arc background
    lv_obj_set_style_arc_color(progress_arc, COLOR_ARC_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(progress_arc, 18, LV_PART_MAIN);
    
    // Arc indicator (orange during brew)
    lv_obj_set_style_arc_color(progress_arc, COLOR_ACCENT_ORANGE, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(progress_arc, 18, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(progress_arc, true, LV_PART_INDICATOR);
    
    // Hide knob
    lv_obj_set_style_bg_opa(progress_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(progress_arc, LV_OBJ_FLAG_CLICKABLE);
    
    // === Status at top ===
    status_label = lv_label_create(container);
    lv_label_set_text(status_label, "BREWING");
    lv_obj_set_style_text_font(status_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(status_label, COLOR_ACCENT_ORANGE, 0);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 50);
    
    // === Timer (large, central) ===
    timer_label = lv_label_create(container);
    lv_label_set_text(timer_label, "00:00");
    lv_obj_set_style_text_font(timer_label, FONT_TEMP, 0);
    lv_obj_set_style_text_color(timer_label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(timer_label, LV_ALIGN_CENTER, 0, -50);
    
    // === Current Weight (large) ===
    weight_label = lv_label_create(container);
    lv_label_set_text(weight_label, "0.0g");
    lv_obj_set_style_text_font(weight_label, FONT_XLARGE, 0);
    lv_obj_set_style_text_color(weight_label, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(weight_label, LV_ALIGN_CENTER, 0, 20);
    
    // === Target Weight ===
    weight_target_label = lv_label_create(container);
    lv_label_set_text(weight_target_label, "/ 36.0g");
    lv_obj_set_style_text_font(weight_target_label, FONT_MEDIUM, 0);
    lv_obj_set_style_text_color(weight_target_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(weight_target_label, LV_ALIGN_CENTER, 0, 55);
    
    // === Flow Rate ===
    flow_label = lv_label_create(container);
    lv_label_set_text(flow_label, "0.0 ml/s");
    lv_obj_set_style_text_font(flow_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(flow_label, COLOR_TEXT_MUTED, 0);
    lv_obj_align(flow_label, LV_ALIGN_CENTER, 0, 90);
    
    // === Pressure (bottom left) ===
    pressure_label = lv_label_create(container);
    lv_label_set_text(pressure_label, "9.0 bar");
    lv_obj_set_style_text_font(pressure_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(pressure_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_align(pressure_label, LV_ALIGN_BOTTOM_LEFT, 60, -60);
    
    // Pressure icon
    lv_obj_t* pressure_icon = lv_label_create(container);
    lv_label_set_text(pressure_icon, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_font(pressure_icon, FONT_SMALL, 0);
    lv_obj_set_style_text_color(pressure_icon, COLOR_TEXT_MUTED, 0);
    lv_obj_align(pressure_icon, LV_ALIGN_BOTTOM_LEFT, 60, -80);
    
    // === Ratio (bottom right) ===
    ratio_label = lv_label_create(container);
    lv_label_set_text(ratio_label, "1:0.0");
    lv_obj_set_style_text_font(ratio_label, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(ratio_label, COLOR_TEXT_SECONDARY, 0);
    lv_obj_align(ratio_label, LV_ALIGN_BOTTOM_RIGHT, -60, -60);
    
    // Ratio label
    lv_obj_t* ratio_title = lv_label_create(container);
    lv_label_set_text(ratio_title, "Ratio");
    lv_obj_set_style_text_font(ratio_title, FONT_SMALL, 0);
    lv_obj_set_style_text_color(ratio_title, COLOR_TEXT_MUTED, 0);
    lv_obj_align(ratio_title, LV_ALIGN_BOTTOM_RIGHT, -60, -80);
    
    LOG_I("Brewing screen created");
    return screen;
}

// =============================================================================
// Screen Update
// =============================================================================

void screen_brewing_update(const ui_state_t* state) {
    if (!screen || !state) return;
    
    // Update timer
    uint32_t secs = state->brew_time_ms / 1000;
    uint32_t mins = secs / 60;
    secs = secs % 60;
    
    char timer_str[16];
    snprintf(timer_str, sizeof(timer_str), "%02lu:%02lu", (unsigned long)mins, (unsigned long)secs);
    lv_label_set_text(timer_label, timer_str);
    
    // Update weight
    char weight_str[16];
    snprintf(weight_str, sizeof(weight_str), "%.1fg", state->brew_weight);
    lv_label_set_text(weight_label, weight_str);
    
    // Update target weight
    char target_str[16];
    snprintf(target_str, sizeof(target_str), "/ %.1fg", state->target_weight);
    lv_label_set_text(weight_target_label, target_str);
    
    // Update flow rate
    char flow_str[16];
    snprintf(flow_str, sizeof(flow_str), "%.1f ml/s", state->flow_rate);
    lv_label_set_text(flow_label, flow_str);
    
    // Update pressure
    char pressure_str[16];
    snprintf(pressure_str, sizeof(pressure_str), "%.1f bar", state->pressure);
    lv_label_set_text(pressure_label, pressure_str);
    
    // Update pressure color based on value
    lv_color_t pressure_color = theme_get_pressure_color(state->pressure);
    lv_obj_set_style_text_color(pressure_label, pressure_color, 0);
    
    // Update ratio (if dose_weight is set)
    if (state->dose_weight > 0 && state->brew_weight > 0) {
        float ratio = state->brew_weight / state->dose_weight;
        char ratio_str[16];
        snprintf(ratio_str, sizeof(ratio_str), "1:%.1f", ratio);
        lv_label_set_text(ratio_label, ratio_str);
    }
    
    // Update progress arc (based on weight target)
    if (state->target_weight > 0) {
        int pct = (int)((state->brew_weight / state->target_weight) * 100);
        pct = LV_CLAMP(0, pct, 100);
        lv_arc_set_value(progress_arc, pct);
        
        // Change arc color when approaching target
        if (pct >= 90) {
            lv_obj_set_style_arc_color(progress_arc, COLOR_SUCCESS, LV_PART_INDICATOR);
        } else if (pct >= 75) {
            lv_obj_set_style_arc_color(progress_arc, COLOR_WARNING, LV_PART_INDICATOR);
        } else {
            lv_obj_set_style_arc_color(progress_arc, COLOR_ACCENT_ORANGE, LV_PART_INDICATOR);
        }
    }
    
    // Update status text based on scale connection
    if (!state->scale_connected) {
        lv_label_set_text(status_label, "BREWING (No Scale)");
        lv_label_set_text(weight_label, "--.-g");
    } else {
        lv_label_set_text(status_label, "BREWING");
    }
}

void screen_brewing_reset(void) {
    if (!screen) return;
    
    lv_label_set_text(timer_label, "00:00");
    lv_label_set_text(weight_label, "0.0g");
    lv_arc_set_value(progress_arc, 0);
    lv_obj_set_style_arc_color(progress_arc, COLOR_ACCENT_ORANGE, LV_PART_INDICATOR);
}

