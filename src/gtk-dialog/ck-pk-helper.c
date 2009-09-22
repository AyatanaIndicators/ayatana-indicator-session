/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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


#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <polkit/polkit.h>

#include "logout-dialog.h"
#include "ck-pk-helper.h"

static gboolean
ck_multiple_users (void)
{
	DBusGConnection * sbus = dbus_g_bus_get(DBUS_BUS_SYSTEM, NULL);
	g_return_val_if_fail(sbus != NULL, TRUE); /* worst case */
	DBusGProxy * proxy = dbus_g_proxy_new_for_name(sbus, "org.freedesktop.ConsoleKit",
	                                                     "/org/freedesktop/ConsoleKit/Manager",
	                                                     "org.freedesktop.ConsoleKit.Manager");

	if (proxy == NULL) {
		return TRUE;
	}

	gboolean result;
	GPtrArray * seats = NULL;

	result = dbus_g_proxy_call(proxy, "GetSeats", NULL, G_TYPE_INVALID,
	                           dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &seats, G_TYPE_INVALID);

	if (!result) {
		g_warning("Unable to get the seats for ConsoleKit");
		g_object_unref(proxy);
		return TRUE;
	}
	
	gchar * this_session_id = NULL;
	
	result = dbus_g_proxy_call(proxy, "GetCurrentSession", NULL, G_TYPE_INVALID,
							DBUS_TYPE_G_OBJECT_PATH, &this_session_id, G_TYPE_INVALID);
							
	g_object_unref(proxy);
							
	if (!result) {
		g_warning("Unable to get current session from ConsoleKit");
		return TRUE;
	}
	
	proxy = dbus_g_proxy_new_for_name(sbus, "org.freedesktop.ConsoleKit",
									this_session_id, "org.freedesktop.ConsoleKit.Session");

	if (proxy == NULL) {
		return TRUE;
	}
	
	guint this_session_uid;
	
	result = dbus_g_proxy_call(proxy, "GetUnixUser", NULL, G_TYPE_INVALID,
							G_TYPE_UINT, &this_session_uid, G_TYPE_INVALID);										

	if (!result) {
		g_warning("Unable to get UID from ConsoleKit");
		return TRUE;
	}
	
	guint seat;
	gboolean multiple_users = FALSE;
	for (seat = 0; seat < seats->len; seat++) {
		gchar * seat_id = g_ptr_array_index(seats, seat);
		DBusGProxy * seat_proxy = dbus_g_proxy_new_for_name(sbus, "org.freedesktop.ConsoleKit",
		                                                    seat_id, "org.freedesktop.ConsoleKit.Seat");
		g_free(seat_id);

		if (seat_proxy == NULL) {
			continue;
		}

		GPtrArray * sessions = NULL;

		gboolean result = dbus_g_proxy_call(seat_proxy,
		                                    "GetSessions", NULL, G_TYPE_INVALID,
	                                        dbus_g_type_get_collection("GPtrArray", DBUS_TYPE_G_OBJECT_PATH), &sessions, G_TYPE_INVALID);

		g_object_unref(seat_proxy);
		if (!result) {
			continue;
		}

		guint session;
		for (session = 0; session < sessions->len; session++) {
			gchar * session_id = g_ptr_array_index(sessions, session);
			if (g_strcmp0(this_session_id, session_id) == 0) {
				continue;
			}
			DBusGProxy * session_proxy = dbus_g_proxy_new_for_name(sbus, "org.freedesktop.ConsoleKit",
		                                                    	session_id, "org.freedesktop.ConsoleKit.Session");
		    g_free(session_id);
		    
		    if (session_proxy == NULL) {
		    	continue;
		    }
		    
		    guint session_uid;
		    result =  dbus_g_proxy_call(session_proxy, "GetUnixUser", NULL, G_TYPE_INVALID,
		    							G_TYPE_UINT, &session_uid, G_TYPE_INVALID);	
		    g_object_unref(session_proxy);
		    							
		    if (!result) {
		    	continue;
		    }
		    
		    if (session_uid != this_session_uid) {
		    	multiple_users = TRUE;
		    	break;
		    }
		}
		
		g_ptr_array_free(sessions, TRUE);
		
		if (multiple_users) {
			break;
		}
	}

	g_ptr_array_free(seats, TRUE);
	g_object_unref(proxy);
	g_free(this_session_id);

	return multiple_users;
}

gboolean
pk_require_auth (LogoutDialogAction action) {
	if (action == LOGOUT_DIALOG_LOGOUT) {
		return FALSE;
	}

	gchar * pk_action;
	if (ck_multiple_users()) {
		if (action == LOGOUT_DIALOG_RESTART) {
			pk_action = "org.freedesktop.consolekit.system.restart-multiple-users";
		} else {
			pk_action = "org.freedesktop.consolekit.system.stop-multiple-users";
		}
	} else {
		if (action == LOGOUT_DIALOG_RESTART) {
			pk_action = "org.freedesktop.consolekit.system.restart";
		} else {
			pk_action = "org.freedesktop.consolekit.system.stop";
		}
	}

	PolkitAuthorizationResult *polres = NULL;
	gboolean ret = FALSE;
	if (pk_can_do_action(pk_action, &polres)) {
		if (polkit_authorization_result_get_is_challenge (polres)) {
			ret = TRUE;
		}
		g_debug ("pk_require_auth(%s): authorized, is_challenge: %i", pk_action, ret);
	} else {
		g_debug ("pk_require_auth(%s): not authorized", pk_action);
	}
	if (polres) {
		g_object_unref (polres);
	}
	return ret;
}

gboolean
pk_can_do_action (const gchar    *action_id, PolkitAuthorizationResult ** pol_result)
{
	PolkitAuthority *authority;
	PolkitSubject *subject;
	PolkitAuthorizationResult *result;
	gboolean ret;

	authority = polkit_authority_get();
	if (!authority) {
		g_warning ("Could not get PolicyKit authority instance");
		return FALSE;
	}
	subject = polkit_unix_process_new (getpid());

	result = polkit_authority_check_authorization_sync (authority, subject, action_id, NULL, 0, NULL, NULL);
	g_object_unref (authority);

	ret = FALSE;
	if (result) {
		ret = polkit_authorization_result_get_is_authorized (result) || 
		      polkit_authorization_result_get_is_challenge (result);
		g_debug ("pk_can_do_action(%s): %i", action_id, ret);
	} else {
		g_warning ("pk_can_do_action(%s): check_authorization returned NULL", action_id);
	}
	if (pol_result) {
		*pol_result = result;
	} else {
		g_object_unref (result);
	}
	return ret;
	
}
