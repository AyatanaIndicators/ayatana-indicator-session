/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
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

#include "mock-end-session-dialog.h"

#if 0
gboolean
MockEndSessionDialog :: handle_lock (GnomeScreenSaver      * ss,
                                GDBusMethodInvocation * inv,
                                gpointer                gself)
{
  static_cast<MockEndSessionDialog*>(gself)->my_last_action = Lock;
  gnome_screen_saver_complete_lock (ss, inv);
  return true;
}

gboolean
MockEndSessionDialog :: handle_simulate_user_activity (GnomeScreenSaver      * ss,
                                                  GDBusMethodInvocation * inv,
                                                  gpointer                gself)
{
  static_cast<MockEndSessionDialog*>(gself)->my_last_action = UserActivity;
  gnome_screen_saver_complete_simulate_user_activity (ss, inv);
  return true;
}
#endif

gboolean
MockEndSessionDialog :: handle_open (EndSessionDialog      * object,
                                     GDBusMethodInvocation * invocation,
                                     guint                   arg_type G_GNUC_UNUSED,
                                     guint                   arg_timestamp G_GNUC_UNUSED,
                                     guint                   arg_seconds_to_stay_open G_GNUC_UNUSED,
                                     const gchar * const   * inhibitor_paths G_GNUC_UNUSED,
                                     gpointer                gself)
{
  static_cast<MockEndSessionDialog*>(gself)->my_isOpen = true;
  end_session_dialog_complete_open (object, invocation);
  return true;
}

/***
****
***/

namespace
{
  const char * const MY_NAME = "com.canonical.Unity";
  const char * const MY_PATH = "/org/gnome/SessionManager/EndSessionDialog";
}

MockEndSessionDialog :: MockEndSessionDialog (GMainLoop       * loop,
                                              GDBusConnection * bus_connection):
  MockObject (loop, bus_connection, MY_NAME, MY_PATH),
  my_skeleton (end_session_dialog_skeleton_new ()),
  my_isOpen (false)
{
  g_signal_connect (my_skeleton, "handle-open",
                    G_CALLBACK(handle_open), this);
#if 0
  g_signal_connect (my_skeleton, "handle-lock",
                    G_CALLBACK(handle_lock), this);
  g_signal_connect (my_skeleton, "handle-simulate-user-activity",
                    G_CALLBACK(handle_simulate_user_activity), this);
#endif

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockEndSessionDialog :: ~MockEndSessionDialog ()
{
  g_clear_object (&my_skeleton);
}
