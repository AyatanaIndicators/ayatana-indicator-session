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

#ifndef MOCK_CONSOLEKIT_MANAGER_H
#define MOCK_CONSOLEKIT_MANAGER_H

#include <set>
#include <string>
#include "mock-object.h"
#include "backend-dbus/dbus-consolekit-manager.h"

class MockConsoleKitSession;
class MockConsoleKitSeat;

class MockConsoleKitManager: public MockObject
{
  public:

    MockConsoleKitManager (GMainLoop       * loop,
                           GDBusConnection * bus_connection);
    virtual ~MockConsoleKitManager ();

    void add_seat (MockConsoleKitSeat * seat);

    MockConsoleKitSession * current_session ();

  public:

    enum Action { None, Shutdown, Restart };

    Action last_action () const { return my_last_action; }

    void clear_last_action () { my_last_action = None; }

  private:

    typedef std::set<MockConsoleKitSeat*> seats_t;
    seats_t my_seats;

    ConsoleKitManager * my_skeleton;

    std::string my_current_ssid;

    Action my_last_action;

    static gboolean on_get_current_session (ConsoleKitManager *,
                                            GDBusMethodInvocation *,
                                            gpointer );
    static gboolean on_get_seats (ConsoleKitManager *,
                                  GDBusMethodInvocation *,
                                  gpointer );
    static gboolean handle_restart (ConsoleKitManager *,
                                    GDBusMethodInvocation *,
                                    gpointer);
    static gboolean handle_stop (ConsoleKitManager *,
                                 GDBusMethodInvocation *,
                                 gpointer);

};

#endif // #ifndef MOCK_CONSOLEKIT_MANAGER_H
