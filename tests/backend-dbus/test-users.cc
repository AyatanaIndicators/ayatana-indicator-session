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

#include "gtest-mock-dbus-fixture.h"

#include "backend.h"
#include "backend-dbus/backend-dbus.h"

/***
****
***/

class Users: public GTestMockDBusFixture
{
  private:

    typedef GTestMockDBusFixture super;

  protected:

    GCancellable * cancellable;
    IndicatorSessionUsers * users;

    virtual void SetUp ()
    {
      super :: SetUp ();

      init_event_keys (0);

      // init 'users'
      cancellable = g_cancellable_new ();
      users = 0;
      backend_get (cancellable, NULL, &users, NULL);
      g_assert (users != 0);
      wait_msec (100);
    }

    virtual void TearDown ()
    {
      g_cancellable_cancel (cancellable);
      g_clear_object (&cancellable);
      g_clear_object (&users);

      super :: TearDown ();
    }

  protected:

    void compare_user (const IndicatorSessionUser * isu,
                       MockUser                   * mu,
                       bool                         is_logged_in,
                       bool                         is_current_user)
    {
      ASSERT_TRUE (isu != 0);
      ASSERT_TRUE (mu != 0);

      ASSERT_EQ (mu->uid(), isu->uid);
      ASSERT_EQ (mu->login_frequency(), isu->login_frequency);
      ASSERT_STREQ (mu->username(), isu->user_name);
      ASSERT_STREQ (mu->realname(), isu->real_name);
      ASSERT_EQ (is_logged_in, isu->is_logged_in);
      ASSERT_EQ (is_current_user, isu->is_current_user);
      // FIXME: test icon file?
    }

    void compare_user (const std::string & key,
                       MockUser          * mu,
                       bool                is_logged_in,
                       bool                is_current_user)
    {
      IndicatorSessionUser * isu;
      isu = indicator_session_users_get_user (users, key.c_str());
      compare_user (isu, mu, is_logged_in, is_current_user);
      indicator_session_user_free (isu);
    }

  private:

    void init_event_keys (size_t n)
    {
      expected_event_count = n;
      event_keys.clear();
    }

    static gboolean
    wait_for_signals__timeout (gpointer name)
    {
      g_error ("%s: timed out waiting for signal '%s'", G_STRLOC, (char*)name);
      return G_SOURCE_REMOVE;
    }

    static void
    wait_for_signals__event (IndicatorSessionUser * u G_GNUC_UNUSED,
                            const char            * key,
                            gpointer                gself)
    {
      Users * self = static_cast<Users*>(gself);

      self->event_keys.push_back (key);

      if (self->event_keys.size() == self->expected_event_count)
        g_main_loop_quit (self->loop);
    }

  protected:

    std::vector<std::string> event_keys;
    size_t expected_event_count;

    void wait_for_signals (gpointer o, const gchar * name, size_t n)
    {
      const int timeout_seconds = 5; // arbitrary

      init_event_keys (n);

      guint handler_id = g_signal_connect (o, name,
                                           G_CALLBACK(wait_for_signals__event),
                                           this);
      gulong timeout_id = g_timeout_add_seconds (timeout_seconds,
                                                 wait_for_signals__timeout,
                                                 (gpointer)name);
      g_main_loop_run (loop);
      g_source_remove (timeout_id);
      g_signal_handler_disconnect (o, handler_id);
    }
};

/***
****
***/

/**
 * Confirm that the fixture's SetUp() and TearDown() work
 */
TEST_F (Users, HelloWorld)
{
  ASSERT_TRUE (true);
}


/**
 * Confirm that 'users' can get the cached users from our Mock Accounts
 */
TEST_F (Users, InitialUsers)
{
  const guint logged_in_uid = ck_session->user()->uid();
  GStrv keys = indicator_session_users_get_keys (users);

  ASSERT_EQ (12, g_strv_length (keys));

  for (int i=0; keys && keys[i]; ++i)
    {
      MockUser * mu = accounts->find_by_path (keys[i]);
      const bool is_logged_in = mu->uid() == logged_in_uid;
      const bool is_current_user = mu->uid() == logged_in_uid;
      compare_user (keys[i], mu, is_logged_in, is_current_user);
    }

  g_strfreev (keys);
}

/**
 * Confirm that 'users' can tell when a new user is added
 */
TEST_F (Users, UserAdded)
{
  MockUser * mu;

  mu = new MockUser (loop, conn, "pcushing", "Peter Cushing", 2);
  accounts->add_user (mu);
  ASSERT_EQ (0, event_keys.size());
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_ADDED, 1);
  ASSERT_EQ (1, event_keys.size());
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());

  compare_user (event_keys[0], mu, false, false);
}

/**
 * Confirm that 'users' can tell when a user is removed
 */
TEST_F (Users, UserRemoved)
{
  MockUser * mu;

  mu = accounts->find_by_username ("pdavison");
  accounts->remove_user (mu);
  ASSERT_EQ (0, event_keys.size());
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_REMOVED, 1);
  ASSERT_EQ (1, event_keys.size());
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());

  GStrv keys = indicator_session_users_get_keys (users);
  ASSERT_EQ (11, g_strv_length (keys));
  g_strfreev (keys);

  ASSERT_TRUE (indicator_session_users_get_user (users, mu->path()) == NULL);

  delete mu;
}

/**
 * Confirm that 'users' notices when a user's real name changes
 */
TEST_F (Users, RealnameChanged)
{
  MockUser * mu;

  mu = accounts->find_by_username ("pdavison");
  const char * const realname = "Peter M. G. Moffett";
  mu->set_realname (realname);
  ASSERT_NE (mu->realname(), realname);
  ASSERT_STREQ (mu->realname(), realname);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 1);
  ASSERT_EQ (1, event_keys.size());
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());
  compare_user (mu->path(), mu, false, false);
}

/**
 * Confirm that 'users' notices when users log in and out
 */
TEST_F (Users, LogInLogOut)
{
  // The fist doctor logs in.
  // Confirm that 'users' notices.
  MockUser * mu = accounts->find_by_username ("whartnell");
  compare_user (mu->path(), mu, false, false);
  MockConsoleKitSession * session = ck_seat->add_session_by_user (mu);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 1);
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());
  compare_user (mu->path(), mu, true, false);

  // The fist doctor logs out.
  // Confirm that 'users' notices.
  ck_seat->remove_session (session);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 1);
  ASSERT_EQ (1, event_keys.size());
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());
  compare_user (event_keys[0], mu, false, false);

  delete session;
}

/**
 * Confirm that 'users' notices when the active session changes
 */
TEST_F (Users, ActivateSession)
{
  // The fist doctor logs in.
  // Confirm that 'users' notices.
  MockUser * mu = accounts->find_by_username ("whartnell");
  compare_user (mu->path(), mu, false, false);
  MockConsoleKitSession * session = ck_seat->add_session_by_user (mu);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 1);
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());
  compare_user (mu->path(), mu, true, false);

  // activate the first doctor's session.
  // confirm that 'users' sees he's active and that ck_session isn't.
  // this should come in the form of two 'user-changed' events
  ck_seat->activate_session (session);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 2);
  ASSERT_EQ (2, event_keys.size());
  compare_user (event_keys[0], ck_session->user(), true, false);
  compare_user (event_keys[1], mu, true, true);

  // switch back to the previous
  // confirm that 'users' sees it's active and the first doctor's session isn't
  // this should come in the form of two 'user-changed' events
  ck_seat->activate_session (ck_session);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 2);
  ASSERT_EQ (2, event_keys.size());
  compare_user (event_keys[0], mu, true, false);
  compare_user (event_keys[1], ck_session->user(), true, true);
}

/**
 * Confirm that we can change the active session via users' API.
 * This is nearly the same as ActivateSession but uses users' API
 */
TEST_F (Users, ActivateUser)
{
  // The fist doctor logs in.
  // Confirm that 'users' notices.
  MockUser * mu = accounts->find_by_username ("whartnell");
  compare_user (mu->path(), mu, false, false);
  MockConsoleKitSession * session = ck_seat->add_session_by_user (mu);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 1);
  ASSERT_STREQ (mu->path(), event_keys[0].c_str());
  compare_user (mu->path(), mu, true, false);

  // activate the first doctor's session.
  // confirm that 'users' sees he's active and that ck_session isn't.
  // this should come in the form of two 'user-changed' events
  indicator_session_users_activate_user (users, mu->path());
  ck_seat->activate_session (session);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 2);
  ASSERT_EQ (2, event_keys.size());
  compare_user (event_keys[0], ck_session->user(), true, false);
  compare_user (event_keys[1], mu, true, true);

  // switch back to the previous
  // confirm that 'users' sees it's active and the first doctor's session isn't
  // this should come in the form of two 'user-changed' events
  indicator_session_users_activate_user (users, ck_session->user()->path());
  ck_seat->activate_session (ck_session);
  wait_for_signals (users, INDICATOR_SESSION_USERS_SIGNAL_USER_CHANGED, 2);
  ASSERT_EQ (2, event_keys.size());
  compare_user (event_keys[0], mu, true, false);
  compare_user (event_keys[1], ck_session->user(), true, true);
}

/**
 * Confirm that adding a Guest doesn't show up in the users list
 */
TEST_F (Users, UnwantedGuest)
{
  GStrv keys;

  keys = indicator_session_users_get_keys (users);
  const size_t n = g_strv_length (keys);
  g_strfreev (keys);

  MockUser * mu = new MockUser (loop, conn, "guest-jjbEVV", "Guest", 1);
  mu->set_system_account (true);
  accounts->add_user (mu);
  wait_msec (50);

  keys = indicator_session_users_get_keys (users);
  ASSERT_EQ (n, g_strv_length (keys));
  g_strfreev (keys);
}


/**
 * Confirm that we can detect live sessions
 */
TEST_F (Users, LiveSession)
{
  gboolean b;

  // not initially a live session
  ASSERT_FALSE (indicator_session_users_is_live_session (users));
  g_object_get (users, INDICATOR_SESSION_USERS_PROP_IS_LIVE_SESSION, &b, NULL);
  ASSERT_FALSE (b);

  // now add the criteria for a live session
  MockUser * live_user = new MockUser (loop, conn, "ubuntu", "Ubuntu", 1, 999);
  live_user->set_system_account (true);
  accounts->add_user (live_user);
  MockConsoleKitSession * session = ck_seat->add_session_by_user (live_user);
  wait_msec (100);
  ck_seat->activate_session (session);
  wait_for_signal (users, "notify::"INDICATOR_SESSION_USERS_PROP_IS_LIVE_SESSION);

  // confirm the backend thinks it's a live session
  ASSERT_TRUE (indicator_session_users_is_live_session (users));
  g_object_get (users, INDICATOR_SESSION_USERS_PROP_IS_LIVE_SESSION, &b, NULL);
  ASSERT_TRUE (b);
}
