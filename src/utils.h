/*
 * Copyright 2017 Mike Gabriel <mike.gabriel@das-netzwerkteam.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INDICATOR_SESSION_UTILS_H__
#define __INDICATOR_SESSION_UTILS_H__

#include <glib.h>
#include <string.h>


/* XDG_CURRENT_DESKTOP environment variable values
 * as set by well-known desktop environments
 */
#define DESKTOP_UNITY  "Unity"
#define DESKTOP_MATE   "MATE"
#define DESKTOP_BUDGIE "Budgie:GNOME"
#define DESKTOP_GNOME  "GNOME"
#define DESKTOP_XFCE   "XFCE"

gboolean is_unity();
gboolean is_gnome();
gboolean is_mate();
gboolean is_budgie();
gboolean is_xfce();

const char* get_distro_name();
const char* get_distro_url();
const char* get_distro_bts_url();
const char* get_desktop_name();
GHashTable*  get_os_release();

#endif /* __INDICATOR_SESSION_UTILS_H__ */
