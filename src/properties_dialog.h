/*
 *  Copyright (c) Stephan Arts 2009-2010 <stephan@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __RISTRETTO_PROPERTIES_DIALOG_H__
#define __RISTRETTO_PROPERTIES_DIALOG_H__

G_BEGIN_DECLS

#define RSTTO_TYPE_PROPERTIES_DIALOG rstto_properties_dialog_get_type()

#define RSTTO_PROPERTIES_DIALOG(obj)( \
        G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                RSTTO_TYPE_PROPERTIES_DIALOG, \
                RsttoPropertiesDialog))

#define RSTTO_IS_PROPERTIES_DIALOG(obj)( \
        G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                RSTTO_TYPE_PROPERTIES_DIALOG))

#define RSTTO_PROPERTIES_DIALOG_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_CAST ((klass), \
                RSTTO_TYPE_PROPERTIES_DIALOG, \
                RsttoPropertiesDialogClass))

#define RSTTO_IS_PROPERTIES_DIALOG_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_TYPE ((klass), \
                RSTTO_TYPE_PROPERTIES_DIALOG()))

typedef struct _RsttoPropertiesDialog RsttoPropertiesDialog;

typedef struct _RsttoPropertiesDialogPriv RsttoPropertiesDialogPriv;

struct _RsttoPropertiesDialog
{
    GtkDialog parent;
    RsttoPropertiesDialogPriv *priv;
};

typedef struct _RsttoPropertiesDialogClass RsttoPropertiesDialogClass;

struct _RsttoPropertiesDialogClass
{
    GtkDialogClass  parent_class;
};

GType
rstto_properties_dialog_get_type();

GtkWidget *
rstto_properties_dialog_new (
        GtkWindow *parent,
        GFile *file
        );

G_END_DECLS

#endif /* __RISTRETTO_PROPERTIES_DIALOG_H__ */