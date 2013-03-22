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

#include "mock-consolekit-session.h"
#include "mock-user.h"

namespace
{
  const char * const DEFAULT_X11_DISPLAY = ":0:0";

  const char * const CONSOLEKIT_NAME = "org.freedesktop.ConsoleKit";

  std::string next_unique_ssid ()
  {
    static int id = 333; // arbitrary

    char * tmp;
    std::string ret;

    tmp = g_strdup_printf ("/org/freedesktop/ConsoleKit/Session%d", id++);
    ret = tmp;
    g_free (tmp);
    return ret;
  }
}

void
MockConsoleKitSession :: set_user (MockUser * user)
{
  my_user = user;
}

/***
****
***/

gboolean
MockConsoleKitSession :: on_get_seat_id_static (ConsoleKitSession     * cks,
                                                GDBusMethodInvocation * inv,
                                                gpointer                gself)
{
  const std::string& sid = static_cast<MockConsoleKitSession*>(gself)->my_sid;
  g_debug ("%s %s returning seat id of %s", G_STRLOC, G_STRFUNC, sid.c_str());
  console_kit_session_complete_get_seat_id (cks, inv, sid.c_str());
  return true;
}

gboolean
MockConsoleKitSession :: on_get_unix_user_static (ConsoleKitSession     * cks,
                                                  GDBusMethodInvocation * inv,
                                                  gpointer                gself)
{
  MockUser * user = static_cast<MockConsoleKitSession*>(gself)->my_user;
  g_debug ("%s %s returning uid of %u", G_STRLOC, G_STRFUNC, user->uid());
  console_kit_session_complete_get_unix_user (cks, inv, user->uid());
  return true;
}

gboolean
MockConsoleKitSession :: on_get_x11_display (ConsoleKitSession     * cks,
                                             GDBusMethodInvocation * inv,
                                             gpointer                gself)
{
  MockConsoleKitSession * self = static_cast<MockConsoleKitSession*>(gself);
  const char * x11 = self->x11_display();
  g_debug ("%s %s returning x11 display '%s'", G_STRLOC, G_STRFUNC, x11);
  console_kit_session_complete_get_x11_display (cks, inv, x11);
  return true;
}

/***
****
***/

MockConsoleKitSession :: MockConsoleKitSession (GMainLoop       * loop,
                                                GDBusConnection * conn):
  MockObject (loop, conn, CONSOLEKIT_NAME, next_unique_ssid ()),
  my_skeleton (console_kit_session_skeleton_new ()),
  my_x11_display (DEFAULT_X11_DISPLAY),
  my_user (0)
{
  g_signal_connect (my_skeleton, "handle-get-seat-id",
                    G_CALLBACK(on_get_seat_id_static), this);
  g_signal_connect (my_skeleton, "handle-get-unix-user",
                    G_CALLBACK(on_get_unix_user_static), this);
  g_signal_connect (my_skeleton, "handle-get-x11-display",
                    G_CALLBACK(on_get_x11_display), this);

  set_skeleton (G_DBUS_INTERFACE_SKELETON(my_skeleton));
}

MockConsoleKitSession :: ~MockConsoleKitSession ()
{
  const int n = g_signal_handlers_disconnect_by_data (my_skeleton, this);
  g_assert (n == 3);
  g_clear_object (&my_skeleton);
}
