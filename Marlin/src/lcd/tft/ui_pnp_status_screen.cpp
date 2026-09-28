/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2026 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * Pick and Place status screen for TFT_COLOR_UI
 */

#include "../../inc/MarlinConfig.h"

#if ALL(TFT_COLOR_UI, PNP_STATUS_SCREEN)

#if !HAS_UI_480x320 || ENABLED(TFT_COLOR_UI_PORTRAIT)
  #error "PNP_STATUS_SCREEN requires a 480x320 landscape display."
#elif FAN_COUNT <= PNP_PUMP_FAN || FAN_COUNT <= PNP_VALVE_FAN || FAN_COUNT <= PNP_LIGHT_FAN
  #error "PNP_PUMP_FAN, PNP_VALVE_FAN and PNP_LIGHT_FAN must be existing fan indexes."
#endif

#include "ui_common.h"
#include "ui_pnp_480x320.h"

#include "../marlinui.h"
#include "../menu/menu.h"
#include "../../libs/numtostr.h"

#include "../../gcode/queue.h"
#include "../../module/temperature.h"
#include "../../module/motion.h"

//
// Outputs
//

inline bool pnp_output_is_on(const uint8_t fan) { return fans[fan].speed > 0; }

#if ENABLED(TOUCH_SCREEN)

  static void toggle_output(const uint8_t fan) {
    quick_feedback();
    thermalManager.set_fan_speed(fan, pnp_output_is_on(fan) ? 0 : 255);
    ui.refresh();
  }

  static void toggle_pump()  { toggle_output(PNP_PUMP_FAN); }
  static void toggle_valve() { toggle_output(PNP_VALVE_FAN); }
  static void toggle_light() { toggle_output(PNP_LIGHT_FAN); }

  static void home_all() {
    quick_feedback();
    queue.inject_P(G28_STR);
  }

#endif

//
// Vacuum level in kPa (0 = atmospheric, negative = vacuum)
//

#if ENABLED(PNP_VACUUM_SIMULATED)

  // No sensor hardware. The level follows a target chosen by the pump / valve state.
  static float pnp_vacuum_kpa() {
    static float level = 0;
    static millis_t last_ms = 0;

    const float target = !pnp_output_is_on(PNP_PUMP_FAN) ? 0.0f
                       : pnp_output_is_on(PNP_VALVE_FAN) ? -60.0f : -15.0f;

    const millis_t ms = millis();
    const float dt = _MIN((ms - last_ms) * 0.001f, 1.0f);
    last_ms = ms;

    level += (target - level) * _MIN(dt * 3.0f, 1.0f);  // ~0.3s time constant
    return level;
  }

#else

  #error "PNP_STATUS_SCREEN currently requires PNP_VACUUM_SIMULATED. Reading a vacuum sensor is not implemented."

#endif

//
// Tiles
//

static void draw_tile_frame(const uint8_t index, FSTR_P const label, const uint16_t color) {
  tft.canvas(PNP_TILE_X(index), PNP_TILE_Y, PNP_TILE_W, PNP_TILE_H);
  tft.set_background(COLOR_BACKGROUND);
  tft.add_rectangle(0, 0, PNP_TILE_W, PNP_TILE_H, color);

  tft_string.set(label);
  tft.add_text(tft_string.center(PNP_TILE_W), tft_string.vcenter(PNP_TILE_LABEL_H), COLOR_PNP_LABEL, tft_string);
}

static void draw_output_tile(const uint8_t index, FSTR_P const label, const MarlinImage image, const uint8_t fan, void (*toggle)()) {
  const bool on = pnp_output_is_on(fan);
  const uint16_t color = on ? COLOR_PNP_ON : COLOR_PNP_OFF;

  draw_tile_frame(index, label, color);

  tft.add_image(PNP_TILE_ICON_X, PNP_TILE_ICON_Y, image, color);
  tft_string.set(on ? F("ON") : F("OFF"));
  tft.add_text(tft_string.center(PNP_TILE_W), PNP_TILE_STATE_Y + tft_string.vcenter(PNP_TILE_STATE_H), color, tft_string);

  #if ENABLED(TOUCH_SCREEN)
    touch.add_control(BUTTON, PNP_TILE_X(index), PNP_TILE_Y, PNP_TILE_W, PNP_TILE_H, toggle);
  #else
    UNUSED(toggle);
  #endif
}

static void draw_vacuum_tile(const uint8_t index) {
  const float kpa = pnp_vacuum_kpa();
  const bool picked = kpa <= PNP_VACUUM_PICKED_KPA;
  const uint16_t color = picked ? COLOR_PNP_PICKED : COLOR_PNP_VACUUM;

  draw_tile_frame(index, F("VACUUM"), picked ? COLOR_PNP_PICKED : COLOR_PNP_OFF);

  tft_string.set(i16tostr3rj(int16_t(kpa - 0.5f)));
  tft_string.trim();
  tft.add_text(tft_string.center(PNP_TILE_W), PNP_VACUUM_VALUE_Y, color, tft_string);

  tft_string.set(F("kPa"));
  tft.add_text(tft_string.center(PNP_TILE_W), PNP_VACUUM_UNIT_Y, color, tft_string);

  const float ratio = constrain(kpa / (PNP_VACUUM_MIN_KPA), 0.0f, 1.0f);
  tft.add_rectangle(PNP_VACUUM_BAR_X, PNP_VACUUM_BAR_Y, PNP_VACUUM_BAR_W, PNP_VACUUM_BAR_H, COLOR_PROGRESS_FRAME);
  const uint16_t bar_w = (PNP_VACUUM_BAR_W - 2) * ratio;
  if (bar_w)
    tft.add_bar(PNP_VACUUM_BAR_X + 1, PNP_VACUUM_BAR_Y + 1, bar_w, PNP_VACUUM_BAR_H - 2, color);
}

//
// Position
//

static void draw_axis_cell(const uint8_t cell, const char mark, const char * const value, const uint16_t color) {
  const uint16_t x = cell * (PNP_POS_CELL_W);
  tft_string.set(mark);
  tft.add_text(x + PNP_POS_MARK_X, VCENTER, COLOR_AXIS_HOMED, tft_string);
  tft_string.set(value);
  tft_string.trim();
  tft.add_text(x + PNP_POS_VALUE_RIGHT - tft_string.width(), VCENTER, color, tft_string);
}

static void draw_linear_axis(const uint8_t cell, const char mark, const AxisEnum axis, const float value, const bool blink) {
  const bool nh = motion.axis_should_home(axis);
  draw_axis_cell(cell, mark, blink && nh ? "?" : ftostr42_52(value), nh ? COLOR_AXIS_NOT_HOMED : COLOR_AXIS_HOMED);
}

static void draw_position(const bool blink) {
  #if ALL(MOVE_AXIS_SCREEN, TOUCH_SCREEN)
    touch.add_control(MENU_SCREEN, PNP_POS_X, PNP_POS_Y, PNP_POS_W, PNP_POS_H, (intptr_t)ui.move_axis_screen);
  #endif

  tft.canvas(PNP_POS_X, PNP_POS_Y, PNP_POS_W, PNP_POS_H);
  tft.set_background(COLOR_BACKGROUND);
  tft.add_rectangle(0, 0, PNP_POS_W, PNP_POS_H, COLOR_AXIS_HOMED);

  TERN_(HAS_X_AXIS, draw_linear_axis(0, 'X', X_AXIS, motion.logical_x(motion.position.x), blink));
  TERN_(HAS_Y_AXIS, draw_linear_axis(1, 'Y', Y_AXIS, motion.logical_y(motion.position.y), blink));
  TERN_(HAS_Z_AXIS, draw_linear_axis(2, 'Z', Z_AXIS, motion.logical_z(motion.position.z), blink));

  // The rotational axis has no home sensor, so it is always shown as valid
  TERN_(HAS_I_AXIS, draw_axis_cell(3, AXIS4_NAME, ftostr51sign(motion.logical_i(motion.position.i)), COLOR_AXIS_HOMED));
}

//
// Status Screen
//

void MarlinUI::draw_status_screen() {
  const bool blink = get_blink();
  TERN_(TOUCH_SCREEN, touch.clear());

  draw_output_tile(0, F("PUMP"),  imgPnpPump,  PNP_PUMP_FAN,  TERN(TOUCH_SCREEN, toggle_pump,  nullptr));
  draw_output_tile(1, F("VALVE"), imgPnpValve, PNP_VALVE_FAN, TERN(TOUCH_SCREEN, toggle_valve, nullptr));
  draw_vacuum_tile(2);
  draw_output_tile(3, F("LIGHT"), imgPnpLight, PNP_LIGHT_FAN, TERN(TOUCH_SCREEN, toggle_light, nullptr));

  draw_position(blink);

  // Feed rate
  tft.canvas(PNP_FEEDRATE_X, PNP_FEEDRATE_Y, PNP_FEEDRATE_W, PNP_FEEDRATE_H);
  tft.set_background(COLOR_BACKGROUND);
  const uint16_t color = motion.feedrate_percentage == 100 ? COLOR_RATE_100 : COLOR_RATE_ALTERED;
  tft.add_image(0, 0, imgFeedRate, color);
  tft_string.set(i16tostr3rj(motion.feedrate_percentage));
  tft_string.add('%');
  tft.add_text(36, tft_string.vcenter(30), color, tft_string);
  TERN_(TOUCH_SCREEN, touch.add_control(FEEDRATE, PNP_FEEDRATE_X, PNP_FEEDRATE_Y, PNP_FEEDRATE_W, PNP_FEEDRATE_H));

  #if ENABLED(TOUCH_SCREEN)
    add_control(PNP_HOME_X, PNP_HOME_Y, BUTTON, home_all, imgHome);
    add_control(PNP_MENU_X, PNP_MENU_Y, menu_main, imgSettings);
  #endif

  // Status message
  tft.canvas(STATUS_MESSAGE_X, STATUS_MESSAGE_Y, STATUS_MESSAGE_W, STATUS_MESSAGE_H);
  tft.set_background(COLOR_BACKGROUND);
  tft_string.set(status_message);
  tft_string.trim();
  tft.add_text(STATUS_MESSAGE_TEXT_X, STATUS_MESSAGE_TEXT_Y, COLOR_STATUS_MESSAGE, tft_string);
}

#endif // TFT_COLOR_UI && PNP_STATUS_SCREEN
