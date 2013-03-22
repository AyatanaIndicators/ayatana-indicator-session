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

#include "gtest-dbus-fixture.h"

#include "mock-accounts.h"
#include "mock-consolekit-manager.h"
#include "mock-consolekit-seat.h"
#include "mock-consolekit-session.h"
#include "mock-display-manager-seat.h"
#include "mock-end-session-dialog.h"
#include "mock-screen-saver.h"
#include "mock-session-manager.h"
#include "mock-upower.h"
#include "mock-user.h"
#include "mock-webcredentials.h"

/***
****
***/

class GTestMockDBusFixture: public GTestDBusFixture
{
  private:

    typedef GTestDBusFixture super;

  protected:

    MockScreenSaver * screen_saver;
    MockSessionManager * session_manager;
    MockDisplayManagerSeat * dm_seat;
    MockAccounts * accounts;
    MockConsoleKitSession * ck_session;
    MockConsoleKitSeat * ck_seat;
    MockConsoleKitManager * ck_manager;
    MockUPower * upower;
    MockEndSessionDialog * end_session_dialog;
    MockWebcredentials * webcredentials;

  protected:

    virtual void SetUp ()
    {
      super :: SetUp ();

      webcredentials = new MockWebcredentials (loop, conn);
      end_session_dialog = new MockEndSessionDialog (loop, conn);
      session_manager = new MockSessionManager (loop, conn);
      screen_saver = new MockScreenSaver (loop, conn);
      upower = new MockUPower (loop, conn);
      dm_seat = new MockDisplayManagerSeat (loop, conn);
      g_setenv ("XDG_SEAT_PATH", dm_seat->path(), TRUE);
      dm_seat->set_guest_allowed (false);
      accounts = build_accounts_mock ();
      ck_manager = new MockConsoleKitManager (loop, conn);
      ck_seat = new MockConsoleKitSeat (loop, conn, true);
      MockUser * user = accounts->find_by_username ("msmith");
      ck_session = ck_seat->add_session_by_user (user);
      ck_manager->add_seat (ck_seat);
      dm_seat->set_consolekit_seat (ck_seat);
      dm_seat->switch_to_user (user->username());
      ASSERT_EQ (ck_session, ck_manager->current_session());
    }

  protected:

    virtual void TearDown ()
    {
      delete accounts;
      delete ck_manager;
      delete dm_seat;
      delete upower;
      delete screen_saver;
      delete session_manager;
      delete end_session_dialog;
      delete webcredentials;

      super :: TearDown ();
    }

  private:

    MockAccounts * build_accounts_mock ()
    {
      struct {
        guint64 login_frequency;
        const gchar * user_name;
        const gchar * real_name;
      } users[] = {
        { 134, "whartnell",  "First Doctor"    },
        { 119, "ptroughton", "Second Doctor"   },
        { 128, "jpertwee",   "Third Doctor"    },
        { 172, "tbaker",     "Fourth Doctor"   },
        {  69, "pdavison",   "Fifth Doctor"    },
        {  31, "cbaker",     "Sixth Doctor"    },
        {  42, "smccoy",     "Seventh Doctor"  },
        {   1, "pmcgann",    "Eigth Doctor"    },
        {  13, "ceccleston", "Ninth Doctor"    },
        {  47, "dtennant",   "Tenth Doctor"    },
        {  34, "msmith",     "Eleventh Doctor" },
        {   1, "rhurndall",  "First Doctor"    }
      };

      MockAccounts * a = new MockAccounts (loop, conn);
      for (int i=0, n=G_N_ELEMENTS(users); i<n; ++i)
        a->add_user (new MockUser (loop, conn,
                                   users[i].user_name,
                                   users[i].real_name,
                                   users[i].login_frequency));
      return a;
    }
};

