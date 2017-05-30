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

#include "utils.h"

gboolean
is_unity ()
{
  const gchar *xdg_current_desktop;
  gchar **desktop_names;
  int i;

  xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (xdg_current_desktop != NULL) {
    desktop_names = g_strsplit (xdg_current_desktop, ":", 0);
    for (i = 0; desktop_names[i]; ++i) {
      if (!g_strcmp0 (desktop_names[i], "Unity")) {
        g_strfreev (desktop_names);
        return TRUE;
      }
    }
    g_strfreev (desktop_names);
  }
  return FALSE;
}

gboolean
is_gnome ()
{
  const gchar *xdg_current_desktop;
  gchar **desktop_names;
  int i;

  xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (xdg_current_desktop != NULL) {
    desktop_names = g_strsplit (xdg_current_desktop, ":", 0);
    for (i = 0; desktop_names[i]; ++i) {
      if (!g_strcmp0 (desktop_names[i], "GNOME")) {
        g_strfreev (desktop_names);
        return TRUE;
      }
    }
    g_strfreev (desktop_names);
  }
  return FALSE;
}

gboolean
is_mate ()
{
  const gchar *xdg_current_desktop;
  gchar **desktop_names;
  int i;

  xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (xdg_current_desktop != NULL) {
    desktop_names = g_strsplit (xdg_current_desktop, ":", 0);
    for (i = 0; desktop_names[i]; ++i) {
      if (!g_strcmp0 (desktop_names[i], "MATE")) {
        g_strfreev (desktop_names);
        return TRUE;
      }
    }
    g_strfreev (desktop_names);
  }
  return FALSE;
}
