/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Li Bing Chen 2012 <bingquick@gmail.com>
 * 
 * vmchooser is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * vmchooser is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gtk/gtk.h>

#include <glib/gi18n.h>
#include "vmchooser.h"

/* For testing propose use the local (not installed) ui file */
 
#define UI_FILE PACKAGE_DATA_DIR"/vmchooser/ui/vmchooser.ui"
#define CONF_FILE PACKAGE_DATA_DIR"/vmchooser/ui/vmchooser.conf"

/*
#define CONF_FILE "src/vmchooser.conf"
#define UI_FILE "src/vmchooser.ui"
*/

#define TOP_WINDOW "window"

 
/* Signal handlers */
/* Note: These may not be declared static because signal autoconnection
 * only works with non-static methods
 */

/* Called when the window is closed */

void
destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

void
clear_warn (GtkEditable *editable, gpointer user_data)
{
	if (sizeof(gtk_label_get_label(GTK_LABEL(wgWarn))) > 1)
	{
		gtk_label_set_label (GTK_LABEL(wgWarn),NULL);
	}
}

void
on_pass_activate (GtkEntry *entry, gpointer user_data)
{
	gtk_widget_grab_focus (btLogin);
}

void
on_id_activate (GtkEntry *entry, gpointer user_data)
{
	gtk_widget_grab_focus (GTK_WIDGET(wgPass));
}

void
on_button_clicked (GtkButton *button, gpointer user_data)
{
	const gchar* id = gtk_entry_get_text (GTK_ENTRY(wgID));
	const gchar* pass = gtk_entry_get_text (GTK_ENTRY(wgPass));
	const gchar* vm = gtk_combo_box_text_get_active_text 
		(GTK_COMBO_BOX_TEXT(wgChooser));
	if ( ! check_pass (id, pass))
	{
		gtk_label_set_label (GTK_LABEL(wgWarn), "Authentication failed!");
		gtk_widget_grab_focus (GTK_WIDGET(wgID));
	} else {
		g_print ("%s:%s\n", id, vm);
		gtk_main_quit ();
	}
}

static GtkWidget*

create_window (void)
{
	GtkWidget *window;
	GtkBuilder *builder;
	GError* error = NULL;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, NULL);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
        if (!window)
        {
                g_critical ("Widget \"%s\" is missing in file %s.",
				TOP_WINDOW,
				UI_FILE);
        }

	wgWarn = GTK_WIDGET (gtk_builder_get_object (builder, "warn"));
	wgBanner = GTK_WIDGET (gtk_builder_get_object (builder, "banner"));
	wgUsername = GTK_WIDGET (gtk_builder_get_object (builder, "username"));
	wgFootnote = GTK_WIDGET (gtk_builder_get_object (builder, "footnote"));
	wgID = GTK_WIDGET (gtk_builder_get_object (builder, "id"));
	wgPass = GTK_WIDGET (gtk_builder_get_object (builder, "pass"));
	wgChooser = GTK_WIDGET (gtk_builder_get_object (builder, "cb_chooser"));
	btLogin = GTK_WIDGET (gtk_builder_get_object (builder, "bt_login"));

	g_object_unref (builder);
	
	return window;
}


int
main (int argc, char *argv[])
{
 	GtkWidget *window;


#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	
	gtk_init (&argc, &argv);
	window = create_window ();
	load_conf(CONF_FILE);
	gtk_widget_show (window);

	gtk_main ();
	return 0;
}
