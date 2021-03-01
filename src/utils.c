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

static  gboolean
is_xdg_current_desktop (const gchar* desktop)
{
  const gchar *xdg_current_desktop;
  gchar **desktop_names;
  int i;

  xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
  if (xdg_current_desktop != NULL) {
    desktop_names = g_strsplit (xdg_current_desktop, ":", 0);
    for (i = 0; desktop_names[i]; ++i) {
      if (!g_strcmp0 (desktop_names[i], desktop)) {
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
  return is_xdg_current_desktop(DESKTOP_GNOME);
}

gboolean
is_unity ()
{
  return is_xdg_current_desktop(DESKTOP_UNITY);
}

gboolean
is_mate ()
{
  return is_xdg_current_desktop(DESKTOP_MATE);
}

gboolean
is_xfce ()
{
  return is_xdg_current_desktop(DESKTOP_XFCE);
}

GHashTable*
get_os_release (void)
{
  static const char * const os_release = "/etc/os-release";
  GHashTable * hash;
  GIOChannel * io;

  hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  if ((io = g_io_channel_new_file (os_release, "r", NULL)))
    {
      GString * key = g_string_new (NULL);

      for (;;)
        {
          GIOStatus status;
          char * in;
          GError * error;
          gchar * val;

          /* read a line */
          status = g_io_channel_read_line_string (io, key, NULL, NULL);
          if (status == G_IO_STATUS_EOF)
            break;

          /* ignore blank lines & comments */
          if (!key->len || key->str[0]=='#')
            continue;

          /* split into key=value */
          in = strchr(key->str, '=');
          if (!in)
            continue;
          *in++ = '\0';

          /* unmunge the value component */
          g_strstrip(in); /* eat linefeed */
          error = NULL;
          val = g_shell_unquote (in, &error);
          if (error != NULL)
            {
              g_warning("Unable to unquote \"%s\": %s", in, error->message);
              val = g_strdup(in);
              g_clear_error(&error);
            }

          g_debug("from \"%s\": key [%s] val [%s]", os_release, key->str, val);
          g_hash_table_insert (hash, g_strdup(key->str), val); /* hash owns val now */
        }

      g_string_free(key, TRUE);
      g_io_channel_unref(io);
    }

  return hash;
}

const char*
get_distro_name (void)
{
  static char * distro_name = NULL;

  if (distro_name == NULL)
    {
      GHashTable * os_release = get_os_release();
      gpointer value = g_hash_table_lookup(os_release, "NAME");
      if (value == NULL)
        value = "GNU/Linux"; /* fallback value */
      distro_name = g_strdup(value);
      g_hash_table_destroy(os_release);
    }

  return distro_name;
}

const char*
get_distro_url (void)
{
  static char * distro_url = NULL;

  if (distro_url == NULL)
    {
      GHashTable * os_release = get_os_release();
      gpointer value = g_hash_table_lookup(os_release, "HOME_URL");
      if (value == NULL)
        value = "https://www.gnu.org"; /* fallback value */
      distro_url = g_strdup(value);
      g_hash_table_destroy(os_release);
    }

  return distro_url;
}

const char*
get_distro_bts_url (void)
{
  static char * distro_bts_url = NULL;

  if (distro_bts_url == NULL)
    {
      GHashTable * os_release = get_os_release();
      gpointer value = g_hash_table_lookup(os_release, "BUG_REPORT_URL");
      if (value == NULL)
        value = "https://github.com/AyatanaIndicators/ayatana-indicator-session/issues"; /* fallback value */
      distro_bts_url = g_strdup(value);
      g_hash_table_destroy(os_release);
    }

  return distro_bts_url;
}

const char*
get_desktop_name (void)
{
  static char * desktop_name = NULL;
  const char * xdg_current_desktop;

  if (desktop_name == NULL)
    {
      xdg_current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
      if (xdg_current_desktop != NULL) {
        desktop_name = g_strsplit (xdg_current_desktop, ":", 0)[0];
      }
    }

  return desktop_name;
}
