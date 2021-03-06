/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 8 -*- */
/* gdl-switcher.c
 *
 * Copyright (C) 2003  Ettore Perazzoli,
 *               2007  Naba Kumar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Copied and adapted from ESidebar.[ch] from evolution
 * 
 * Authors: Ettore Perazzoli <ettore@ximian.com>
 *          Naba Kumar  <naba@gnome.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include "gdl-switcher.h"
#include "gdl-tools.h"
#include "libgdlmarshal.h"
#include "libgdltypebuiltins.h"

#include <gtk/gtk.h>

#if HAVE_GNOME
#include <gconf/gconf-client.h>
#endif

static void gdl_switcher_set_property  (GObject            *object,
                                        guint               prop_id,
                                        const GValue       *value,
                                        GParamSpec         *pspec);
static void gdl_switcher_get_property  (GObject            *object,
                                        guint               prop_id,
                                        GValue             *value,
                                        GParamSpec         *pspec);

static void gdl_switcher_add_button  (GdlSwitcher *switcher,
                                      const gchar *label,
                                      const gchar *tooltips,
                                      const gchar *stock_id,
                                      const GdkPixbuf *pixbuf_icon,
                                      gint switcher_id);
static void gdl_switcher_remove_button (GdlSwitcher *switcher, gint switcher_id);
static void gdl_switcher_select_page (GdlSwitcher *switcher, gint switcher_id);
static void gdl_switcher_select_button (GdlSwitcher *switcher, gint switcher_id);
static void gdl_switcher_set_show_buttons (GdlSwitcher *switcher, gboolean show);
static void gdl_switcher_set_style (GdlSwitcher *switcher,
                                    GdlSwitcherStyle switcher_style);
static GdlSwitcherStyle gdl_switcher_get_style (GdlSwitcher *switcher);

enum {
    PROP_0,
    PROP_SWITCHER_STYLE
};

typedef struct {
    GtkWidget *button_widget;
    GtkWidget *label;
    GtkWidget *icon;
    GtkWidget *arrow;
    GtkWidget *hbox;
    GtkTooltips *tooltips;
    int id;
} Button;

struct _GdlSwitcherPrivate {
    GdlSwitcherStyle switcher_style;
    GdlSwitcherStyle toolbar_style;
    
    gboolean show;
    GSList *buttons;

    guint style_changed_id;
    gint buttons_height_request;
    gboolean in_toggle;
};

GDL_CLASS_BOILERPLATE (GdlSwitcher, gdl_switcher, GtkNotebook, GTK_TYPE_NOTEBOOK)

#define INTERNAL_MODE(switcher)  (switcher->priv->switcher_style == \
            GDL_SWITCHER_STYLE_TOOLBAR ? switcher->priv->toolbar_style : \
            switcher->priv->switcher_style)

#define H_PADDING 2
#define V_PADDING 2

/* Utility functions.  */

static Button *
button_new (GtkWidget *button_widget, GtkWidget *label, GtkWidget *icon,
            GtkTooltips *tooltips, GtkWidget *arrow, GtkWidget *hbox, int id)
{
    Button *button = g_new (Button, 1);

    button->button_widget = button_widget;
    button->label = label;
    button->icon = icon;
    button->arrow = arrow;
    button->hbox = hbox;
    button->tooltips = tooltips;
    button->id = id;

    g_object_ref (button_widget);
    g_object_ref (label);
    g_object_ref (icon);
    g_object_ref (arrow);
    g_object_ref (hbox);
    g_object_ref (tooltips);

    return button;
}

static void
button_free (Button *button)
{
    g_object_unref (button->button_widget);
    g_object_unref (button->label);
    g_object_unref (button->icon);
    g_object_unref (button->hbox);
    g_object_unref (button->tooltips);
    g_free (button);
}

static gint
gdl_switcher_get_page_id (GtkWidget *widget)
{
    static gint switcher_id_count = 0;
    gint switcher_id;
    switcher_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                      "__switcher_id"));
    if (switcher_id <= 0) {
        switcher_id = ++switcher_id_count;
        g_object_set_data (G_OBJECT (widget), "__switcher_id",
                           GINT_TO_POINTER (switcher_id));
    }
    return switcher_id;
}

static void
update_buttons (GdlSwitcher *switcher, int new_selected_id)
{
    GSList *p;

    switcher->priv->in_toggle = TRUE;

    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *button = p->data;

        if (button->id == new_selected_id) {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                          (button->button_widget), TRUE);
            gtk_widget_set_sensitive (button->arrow, TRUE);
        } else {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                          (button->button_widget), FALSE);
            gtk_widget_set_sensitive (button->arrow, FALSE);
        }
    }

    switcher->priv->in_toggle = FALSE;
}

/* Callbacks.  */

static void
button_toggled_callback (GtkToggleButton *toggle_button,
                         GdlSwitcher *switcher)
{
    int id = 0;
    gboolean is_active = FALSE;
    GSList *p;

    if (switcher->priv->in_toggle)
        return;

    switcher->priv->in_toggle = TRUE;

    if (gtk_toggle_button_get_active (toggle_button))
        is_active = TRUE;

    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *button = p->data;

        if (button->button_widget != GTK_WIDGET (toggle_button)) {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                          (button->button_widget), FALSE);
            gtk_widget_set_sensitive (button->arrow, FALSE);
        } else {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                          (button->button_widget), TRUE);
            gtk_widget_set_sensitive (button->arrow, TRUE);
            id = button->id;
        }
    }
    
    switcher->priv->in_toggle = FALSE;

    if (is_active)
    {
        gdl_switcher_select_page (switcher, id);
    }
}

/* Returns -1 if layout didn't happen because a resize request was queued */
static int
layout_buttons (GdlSwitcher *switcher)
{
    GtkRequisition client_requisition;
    GtkAllocation *allocation = & GTK_WIDGET (switcher)->allocation;
    GdlSwitcherStyle switcher_style;
    gboolean icons_only;
    int num_btns = g_slist_length (switcher->priv->buttons);
    int btns_per_row;
    GSList **rows, *p;
    Button *button;
    int row_number;
    int max_btn_width = 0, max_btn_height = 0;
    int optimal_layout_width = 0;
    int row_last;
    int x, y;
    int i;
    int rows_count;
    int last_buttons_height;
    
    last_buttons_height = switcher->priv->buttons_height_request;
    
    GDL_CALL_PARENT (GTK_WIDGET_CLASS, size_request,
                     (GTK_WIDGET (switcher), &client_requisition));

    y = allocation->y + allocation->height - V_PADDING - 1;

    if (num_btns == 0)
        return y;

    switcher_style = INTERNAL_MODE (switcher);
    icons_only = (switcher_style == GDL_SWITCHER_STYLE_ICON);
    
    /* Figure out the max width and height */
    optimal_layout_width = H_PADDING;
    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        GtkRequisition requisition;

        button = p->data;
        gtk_widget_size_request (GTK_WIDGET (button->button_widget),
                                 &requisition);
        optimal_layout_width += requisition.width + H_PADDING;
        max_btn_height = MAX (max_btn_height, requisition.height);
        max_btn_width = MAX (max_btn_width, requisition.width);    
    }

    /* Figure out how many rows and columns we'll use. */
    btns_per_row = allocation->width / (max_btn_width + H_PADDING);
    /* Use at least one column */
    if (btns_per_row == 0) btns_per_row = 1;
    
    /* If all the buttons could fit in the single row, have it so */
    if (allocation->width >= optimal_layout_width)
    {
        btns_per_row = num_btns;
    }
    if (!icons_only) {
        /* If using text buttons, we want to try to have a
         * completely filled-in grid, but if we can't, we want
         * the odd row to have just a single button.
         */
        while (num_btns % btns_per_row > 1)
            btns_per_row--;
    }

    rows_count = num_btns / btns_per_row;
    if (num_btns % btns_per_row != 0)
        rows_count++;
     
    /* Assign buttons to rows */
    rows = g_new0 (GSList *, rows_count);

    if (!icons_only && num_btns % btns_per_row != 0) {
        button = switcher->priv->buttons->data;
        rows [0] = g_slist_append (rows [0], button->button_widget);

        p = switcher->priv->buttons->next;
        row_number = p ? 1 : 0;
    } else {
        p = switcher->priv->buttons;
        row_number = 0;
    }

    for (; p != NULL; p = p->next) {
        button = p->data;

        if (g_slist_length (rows [row_number]) == btns_per_row)
            row_number ++;

        rows [row_number] = g_slist_append (rows [row_number],
                                            button->button_widget);
    }

    row_last = row_number;

    /* If there are more than 1 row of buttons, save the current height
     * requirement for subsequent size requests.
     */
    if (row_last > 0)
    {
        switcher->priv->buttons_height_request =
            (row_last + 1) * (max_btn_height + V_PADDING) + 1;
    } else { /* Otherwize clear it */
        if (last_buttons_height >= 0) {

            switcher->priv->buttons_height_request = -1;
        }
    }
    
    /* If it turns out that we now require smaller height for the buttons
     * than it was last time, make a resize request to ensure our
     * size requisition is properly communicated to the parent (otherwise
     * parent tend to keep assuming the older size).
     */
    if (last_buttons_height > switcher->priv->buttons_height_request)
    {
        gtk_widget_queue_resize (GTK_WIDGET (switcher));
        return -1;
    }
    
    /* Layout the buttons. */
    for (i = row_last; i >= 0; i --) {
        int len, extra_width;
        
        y -= max_btn_height;

        /* Check for possible size over flow (taking into account client
         * requisition
         */
        if (y < (allocation->y + client_requisition.height)) {
            /* We have an overflow: Insufficient allocation */
            if (last_buttons_height < switcher->priv->buttons_height_request) {
                /* Request for a new resize */
                gtk_widget_queue_resize (GTK_WIDGET (switcher));
                return -1;
            }
        }
        x = H_PADDING + allocation->x;
        len = g_slist_length (rows[i]);
        if (switcher_style == GDL_SWITCHER_STYLE_TEXT ||
            switcher_style == GDL_SWITCHER_STYLE_BOTH)
            extra_width = (allocation->width - (len * max_btn_width )
                           - (len * H_PADDING)) / len;
        else
            extra_width = 0;
        for (p = rows [i]; p != NULL; p = p->next) {
            GtkAllocation child_allocation;
            
            child_allocation.x = x;
            child_allocation.y = y;
            if (rows_count == 1 && row_number == 0)
            {
                GtkRequisition child_requisition;
                gtk_widget_size_request (GTK_WIDGET (p->data),
                                         &child_requisition);
                child_allocation.width = child_requisition.width;
            }
            else
            {
                child_allocation.width = max_btn_width + extra_width;
            }
            child_allocation.height = max_btn_height;

            gtk_widget_size_allocate (GTK_WIDGET (p->data), &child_allocation);

            x += child_allocation.width + H_PADDING;
        }

        y -= V_PADDING;
    }
    
    for (i = 0; i <= row_last; i ++)
        g_slist_free (rows [i]);
    g_free (rows);

    return y;
}

static void
do_layout (GdlSwitcher *switcher)
{
    GtkAllocation *allocation = & GTK_WIDGET (switcher)->allocation;
    GtkAllocation child_allocation;
    int y;

    if (switcher->priv->show) {
        y = layout_buttons (switcher);
        if (y < 0) /* Layout did not happen and a resize was requested */
            return;
    }
    else
        y = allocation->y + allocation->height;
    
    /* Place the parent widget.  */
    child_allocation.x = allocation->x;
    child_allocation.y = allocation->y;
    child_allocation.width = allocation->width;
    child_allocation.height = y - allocation->y;
    
    GDL_CALL_PARENT (GTK_WIDGET_CLASS, size_allocate,
                     (GTK_WIDGET (switcher), &child_allocation));
}

/* GtkContainer methods.  */

static void
gdl_switcher_forall (GtkContainer *container, gboolean include_internals,
                     GtkCallback callback, void *callback_data)
{
    GdlSwitcher *switcher =
        GDL_SWITCHER (container);
    GSList *p;
    
    GDL_CALL_PARENT (GTK_CONTAINER_CLASS, forall,
                     (GTK_CONTAINER (switcher), include_internals,
                      callback, callback_data));
    if (include_internals) {
        for (p = switcher->priv->buttons; p != NULL; p = p->next) {
            GtkWidget *widget = ((Button *) p->data)->button_widget;
            (* callback) (widget, callback_data);
        }
    }
}

static void
gdl_switcher_remove (GtkContainer *container, GtkWidget *widget)
{
    gint switcher_id;
    GdlSwitcher *switcher =
        GDL_SWITCHER (container);
    GSList *p;
    
    switcher_id = gdl_switcher_get_page_id (widget);
    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *b = (Button *) p->data;

        if (b->id == switcher_id) {
            gtk_widget_unparent (b->button_widget);
            switcher->priv->buttons =
                g_slist_remove_link (switcher->priv->buttons, p);
            button_free (b);
            gtk_widget_queue_resize (GTK_WIDGET (switcher));
            break;
        }
    }
    GDL_CALL_PARENT (GTK_CONTAINER_CLASS, remove,
                     (GTK_CONTAINER (switcher), widget));
}

/* GtkWidget methods.  */

static void
gdl_switcher_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
    GdlSwitcher *switcher = GDL_SWITCHER (widget);
    GSList *p;
    gint button_height = 0;
    
    GDL_CALL_PARENT (GTK_WIDGET_CLASS, size_request,
                     (GTK_WIDGET (switcher), requisition));

    if (!switcher->priv->show)
        return;
    
    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        gint button_width;
        Button *button = p->data;
        GtkRequisition button_requisition;

        gtk_widget_size_request (button->button_widget, &button_requisition);
        button_width = button_requisition.width + 2 * H_PADDING;
        requisition->width = MAX (requisition->width, button_width);
        button_height = MAX (button_height,
                             button_requisition.height + 2 * V_PADDING);
    }
    
    if (switcher->priv->buttons_height_request > 0) {
        requisition->height += switcher->priv->buttons_height_request;
    } else {
        requisition->height += button_height + V_PADDING;
    }
}

static void
gdl_switcher_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    widget->allocation = *allocation;
    do_layout (GDL_SWITCHER (widget));
}

static gint
gdl_switcher_expose (GtkWidget *widget, GdkEventExpose *event)
{
    GSList *p;
    GdlSwitcher *switcher = GDL_SWITCHER (widget);
    if (switcher->priv->show) {
        for (p = switcher->priv->buttons; p != NULL; p = p->next) {
            GtkWidget *button = ((Button *) p->data)->button_widget;
            gtk_container_propagate_expose (GTK_CONTAINER (widget),
                                            button, event);
        }
    }
    return GDL_CALL_PARENT_WITH_DEFAULT (GTK_WIDGET_CLASS, expose_event,
                                  (widget, event), FALSE);
}

static void
gdl_switcher_map (GtkWidget *widget)
{
    GSList *p;
    GdlSwitcher *switcher = GDL_SWITCHER (widget);
    
    if (switcher->priv->show) {
        for (p = switcher->priv->buttons; p != NULL; p = p->next) {
            GtkWidget *button = ((Button *) p->data)->button_widget;
            gtk_widget_map (button);
        }
    }
    GDL_CALL_PARENT (GTK_WIDGET_CLASS, map, (widget));
}

/* GObject methods.  */

static void
gdl_switcher_set_property  (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GdlSwitcher *switcher = GDL_SWITCHER (object);

    switch (prop_id) {
        case PROP_SWITCHER_STYLE:
            gdl_switcher_set_style (switcher, g_value_get_enum (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_switcher_get_property  (GObject      *object,
                            guint         prop_id,
                            GValue       *value,
                            GParamSpec   *pspec)
{
    GdlSwitcher *switcher = GDL_SWITCHER (object);

    switch (prop_id) {
        case PROP_SWITCHER_STYLE:
            g_value_set_enum (value, gdl_switcher_get_style (switcher));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gdl_switcher_dispose (GObject *object)
{
    GdlSwitcherPrivate *priv = GDL_SWITCHER (object)->priv;
    
#if HAVE_GNOME
    GConfClient *gconf_client = gconf_client_get_default ();
    
    if (priv->style_changed_id) {
        gconf_client_notify_remove (gconf_client, priv->style_changed_id);
        priv->style_changed_id = 0;
    }
    g_object_unref (gconf_client);
#endif
    
    g_slist_foreach (priv->buttons, (GFunc) button_free, NULL);
    g_slist_free (priv->buttons);
    priv->buttons = NULL;

    GDL_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gdl_switcher_finalize (GObject *object)
{
    GdlSwitcherPrivate *priv = GDL_SWITCHER (object)->priv;

    g_free (priv);

    GDL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/* Signal handlers */

static void 
gdl_switcher_notify_cb (GObject *g_object, GParamSpec *pspec,
                        GdlSwitcher *switcher) 
{
    gboolean show_tabs;
    g_return_if_fail (switcher != NULL && GDL_IS_SWITCHER (switcher));
    show_tabs = gtk_notebook_get_show_tabs (GTK_NOTEBOOK (switcher));
    gdl_switcher_set_show_buttons (switcher, !show_tabs);
}

static void
gdl_switcher_switch_page_cb (GtkNotebook *nb, GtkNotebookPage *page,
                             gint page_num, GdlSwitcher *switcher)
{
    GtkWidget       *page_widget;
    GtkWidget       *tablabel;
    gint             switcher_id;
    
    /* Change switcher button */
    page_widget = gtk_notebook_get_nth_page (nb, page_num);
    switcher_id = gdl_switcher_get_page_id (page_widget);
    gdl_switcher_select_button (GDL_SWITCHER (switcher), switcher_id);
}

static void
gdl_switcher_page_added_cb (GtkNotebook *nb, GtkWidget *page,
                            gint page_num, GdlSwitcher *switcher)
{
    gint         switcher_id;
 
    switcher_id = gdl_switcher_get_page_id (page);
    
    gdl_switcher_add_button (GDL_SWITCHER (switcher), NULL, NULL, NULL, NULL,
                             switcher_id);
    gdl_switcher_select_button (GDL_SWITCHER (switcher), switcher_id);
}

static void
gdl_switcher_select_page (GdlSwitcher *switcher, gint id)
{
    GList *children, *node;
    children = gtk_container_get_children (GTK_CONTAINER (switcher));
    node = children;
    while (node)
    {
        gint switcher_id;
        switcher_id = gdl_switcher_get_page_id (GTK_WIDGET (node->data));
        if (switcher_id == id)
        {
            gint page_num;
            page_num = gtk_notebook_page_num (GTK_NOTEBOOK (switcher),
                                              GTK_WIDGET (node->data));
            g_signal_handlers_block_by_func (switcher,
                                             gdl_switcher_switch_page_cb,
                                             switcher);
            gtk_notebook_set_current_page (GTK_NOTEBOOK (switcher), page_num);
            g_signal_handlers_unblock_by_func (switcher,
                                               gdl_switcher_switch_page_cb,
                                               switcher);
            break;
        }
        node = g_list_next (node);
    }
    g_list_free (children);
}

/* Initialization.  */

static void
gdl_switcher_class_init (GdlSwitcherClass *klass)
{
    GtkNotebookClass *notebook_class = GTK_NOTEBOOK_CLASS (klass);
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    container_class->forall = gdl_switcher_forall;
    container_class->remove = gdl_switcher_remove;

    widget_class->size_request = gdl_switcher_size_request;
    widget_class->size_allocate = gdl_switcher_size_allocate;
    widget_class->expose_event = gdl_switcher_expose;
    widget_class->map = gdl_switcher_map;
    
    object_class->dispose  = gdl_switcher_dispose;
    object_class->finalize = gdl_switcher_finalize;
    object_class->set_property = gdl_switcher_set_property;
    object_class->get_property = gdl_switcher_get_property;
    
    g_object_class_install_property (
        object_class, PROP_SWITCHER_STYLE,
        g_param_spec_enum ("switcher-style", _("Switcher Style"),
                           _("Switcher buttons style"),
                           GDL_TYPE_SWITCHER_STYLE,
                           GDL_SWITCHER_STYLE_BOTH,
                           G_PARAM_READWRITE));
}

static void
gdl_switcher_instance_init (GdlSwitcher *switcher)
{
    GdlSwitcherPrivate *priv;

    GTK_WIDGET_SET_FLAGS (switcher, GTK_NO_WINDOW);
  
    priv = g_new0 (GdlSwitcherPrivate, 1);
    switcher->priv = priv;

    priv->show = TRUE;
    priv->buttons_height_request = -1;

    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (switcher), GTK_POS_BOTTOM);
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (switcher), FALSE);
    gtk_notebook_set_show_border (GTK_NOTEBOOK (switcher), FALSE);
    gdl_switcher_set_style (switcher, GDL_SWITCHER_STYLE_BOTH);
    
    /* notebook signals */
    g_signal_connect (switcher, "switch-page",
                      G_CALLBACK (gdl_switcher_switch_page_cb), switcher);
    g_signal_connect (switcher, "page-added",
                      G_CALLBACK (gdl_switcher_page_added_cb), switcher);
    g_signal_connect (switcher, "notify::show-tabs",
                      G_CALLBACK (gdl_switcher_notify_cb), switcher);
}

GtkWidget *
gdl_switcher_new (void)
{
    GdlSwitcher *switcher = g_object_new (gdl_switcher_get_type (), NULL);
    return GTK_WIDGET (switcher);
}

void
gdl_switcher_add_button (GdlSwitcher *switcher, const gchar *label,
                         const gchar *tooltips, const gchar *stock_id,
                         const GdkPixbuf *pixbuf_icon, gint switcher_id)
{
    GtkWidget *button_widget;
    GtkWidget *hbox;
    GtkWidget *icon_widget;
    GtkWidget *label_widget;
    GtkWidget *arrow;
    GtkTooltips *button_tooltips;
    
    button_widget = gtk_toggle_button_new ();
    if (switcher->priv->show)
        gtk_widget_show (button_widget);
    g_signal_connect (button_widget, "toggled",
                      G_CALLBACK (button_toggled_callback),
                      switcher);
    hbox = gtk_hbox_new (FALSE, 3);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_container_add (GTK_CONTAINER (button_widget), hbox);
    gtk_widget_show (hbox);

    if (stock_id)
        icon_widget = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
    else if (pixbuf_icon)
        icon_widget = gtk_image_new_from_pixbuf (pixbuf_icon);
    else
        icon_widget = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_BUTTON);

    gtk_widget_show (icon_widget);
    
    if (!label) {
        gchar *text = g_strdup_printf ("Item %d", switcher_id);
        label_widget = gtk_label_new (text);
        g_free (text);
    } else {
        label_widget = gtk_label_new (label);
    }
    gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);
    gtk_widget_show (label_widget);
    button_tooltips = gtk_tooltips_new();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_tooltips), button_widget,
                          tooltips, NULL);        

    switch (INTERNAL_MODE (switcher)) {
    case GDL_SWITCHER_STYLE_TEXT:
        gtk_box_pack_start (GTK_BOX (hbox), label_widget, TRUE, TRUE, 0);
        gtk_tooltips_disable (button_tooltips);
        break;
    case GDL_SWITCHER_STYLE_ICON:
        gtk_box_pack_start (GTK_BOX (hbox), icon_widget, TRUE, TRUE, 0);
        gtk_tooltips_enable (button_tooltips);
        break;
    case GDL_SWITCHER_STYLE_BOTH:
    default:
        gtk_box_pack_start (GTK_BOX (hbox), icon_widget, FALSE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), label_widget, TRUE, TRUE, 0);
        gtk_tooltips_disable (button_tooltips);
        break;
    }
    arrow = gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_NONE);
    gtk_widget_show (arrow);
    gtk_box_pack_start (GTK_BOX (hbox), arrow, FALSE, FALSE, 0);
    
    switcher->priv->buttons =
        g_slist_append (switcher->priv->buttons,
                        button_new (button_widget, label_widget,
                                    icon_widget, button_tooltips,
                                    arrow, hbox, switcher_id));
    gtk_widget_set_parent (button_widget, GTK_WIDGET (switcher));

    gtk_widget_queue_resize (GTK_WIDGET (switcher));
}

static void
gdl_switcher_remove_button (GdlSwitcher *switcher, gint switcher_id)
{
    GSList *p;

    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *button = p->data;

        if (button->id == switcher_id)
        {
            gtk_container_remove (GTK_CONTAINER (switcher),
                                  button->button_widget);
            break;
        }            
    }
    gtk_widget_queue_resize (GTK_WIDGET (switcher));
}

static void
gdl_switcher_select_button (GdlSwitcher *switcher, gint switcher_id)
{
    update_buttons (switcher, switcher_id);
    
    /* Select the notebook page associated with this button */
    gdl_switcher_select_page (switcher, switcher_id);
}

gint
gdl_switcher_insert_page (GdlSwitcher *switcher, GtkWidget *page,
                          GtkWidget *tab_widget, const gchar *label,
                          const gchar *tooltips, const gchar *stock_id,
                          const GdkPixbuf *pixbuf_icon, gint position)
{
    gint ret_position;
    gint switcher_id;
    g_signal_handlers_block_by_func (switcher,
                                     gdl_switcher_page_added_cb,
                                     switcher);
    
    if (!tab_widget) {
        tab_widget = gtk_label_new (label);
        gtk_widget_show (tab_widget);
    }
    switcher_id = gdl_switcher_get_page_id (page);
    gdl_switcher_add_button (switcher, label, tooltips, stock_id, pixbuf_icon, switcher_id);
    ret_position = gtk_notebook_insert_page (GTK_NOTEBOOK (switcher), page,
                                             tab_widget, position);
    g_signal_handlers_unblock_by_func (switcher,
                                       gdl_switcher_page_added_cb,
                                       switcher);
    return ret_position;
}

static void
set_switcher_style_internal (GdlSwitcher *switcher,
                             GdlSwitcherStyle switcher_style )
{
    GSList *p;
    
    if (switcher_style == GDL_SWITCHER_STYLE_TABS &&
        switcher->priv->show == FALSE)
        return;

    if (switcher_style == GDL_SWITCHER_STYLE_TABS)
    {
        gtk_notebook_set_show_tabs (GTK_NOTEBOOK (switcher), TRUE);
        return;
    }
    
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (switcher), FALSE);
    
    if (switcher_style == INTERNAL_MODE (switcher))
        return;
    
    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *button = p->data;

        gtk_container_remove (GTK_CONTAINER (button->hbox), button->arrow);
        switch (switcher_style) {
        case GDL_SWITCHER_STYLE_TEXT:
            gtk_container_remove (GTK_CONTAINER (button->hbox), button->icon);
            if (INTERNAL_MODE (switcher)
                == GDL_SWITCHER_STYLE_ICON) {
                gtk_box_pack_start (GTK_BOX (button->hbox), button->label,
                                    TRUE, TRUE, 0);
                gtk_widget_show (button->label);
                gtk_tooltips_disable (button->tooltips);
            }
            break;
        case GDL_SWITCHER_STYLE_ICON:
            gtk_container_remove(GTK_CONTAINER (button->hbox), button->label);
            if (INTERNAL_MODE (switcher)
                == GDL_SWITCHER_STYLE_TEXT) {
                gtk_box_pack_start (GTK_BOX (button->hbox), button->icon,
                                    TRUE, TRUE, 0);
                gtk_widget_show (button->icon);
            } else
                gtk_container_child_set (GTK_CONTAINER (button->hbox),
                                         button->icon, "expand", TRUE, NULL);
            gtk_tooltips_enable (button->tooltips);
            break;
        case GDL_SWITCHER_STYLE_BOTH:
            if (INTERNAL_MODE (switcher)
                == GDL_SWITCHER_STYLE_TEXT) {
                gtk_container_remove (GTK_CONTAINER (button->hbox),
                                      button->label);
                gtk_box_pack_start (GTK_BOX (button->hbox), button->icon,
                                    FALSE, TRUE, 0);
                gtk_widget_show (button->icon);
            } else {
                gtk_container_child_set (GTK_CONTAINER (button->hbox),
                                         button->icon, "expand", FALSE, NULL);
            }

            gtk_tooltips_disable (button->tooltips);
            gtk_box_pack_start (GTK_BOX (button->hbox), button->label, TRUE,
                                TRUE, 0);
            gtk_widget_show (button->label);
            break;
        default:
            break;
        }
        gtk_box_pack_start (GTK_BOX (button->hbox), button->arrow, FALSE,
                            FALSE, 0);
    }
}

#if HAVE_GNOME
static GConfEnumStringPair toolbar_styles[] = {
    { GDL_SWITCHER_STYLE_TEXT, "text" },
    { GDL_SWITCHER_STYLE_ICON, "icons" },
    { GDL_SWITCHER_STYLE_BOTH, "both" },
    { GDL_SWITCHER_STYLE_BOTH, "both-horiz" },
    { GDL_SWITCHER_STYLE_BOTH, "both_horiz" },
    { -1, NULL }
};

static void
style_changed_notify (GConfClient *gconf, guint id, GConfEntry *entry,
                      void *data)
{
    GdlSwitcher *switcher = data;
    char *val;
    int switcher_style;    
    
    val = gconf_client_get_string (gconf,
                                   "/desktop/gnome/interface/toolbar_style",
                                   NULL);
    if (val == NULL || !gconf_string_to_enum (toolbar_styles, val,
                                              &switcher_style))
        switcher_style = GDL_SWITCHER_STYLE_BOTH;
    g_free(val);

    set_switcher_style_internal (GDL_SWITCHER (switcher), switcher_style);
    switcher->priv->toolbar_style = switcher_style;

    gtk_widget_queue_resize (GTK_WIDGET (switcher));
}

static void
gdl_switcher_set_style (GdlSwitcher *switcher, GdlSwitcherStyle switcher_style)
{
    GConfClient *gconf_client = gconf_client_get_default ();
    
    if (switcher_style == GDL_SWITCHER_STYLE_TABS &&
        switcher->priv->show == FALSE)
        return;
    
    if (switcher->priv->switcher_style == switcher_style &&
        switcher->priv->show == TRUE)
        return;

    if (switcher->priv->switcher_style == GDL_SWITCHER_STYLE_TOOLBAR) {
        if (switcher->priv->style_changed_id) {
            gconf_client_notify_remove (gconf_client,
                                switcher->priv->style_changed_id);
            switcher->priv->style_changed_id = 0;
        }        
    }
    
    if (switcher_style != GDL_SWITCHER_STYLE_TOOLBAR) {
        set_switcher_style_internal (switcher, switcher_style);

        gtk_widget_queue_resize (GTK_WIDGET (switcher));
    } else {
        /* This is a little bit tricky, toolbar style is more
         * of a meta-style where the actual style is dictated by
         * the gnome toolbar setting, so that is why we have
         * the is_toolbar_style bool - it tracks the toolbar
         * style while the switcher_style member is the actual look and
         * feel */
        switcher->priv->style_changed_id =
            gconf_client_notify_add (gconf_client,
                                     "/desktop/gnome/interface/toolbar_style",
                                     style_changed_notify, switcher,
                                     NULL, NULL);
        style_changed_notify (gconf_client, 0, NULL, switcher);
    }
    
    g_object_unref (gconf_client);

    if (switcher_style != GDL_SWITCHER_STYLE_TABS)
        switcher->priv->switcher_style = switcher_style;
}

#else /* HAVE_GNOME */

static void
gdl_switcher_set_style (GdlSwitcher *switcher, GdlSwitcherStyle switcher_style)
{
    if (switcher_style == GDL_SWITCHER_STYLE_TABS &&
        switcher->priv->show == FALSE)
        return;
    
    if (switcher->priv->switcher_style == switcher_style &&
        switcher->priv->show == TRUE)
        return;

    set_switcher_style_internal (switcher,
                                 ((switcher_style ==
                                   GDL_SWITCHER_STYLE_TOOLBAR)?
                                  GDL_SWITCHER_STYLE_BOTH : switcher_style));
    gtk_widget_queue_resize (GTK_WIDGET (switcher));
    
    if (switcher_style != GDL_SWITCHER_STYLE_TABS)
        switcher->priv->switcher_style = switcher_style;
}

#endif /* HAVE_GNOME */

static void
gdl_switcher_set_show_buttons (GdlSwitcher *switcher, gboolean show)
{
    GSList *p;

    if (switcher->priv->show == show)
        return;
    
    for (p = switcher->priv->buttons; p != NULL; p = p->next) {
        Button *button = p->data;

        if (show)
            gtk_widget_show (button->button_widget);
        else
            gtk_widget_hide (button->button_widget);
    }

    switcher->priv->show = show;

    gtk_widget_queue_resize (GTK_WIDGET (switcher));
}

static GdlSwitcherStyle
gdl_switcher_get_style (GdlSwitcher *switcher)
{
    if (!switcher->priv->show)
        return GDL_SWITCHER_STYLE_TABS;
    return switcher->priv->switcher_style;
}
