/*
Copyright 2011 Canonical Ltd.

Authors:
    Conor Curran <conor.curran@canonical.com>

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

#ifndef _APT_TRANSACTION_H_
#define _APT_TRANSACTION_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define APT_TYPE_TRANSACTION             (apt_transaction_get_type ())
#define APT_TRANSACTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), APT_TYPE_TRANSACTION, AptTransaction))
#define APT_TRANSACTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), APT_TYPE_TRANSACTION, AptTransactionClass))
#define APT_IS_TRANSACTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APT_TYPE_TRANSACTION))
#define APT_IS_TRANSACTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), APT_TYPE_TRANSACTION))
#define APT_TRANSACTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), APT_TYPE_TRANSACTION, AptTransactionClass))

typedef struct _AptTransactionClass AptTransactionClass;
typedef struct _AptTransaction AptTransaction;

struct _AptTransactionClass
{
	GObjectClass parent_class;
};

AptTransaction* apt_transaction_new (gchar* transaction_id);

GType apt_transaction_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _APT_TRANSACTION_H_ */
