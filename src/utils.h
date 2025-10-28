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

const char *get_distro_name(void);
const char *get_distro_url(void);
const char *get_distro_bts_url(void);
const char *get_desktop_name(void);
const char *get_desktop_session(void);
GHashTable *get_os_release(void);

#endif /* __INDICATOR_SESSION_UTILS_H__ */
