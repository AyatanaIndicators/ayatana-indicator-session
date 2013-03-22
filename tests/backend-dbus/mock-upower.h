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

#ifndef MOCK_UPOWER_H
#define MOCK_UPOWER_H

#include "mock-object.h" // parent class
#include "backend-dbus/dbus-upower.h" // UPower

class MockUPower: public MockObject
{
  public:

    MockUPower (GMainLoop       * loop,
                GDBusConnection * bus_connection);
    virtual ~MockUPower ();

    void set_can_suspend (bool b) { upower_set_can_suspend (my_skeleton, b); }
    void set_can_hibernate (bool b) { upower_set_can_hibernate (my_skeleton, b); }

    bool suspend_allowed () const { return my_suspend_allowed; }
    bool hibernate_allowed () const { return my_suspend_allowed; }
    bool can_suspend () const { return upower_get_can_suspend (my_skeleton); }
    bool can_hibernate () const { return upower_get_can_hibernate (my_skeleton); }

  public:

    enum Action { None, Suspend, Hibernate };
    Action last_action () { return my_last_action; }

  private:

    UPower * my_skeleton;
    bool my_can_suspend;
    bool my_can_hibernate;
    bool my_suspend_allowed;
    bool my_hibernate_allowed;
    Action my_last_action;

    static gboolean handle_suspend_allowed (UPower *,
                                            GDBusMethodInvocation *,
                                            gpointer);
    static gboolean handle_suspend (UPower *,
                                    GDBusMethodInvocation *,
                                    gpointer);

    static gboolean handle_hibernate_allowed (UPower *,
                                              GDBusMethodInvocation *,
                                              gpointer);
    static gboolean handle_hibernate (UPower *,
                                      GDBusMethodInvocation *,
                                      gpointer);

};

#endif
