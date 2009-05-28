/*
 *  Copyright (c) 2009 Stephan Arts <stephan@xfce.org>
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

#include <config.h>

#include <gtk/gtk.h>
#include <gtk/gtkmarshal.h>

#include <string.h>

#include <libexif/exif-data.h>

#include "image.h"
#include "navigator.h"

static void 
rstto_navigator_init(RsttoNavigator *);
static void
rstto_navigator_class_init(RsttoNavigatorClass *);
static void
rstto_navigator_dispose(GObject *object);

static void 
rstto_navigator_iter_init(RsttoNavigatorIter *);
static void
rstto_navigator_iter_class_init(RsttoNavigatorIterClass *);
static void
rstto_navigator_iter_dispose(GObject *object);

static RsttoNavigatorIter * rstto_navigator_iter_new ();

static gint
cb_rstto_navigator_image_name_compare_func (RsttoImage *a, RsttoImage *b);

static GObjectClass *parent_class = NULL;
static GObjectClass *iter_parent_class = NULL;

enum
{
    RSTTO_NAVIGATOR_SIGNAL_NEW_IMAGE = 0,
    RSTTO_NAVIGATOR_SIGNAL_REMOVE_IMAGE,
    RSTTO_NAVIGATOR_SIGNAL_COUNT
};

enum
{
    RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED = 0,
    RSTTO_NAVIGATOR_ITER_SIGNAL_COUNT
};

struct _RsttoNavigatorIterPriv
{
    RsttoNavigator *navigator;
    RsttoImage *image;
    gint position;
};

struct _RsttoNavigatorPriv
{
    GList *images;
    gint n_images;
};

static gint rstto_navigator_signals[RSTTO_NAVIGATOR_SIGNAL_COUNT];
static gint rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_COUNT];

GType
rstto_navigator_get_type ()
{
    static GType rstto_navigator_type = 0;

    if (!rstto_navigator_type)
    {
        static const GTypeInfo rstto_navigator_info = 
        {
            sizeof (RsttoNavigatorClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) rstto_navigator_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            sizeof (RsttoNavigator),
            0,
            (GInstanceInitFunc) rstto_navigator_init,
            NULL
        };

        rstto_navigator_type = g_type_register_static (G_TYPE_OBJECT, "RsttoNavigator", &rstto_navigator_info, 0);
    }
    return rstto_navigator_type;
}

static void
rstto_navigator_init(RsttoNavigator *navigator)
{
    navigator->priv = g_new0 (RsttoNavigatorPriv, 1);
}

static void
rstto_navigator_class_init(RsttoNavigatorClass *nav_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS(nav_class);

    parent_class = g_type_class_peek_parent(nav_class);

    object_class->dispose = rstto_navigator_dispose;

    rstto_navigator_signals[RSTTO_NAVIGATOR_SIGNAL_NEW_IMAGE] = g_signal_new("new-image",
            G_TYPE_FROM_CLASS(nav_class),
            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            G_TYPE_OBJECT,
            NULL);

    rstto_navigator_signals[RSTTO_NAVIGATOR_SIGNAL_REMOVE_IMAGE] = g_signal_new("remove-image",
            G_TYPE_FROM_CLASS(nav_class),
            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__OBJECT,
            G_TYPE_NONE,
            1,
            G_TYPE_OBJECT,
            NULL);
}

static void
rstto_navigator_dispose(GObject *object)
{
    /*RsttoNavigator *navigator = RSTTO_NAVIGATOR(object);*/
}

RsttoNavigator *
rstto_navigator_new ()
{
    RsttoNavigator *navigator;

    navigator = g_object_new(RSTTO_TYPE_NAVIGATOR, NULL);

    return navigator;
}

gboolean
rstto_navigator_add_file (RsttoNavigator *navigator, GFile *file, GError **error)
{
    RsttoImage *image = rstto_image_new (file);
    if (image)
    {
        navigator->priv->images = g_list_insert_sorted (navigator->priv->images, image, (GCompareFunc)cb_rstto_navigator_image_name_compare_func);
        navigator->priv->n_images++;

        g_signal_emit (G_OBJECT (navigator), rstto_navigator_signals[RSTTO_NAVIGATOR_SIGNAL_NEW_IMAGE], 0, image, NULL);
        return TRUE;
    }
    return FALSE;
}

gint
rstto_navigator_get_n_images (RsttoNavigator *navigator)
{
    return g_list_length (navigator->priv->images);
}

RsttoNavigatorIter *
rstto_navigator_get_iter (RsttoNavigator *navigator)
{
    RsttoImage *image = NULL;
    if (navigator->priv->images)
        image = navigator->priv->images->data;

    RsttoNavigatorIter *iter = rstto_navigator_iter_new (navigator, image);

    return iter;
}


void
rstto_navigator_remove_image (RsttoNavigator *navigator, RsttoImage *image)
{
    if (g_list_find (navigator->priv->images, image))
    {
        navigator->priv->images = g_list_remove (navigator->priv->images, image);
        g_signal_emit (G_OBJECT (navigator), rstto_navigator_signals[RSTTO_NAVIGATOR_SIGNAL_REMOVE_IMAGE], 0, image, NULL);

        g_object_unref (image);
    }
}

void
rstto_navigator_remove_all (RsttoNavigator *navigator)
{
    g_list_foreach (navigator->priv->images, (GFunc)g_object_unref, NULL);
    g_list_free (navigator->priv->images);
    navigator->priv->images = NULL;
}


/**
 * cb_rstto_navigator_image_name_compare_func:
 * @a:
 * @b:
 *
 *
 * Return value: (see strcmp)
 */
static gint
cb_rstto_navigator_image_name_compare_func (RsttoImage *a, RsttoImage *b)
{
    gchar *a_base = g_file_get_basename (rstto_image_get_file (a));  
    gchar *b_base = g_file_get_basename (rstto_image_get_file (b));  
    gint result = 0;

    result = g_strcasecmp (a_base, b_base);

    g_free (a_base);
    g_free (b_base);
    return result;
}

GType
rstto_navigator_iter_get_type ()
{
    static GType rstto_navigator_iter_type = 0;

    if (!rstto_navigator_iter_type)
    {
        static const GTypeInfo rstto_navigator_iter_info = 
        {
            sizeof (RsttoNavigatorIterClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) rstto_navigator_iter_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            sizeof (RsttoNavigatorIter),
            0,
            (GInstanceInitFunc) rstto_navigator_iter_init,
            NULL
        };

        rstto_navigator_iter_type = g_type_register_static (G_TYPE_OBJECT, "RsttoNavigatorIter", &rstto_navigator_iter_info, 0);
    }
    return rstto_navigator_iter_type;
}

static void
rstto_navigator_iter_init (RsttoNavigatorIter *iter)
{
    iter->priv = g_new0 (RsttoNavigatorIterPriv, 1);
}

static void
rstto_navigator_iter_class_init(RsttoNavigatorIterClass *iter_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS(iter_class);

    iter_parent_class = g_type_class_peek_parent(iter_class);

    object_class->dispose = rstto_navigator_iter_dispose;

    rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED] = g_signal_new("changed",
            G_TYPE_FROM_CLASS(iter_class),
            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0,
            NULL);

}

static void
rstto_navigator_iter_dispose (GObject *object)
{
    RsttoNavigatorIter *iter = RSTTO_NAVIGATOR_ITER(object);
    if (iter->priv->image)
    {
        g_object_unref (iter->priv->image);
        iter->priv->image = NULL;
    }
}

static RsttoNavigatorIter *
rstto_navigator_iter_new (RsttoNavigator *nav, RsttoImage *image)
{
    RsttoNavigatorIter *iter;

    iter = g_object_new(RSTTO_TYPE_NAVIGATOR_ITER, NULL);
    iter->priv->image = image;
    iter->priv->navigator = nav;
    iter->priv->position = -1;

    return iter;
}

gboolean
rstto_navigator_iter_find_image (RsttoNavigatorIter *iter, RsttoImage *image)
{
    gint pos = g_list_index (iter->priv->navigator->priv->images, image);
    if (pos > -1)
    {
        if (iter->priv->image)
        {
            g_object_unref (iter->priv->image);
            iter->priv->image = NULL;
        }
        iter->priv->image = image;
        g_object_ref (iter->priv->image);

        g_signal_emit (G_OBJECT (iter), rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED], 0, NULL);

        return TRUE;
    }
    return FALSE;
}

gint
rstto_navigator_iter_get_position (RsttoNavigatorIter *iter)
{
    if (iter->priv->image == NULL)
    {
        if ((iter->priv->position == -1) && (rstto_navigator_get_n_images (iter->priv->navigator) > 0))
        {
            rstto_navigator_iter_set_position (iter, 0);
        }
    }
    return iter->priv->position;
}

RsttoImage *
rstto_navigator_iter_get_image (RsttoNavigatorIter *iter)
{
    if (iter->priv->image == NULL)
    {
        if ((iter->priv->position == -1) && (rstto_navigator_get_n_images (iter->priv->navigator) > 0))
        {
            rstto_navigator_iter_set_position (iter, 0);
        }
    }
    return RSTTO_IMAGE (iter->priv->image);
}


gboolean
rstto_navigator_iter_set_position (RsttoNavigatorIter *iter, gint pos)
{
    if (iter->priv->image)
    {
        g_object_unref (iter->priv->image);
        iter->priv->image = NULL;
    }

    iter->priv->image = g_list_nth_data (iter->priv->navigator->priv->images, pos); 
    if (iter->priv->image)
    {
        iter->priv->position = pos;
        g_object_ref (iter->priv->image);
    }
    else
    {
        iter->priv->position = -1;
    }
    g_signal_emit (G_OBJECT (iter), rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED], 0, NULL);
}

gboolean
rstto_navigator_iter_next (RsttoNavigatorIter *iter)
{
    if (iter->priv->image)
    {
        g_object_unref (iter->priv->image);
        iter->priv->image = NULL;
    }

    iter->priv->image = g_list_nth_data (iter->priv->navigator->priv->images, iter->priv->position+1); 
    if (iter->priv->image)
    {
        iter->priv->position++;
    }
    else
    {
        iter->priv->position = 0;
        iter->priv->image = g_list_nth_data (iter->priv->navigator->priv->images, 0); 
        if (iter->priv->image == NULL)
        {
            iter->priv->position = -1;
        }
    }
    if (iter->priv->image)
        g_object_ref (iter->priv->image);

    g_signal_emit (G_OBJECT (iter), rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED], 0, NULL);
}

gboolean
rstto_navigator_iter_previous (RsttoNavigatorIter *iter)
{
    if (iter->priv->image)
    {
        g_object_unref (iter->priv->image);
        iter->priv->image = NULL;
    }

    iter->priv->image = g_list_nth_data (iter->priv->navigator->priv->images, iter->priv->position-1); 
    if (iter->priv->image)
    {
        iter->priv->position--;
    }
    else
    {
        iter->priv->position = g_list_length (iter->priv->navigator->priv->images)-1;
        iter->priv->image = g_list_nth_data (iter->priv->navigator->priv->images, iter->priv->position); 
        if (iter->priv->image == NULL)
        {
            iter->priv->position = -1;
        }
    }
    if (iter->priv->image)
        g_object_ref (iter->priv->image);
    g_signal_emit (G_OBJECT (iter), rstto_navigator_iter_signals[RSTTO_NAVIGATOR_ITER_SIGNAL_CHANGED], 0, NULL);
}

RsttoNavigatorIter *
rstto_navigator_iter_clone (RsttoNavigatorIter *iter)
{
    RsttoNavigatorIter *new_iter = rstto_navigator_iter_new (iter->priv->navigator, iter->priv->image);
    new_iter->priv->position = iter->priv->position;

    return new_iter;
}
