/*
This is the c configuration file for the keymap

Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert
Copyright 2020 Ben Roesner (keycapsss.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Sync layer_state to the slave half so its OLED can show the real layer
// instead of always reading 0 (_QWERTY). Harmless if OLED_ENABLE is off.
#define SPLIT_LAYER_STATE_ENABLE

// Turn the OLEDs off after a minute of inactivity, back on on next keypress.
// Only takes effect if OLED_ENABLE=yes in rules.mk.
#define OLED_TIMEOUT 60000
