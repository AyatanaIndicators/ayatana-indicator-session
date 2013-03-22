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

#ifndef MOCK_CONSOLEKIT_SEAT_H
#define MOCK_CONSOLEKIT_SEAT_H

#include <set>
#include <string>
#include "backend-dbus/dbus-consolekit-seat.h"
#include "mock-object.h"

class MockUser;
class MockConsoleKitSession;

class MockConsoleKitSeat: public MockObject
{
  public:

    MockConsoleKitSeat (GMainLoop       * loop,
                        GDBusConnection * bus_connection,
                        bool              can_activate_sessions);
    virtual ~MockConsoleKitSeat ();

    const char * sid() { return path(); }
    MockConsoleKitSession * add_session_by_user (MockUser * user);
    void add_session (MockConsoleKitSession * session);
    void remove_session (MockConsoleKitSession * session);
    void activate_session (MockConsoleKitSession * session);
    void switch_to_guest ();
    void switch_to_user (const char * username);
    bool can_activate_sessions () const { return my_can_activate_sessions; }
    MockConsoleKitSession * find (const char * ssid);

  private:

    static gboolean on_get_active_session (ConsoleKitSeat        * cks,
                                           GDBusMethodInvocation * inv,
                                           gpointer                gself);
    static gboolean on_get_sessions (ConsoleKitSeat         * cks,
                                     GDBusMethodInvocation  * inv,
                                     gpointer                 gself);
    static gboolean on_can_activate_sessions (ConsoleKitSeat        * cks,
                                              GDBusMethodInvocation * inv,
                                              gpointer                gself);


  private:

    ConsoleKitSeat * my_skeleton;

    std::string my_active_ssid;

    typedef std::set<MockConsoleKitSession*> sessions_t;
    sessions_t my_sessions;
    bool my_can_activate_sessions;

};

#endif // #ifndef MOCK_CONSOLEKIT_SEAT_H
