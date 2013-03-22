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

#ifndef MOCK_CONSOLEKIT_SESSION_H
#define MOCK_CONSOLEKIT_SESSION_H

#include <string>
#include "mock-object.h"
#include "backend-dbus/dbus-consolekit-session.h"

class MockUser;

class MockConsoleKitSession: public MockObject
{
  public:

    MockConsoleKitSession (GMainLoop       * loop,
                           GDBusConnection * bus_connection);
    virtual ~MockConsoleKitSession ();

    MockUser * user () { return my_user; }
    void set_user (MockUser * user);
    const char * ssid () { return path(); }
    void set_sid (const std::string& sid) { my_sid = sid; }
    const char * x11_display() { return my_x11_display.c_str(); }
    void set_x11_display (const std::string& x) { my_x11_display = x; }

  private:

    static gboolean on_get_seat_id_static (ConsoleKitSession *,
                                           GDBusMethodInvocation *,
                                           gpointer);
    static gboolean on_get_unix_user_static (ConsoleKitSession *,
                                             GDBusMethodInvocation *,
                                             gpointer);
    static gboolean on_get_x11_display (ConsoleKitSession *,
                                        GDBusMethodInvocation *, 
                                        gpointer);


  private:

    ConsoleKitSession * my_skeleton;
    std::string my_sid;
    std::string my_x11_display;
    MockUser * my_user;
};

#endif // #ifndef MOCK_CONSOLEKIT_SESSION_H
