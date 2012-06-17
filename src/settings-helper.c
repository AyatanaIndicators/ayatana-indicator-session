/*
A small wrapper utility for connecting to GSettings.

Copyright 2009 Canonical Ltd.

Authors:
    Christoph Korn <c_korn@gmx.de>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gio/gio.h>

#include "dbus-shared-names.h"
#include "settings-helper.h"

static GSettings* settings = NULL;

static gboolean
build_settings (void)
{
  if (G_UNLIKELY(settings == NULL))
    {
      settings = g_settings_new (SESSION_SCHEMA);
    }

  return settings != NULL;
}

gboolean
supress_confirmations (void)
{
  g_return_val_if_fail (build_settings(), FALSE);

  return g_settings_get_boolean (settings, SUPPRESS_KEY) ;
}
