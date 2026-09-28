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
#pragma once

/**
 * Layout of the Pick and Place status screen for 480x320 displays
 *
 *   y=4    | PUMP | VALVE | VACUUM | LIGHT |   Tiles
 *   y=132  | X    | Y     | Z      | A     |   Position
 *   y=180  | Home |   Feedrate     | Menu  |   Controls
 *   y=280  |        Status message         |
 */

// Tiles
#define PNP_TILE_COUNT            4
#define PNP_TILE_PITCH            (TFT_WIDTH / PNP_TILE_COUNT)
#define PNP_TILE_MARGIN           4
#define PNP_TILE_W                (PNP_TILE_PITCH - 2 * PNP_TILE_MARGIN)
#define PNP_TILE_H                120
#define PNP_TILE_X(N)             ((N) * PNP_TILE_PITCH + PNP_TILE_MARGIN)
#define PNP_TILE_Y                4

#define PNP_TILE_LABEL_H          30
#define PNP_TILE_ICON_X           ((PNP_TILE_W - 64) / 2)
#define PNP_TILE_ICON_Y           26
#define PNP_TILE_STATE_Y          88
#define PNP_TILE_STATE_H          30

// Vacuum tile
#define PNP_VACUUM_VALUE_Y        38
#define PNP_VACUUM_UNIT_Y         64
#define PNP_VACUUM_BAR_X          8
#define PNP_VACUUM_BAR_Y          100
#define PNP_VACUUM_BAR_W          (PNP_TILE_W - 2 * PNP_VACUUM_BAR_X)
#define PNP_VACUUM_BAR_H          12

// Position
#define PNP_POS_X                 4
#define PNP_POS_Y                 132
#define PNP_POS_W                 (TFT_WIDTH - 8)
#define PNP_POS_H                 FONT_LINE_HEIGHT
#define PNP_POS_CELL_W            (PNP_POS_W / 4)
#define PNP_POS_MARK_X            8
#define PNP_POS_VALUE_RIGHT       (PNP_POS_CELL_W - 8)

// Controls
#define PNP_HOME_X                12
#define PNP_HOME_Y                180
#define PNP_FEEDRATE_W            120
#define PNP_FEEDRATE_H            32
#define PNP_FEEDRATE_X            ((TFT_WIDTH - PNP_FEEDRATE_W) / 2)
#define PNP_FEEDRATE_Y            196
#define PNP_MENU_X                404
#define PNP_MENU_Y                180

// Colors
#define COLOR_PNP_ON              COLOR_VIVID_GREEN
#define COLOR_PNP_OFF             COLOR_GREY
#define COLOR_PNP_LABEL           COLOR_WHITE
#define COLOR_PNP_VACUUM          COLOR_CYAN
#define COLOR_PNP_PICKED          COLOR_VIVID_GREEN
