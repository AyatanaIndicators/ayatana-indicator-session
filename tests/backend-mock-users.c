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

#include "backend-mock.h"
#include "backend-mock-users.h"

struct _IndicatorSessionUsersMockPriv
{
  GHashTable * users;
};

typedef IndicatorSessionUsersMockPriv priv_t;

G_DEFINE_TYPE (IndicatorSessionUsersMock,
               indicator_session_users_mock,
               INDICATOR_TYPE_SESSION_USERS)

/***
****
***/

static void
my_dispose (GObject * o)
{
  G_OBJECT_CLASS (indicator_session_users_mock_parent_class)->dispose (o);
}

static void
my_finalize (GObject * o)
{
  priv_t * p = INDICATOR_SESSION_USERS_MOCK (o)->priv;

  g_hash_table_destroy (p->users);

  G_OBJECT_CLASS (indicator_session_users_mock_parent_class)->finalize (o);
}

static gboolean
my_is_live_session (IndicatorSessionUsers * users G_GNUC_UNUSED)
{
  return g_settings_get_boolean (mock_settings, "is-live-session");
}

static void
my_activate_user (IndicatorSessionUsers * users, const char * key)
{
  g_message ("%s %s users %p key %s FIXME", G_STRLOC, G_STRFUNC, (void*)users, key);
}

static GStrv
my_get_keys (IndicatorSessionUsers * users)
{
  int i;
  priv_t * p;
  gchar ** keys;
  GHashTableIter iter;
  gpointer key;

  g_return_val_if_fail (INDICATOR_IS_SESSION_USERS_MOCK(users), NULL);
  p = INDICATOR_SESSION_USERS_MOCK (users)->priv;

  i = 0;
  keys = g_new (gchar*, g_hash_table_size(p->users)+1);
  g_hash_table_iter_init (&iter, p->users);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    keys[i++] = g_strdup (key);
  keys[i] = NULL;

  return keys;
}

static IndicatorSessionUser *
my_get_user (IndicatorSessionUsers * self, const gchar * key)
{
  priv_t * p;
  const IndicatorSessionUser * src;
  IndicatorSessionUser * ret = NULL;

  g_return_val_if_fail (INDICATOR_IS_SESSION_USERS_MOCK(self), NULL);
  p = INDICATOR_SESSION_USERS_MOCK (self)->priv;

  if ((src = g_hash_table_lookup (p->users, key)))
    {
      ret = g_new0 (IndicatorSessionUser, 1);
      ret->is_current_user = src->is_current_user;
      ret->is_logged_in = src->is_logged_in;
      ret->uid = src->uid;
      ret->login_frequency = src->login_frequency;
      ret->user_name = g_strdup (src->user_name);
      ret->real_name = g_strdup (src->real_name);
      ret->icon_file = g_strdup (src->icon_file);
    }

  return ret;
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_users_mock_class_init (IndicatorSessionUsersMockClass * klass)
{
  GObjectClass * object_class;
  IndicatorSessionUsersClass * users_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = my_dispose;
  object_class->finalize = my_finalize;

  users_class = INDICATOR_SESSION_USERS_CLASS (klass);
  users_class->is_live_session = my_is_live_session;
  users_class->get_keys = my_get_keys;
  users_class->get_user = my_get_user;
  users_class->activate_user = my_activate_user;

  g_type_class_add_private (klass, sizeof (IndicatorSessionUsersMockPriv));
}

static void
/* cppcheck-suppress unusedFunction */
indicator_session_users_mock_init (IndicatorSessionUsersMock * self)
{
  priv_t * p;

  p = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                   INDICATOR_TYPE_SESSION_USERS_MOCK,
                                   IndicatorSessionUsersMockPriv);
  self->priv = p;

  p->users = g_hash_table_new_full (g_str_hash,
                                    g_str_equal,
                                    g_free,
                                    (GDestroyNotify)indicator_session_user_free);

  g_signal_connect_swapped (mock_settings, "changed::is-live-session",
                            G_CALLBACK(indicator_session_users_notify_is_live_session), self);
}

/***
****  Public
***/

IndicatorSessionUsers *
indicator_session_users_mock_new (void)
{
  gpointer o = g_object_new (INDICATOR_TYPE_SESSION_USERS_MOCK, NULL);

  return INDICATOR_SESSION_USERS (o);
}


void
indicator_session_users_mock_add_user (IndicatorSessionUsersMock * self,
                                       const char                * key,
                                       IndicatorSessionUser      * user)
{
  g_return_if_fail (INDICATOR_IS_SESSION_USERS_MOCK (self));
  g_return_if_fail (key && *key);
  g_return_if_fail (user != NULL);

  g_hash_table_insert (self->priv->users, g_strdup(key), user);
  indicator_session_users_added (INDICATOR_SESSION_USERS (self), key);
}

void
indicator_session_users_mock_remove_user (IndicatorSessionUsersMock * self,
                                          const char                * key)
{
  g_return_if_fail (INDICATOR_IS_SESSION_USERS_MOCK (self));
  g_return_if_fail (key && *key);

  g_hash_table_remove (self->priv->users, key);
  indicator_session_users_removed (INDICATOR_SESSION_USERS (self), key);
}

