/* 
 * gnc-plugin-menu-additions.c -- 
 * Copyright (C) 2005 David Hampton hampton@employees.org>
 *
 * From:
 * gnc-menu-extensions.c -- functions to build dynamic menus
 * Copyright (C) 1999 Rob Browning
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact:
 *
 * Free Software Foundation           Voice:  +1-617-542-5942
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652
 * Boston, MA  02111-1307,  USA       gnu@gnu.org
 */

/** @addtogroup MenuPlugins
    @{ */
/** @addtogroup PluginMenuAdditions Non-GtkAction Menu Support
    @{ */
/** @internal
    @file gnc-plugin-menu-additions.c
    @brief Utility functions for writing import modules.
    @author Copyright (C) 2002 David Hampton <hampton@employees.org>
*/

#include "config.h"

#include <string.h>
#include <glib/gprintf.h>
#include <libgnome/libgnome.h>
#include <g-wrap-wct.h>

#include "guile-util.h"
#include "gnc-engine.h"
#include "gnc-main-window.h"
#include "gnc-plugin-menu-additions.h"
#include "gnc-window.h"
#include "gnc-trace.h"
#include "messages.h"
#include "gnc-gconf-utils.h"
#include "gnc-ui.h"
#include "gnc-menu-extensions.h"

static GObjectClass *parent_class = NULL;

static void gnc_plugin_menu_additions_class_init (GncPluginMenuAdditionsClass *klass);
static void gnc_plugin_menu_additions_init (GncPluginMenuAdditions *plugin);
static void gnc_plugin_menu_additions_finalize (GObject *object);

static void gnc_plugin_menu_additions_add_to_window (GncPlugin *plugin, GncMainWindow *window, GQuark type);
static void gnc_plugin_menu_additions_remove_from_window (GncPlugin *plugin, GncMainWindow *window, GQuark type);

/* Callbacks on other objects */
static void gnc_plugin_menu_additions_main_window_page_changed (GncMainWindow *window,
								GncPluginPage *page);

/* Command callbacks */

/* This static indicates the debugging module that this .o belongs to.  */
static QofLogModule log_module = GNC_MOD_GUI;


#define PLUGIN_ACTIONS_NAME "gnc-plugin-menu-additions-actions"

struct GncPluginMenuAdditionsPrivate
{
  gpointer dummy;
};

typedef struct _GncPluginMenuAdditionsPerWindow
{
  GncMainWindow  *window;
  GtkUIManager   *ui_manager;
  GtkActionGroup *group;
  gint merge_id;
} GncPluginMenuAdditionsPerWindow;

/************************************************************
 *                  Object Implementation                   *
 ************************************************************/

GType
gnc_plugin_menu_additions_get_type (void)
{
  static GType gnc_plugin_menu_additions_type = 0;

  if (gnc_plugin_menu_additions_type == 0) {
    static const GTypeInfo our_info = {
      sizeof (GncPluginMenuAdditionsClass),
      NULL,		/* base_init */
      NULL,		/* base_finalize */
      (GClassInitFunc) gnc_plugin_menu_additions_class_init,
      NULL,		/* class_finalize */
      NULL,		/* class_data */
      sizeof (GncPluginMenuAdditions),
      0,
      (GInstanceInitFunc) gnc_plugin_menu_additions_init
    };

    gnc_plugin_menu_additions_type = g_type_register_static (GNC_TYPE_PLUGIN,
							 "GncPluginMenuAdditions",
							 &our_info, 0);
  }

  return gnc_plugin_menu_additions_type;
}

static void
gnc_plugin_menu_additions_class_init (GncPluginMenuAdditionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GncPluginClass *plugin_class = GNC_PLUGIN_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = gnc_plugin_menu_additions_finalize;

  /* plugin info */
  plugin_class->plugin_name   = GNC_PLUGIN_MENU_ADDITIONS_NAME;

  /* function overrides */
  plugin_class->add_to_window = gnc_plugin_menu_additions_add_to_window;
  plugin_class->remove_from_window = gnc_plugin_menu_additions_remove_from_window;
}

static void
gnc_plugin_menu_additions_init (GncPluginMenuAdditions *plugin)
{
  ENTER("plugin %p", plugin);
  plugin->priv = g_new0 (GncPluginMenuAdditionsPrivate, 1);
  LEAVE("");
}

static void
gnc_plugin_menu_additions_finalize (GObject *object)
{
  GncPluginMenuAdditions *plugin;

  g_return_if_fail (GNC_IS_PLUGIN_MENU_ADDITIONS (object));

  plugin = GNC_PLUGIN_MENU_ADDITIONS (object);
  ENTER("plugin %p", plugin);

  g_return_if_fail (plugin->priv != NULL);
  g_free (plugin->priv);

  G_OBJECT_CLASS (parent_class)->finalize (object);
  LEAVE("");
}

GncPlugin *
gnc_plugin_menu_additions_new (void)
{
  GncPlugin *plugin_page = NULL;

  ENTER("");
  plugin_page = GNC_PLUGIN (g_object_new (GNC_TYPE_PLUGIN_MENU_ADDITIONS, NULL));
  LEAVE("plugin %p", plugin_page);
  return plugin_page;
}

/************************************************************
 *              Plugin Function Implementation              *
 ************************************************************/

static SCM
gnc_main_window_to_scm (GncMainWindow *window)
{
  static SCM main_window_type = SCM_UNDEFINED;

  if (!window)
    return SCM_BOOL_F;

  if (main_window_type == SCM_UNDEFINED)
  {
    main_window_type = scm_c_eval_string ("<gnc:MainWindow*>");

    /* don't really need this - types are bound globally anyway. */
    if (main_window_type != SCM_UNDEFINED)
      scm_gc_protect_object (main_window_type);
  }
  
  return gw_wcp_assimilate_ptr ((void *)window, main_window_type);
}

/** The user has selected one of the items in the File History menu.
 *  Close down the current session and start up a new one with the
 *  requested file.
 *
 *  @param action A pointer to the action selected by the user.  This
 *  action represents one of the items in the file history menu.
 *
 *  @param data A pointer to the gnc-main-window data to be used by
 *  this function.  This is mainly to find out which window it was
 *  that had a menu selected.  That's not really important for this
 *  function and we're about to close all the windows anyway.
 */
static void
gnc_plugin_menu_additions_action_cb (GtkAction *action,
				     GncMainWindowActionData *data)
{

  g_return_if_fail(GTK_IS_ACTION(action));
  g_return_if_fail(data != NULL);

  gnc_extension_invoke_cb(data->data, gnc_main_window_to_scm(data->window));
}

static void
gnc_menu_additions_menu_setup_one (ExtensionInfo *ext_info,
				   GncPluginMenuAdditionsPerWindow *per_window)
{
  GncMainWindowActionData *cb_data;

  DEBUG( "Adding %s/%s [%s] as [%s]\n", ext_info->path, ext_info->ae.label,
	 ext_info->ae.name, ext_info->typeStr );

  cb_data = g_new0 (GncMainWindowActionData, 1);
  cb_data->window = per_window->window;
  cb_data->data = ext_info->extension;

  if (ext_info->type == GTK_UI_MANAGER_MENUITEM)
    ext_info->ae.callback = (GCallback)gnc_plugin_menu_additions_action_cb;

  gtk_action_group_add_actions_full(per_window->group, &ext_info->ae, 1,
				    cb_data, g_free);
  gtk_ui_manager_add_ui(per_window->ui_manager, per_window->merge_id,
			ext_info->path, ext_info->ae.label, ext_info->ae.name,
			ext_info->type, FALSE);
  gtk_ui_manager_ensure_update(per_window->ui_manager);
}

/** Initialize the file history menu for a window.  This function is
 *  called as part of the initialization of a window, after all the
 *  plugin menu items have been added to the menu structure.  Its job
 *  is to correctly initialize the file history menu.  It does this by
 *  first calling a function that initializes the menu to the current
 *  as maintained in gconf.  It then creates a gconf client that will
 *  listens for any changes to the file history menu, and will update
 *  the meny when they are signalled.
 *
 *  @param plugin A pointer to the gnc-plugin object responsible for
 *  adding/removing the file history menu.
 *
 *  @param window A pointer the gnc-main-window that is being initialized.
 *
 *  @param type Unused
 */
static void
gnc_plugin_menu_additions_add_to_window (GncPlugin *plugin,
					 GncMainWindow *window,
					 GQuark type)
{
  GncPluginMenuAdditionsPerWindow per_window;
  GSList *menu_list;

  ENTER(" ");

  g_signal_connect (G_OBJECT(window), "page_changed",
		    G_CALLBACK (gnc_plugin_menu_additions_main_window_page_changed),
		    plugin);

  per_window.window = window;
  per_window.ui_manager = window->ui_merge;
  per_window.group = gtk_action_group_new ("MenuAdditions" );
  per_window.merge_id = gtk_ui_manager_new_merge_id(window->ui_merge);
  gtk_ui_manager_insert_action_group(window->ui_merge, per_window.group, 0);

  menu_list = gnc_extensions_get_menu_list();
  g_slist_foreach(menu_list, (GFunc)gnc_menu_additions_menu_setup_one,
		  &per_window);

  /* Tell the window code about the actions that were just added
   * behind its back (so to speak) */
  gnc_main_window_manual_merge_actions (window, PLUGIN_ACTIONS_NAME,
					per_window.group, per_window.merge_id);

  LEAVE(" ");
}


/** Finalize the file history menu for this window.  This function is
 *  called as part of the destruction of a window.
 *
 *  @param plugin A pointer to the gnc-plugin object responsible for
 *  adding/removing the file history menu.  It stops the gconf
 *  notifications for this window, and destroys the gconf client
 *  object.
 *
 *  @param window A pointer the gnc-main-window that is being destroyed.
 *
 *  @param type Unused
 */
static void
gnc_plugin_menu_additions_remove_from_window (GncPlugin *plugin,
					      GncMainWindow *window,
					      GQuark type)
{
  GtkActionGroup *group;

  ENTER(" ");

  g_signal_handlers_disconnect_by_func(G_OBJECT(window),
				       G_CALLBACK (gnc_plugin_menu_additions_main_window_page_changed),
				       plugin);

  /* Have to remove our actions manually. Its only automatic if the
   * actions name is installed into the plugin class. */
  group = gnc_main_window_get_action_group(window, PLUGIN_ACTIONS_NAME);
  if (group)
    gtk_ui_manager_remove_action_group(window->ui_merge, group);

  LEAVE(" ");
}

/************************************************************
 *                     Object Callbacks                     *
 ************************************************************/

static void
our_gtk_action_set_visible(GtkAction *action, gboolean visible)
{
  g_object_set(G_OBJECT(action), "visible", visible, NULL);
}


/** Whenever the current page has changed, update the reports menus based
 *  upon the page that is currently selected. */
static void
gnc_plugin_menu_additions_main_window_page_changed (GncMainWindow *window,
						    GncPluginPage *page)
{
  GtkActionGroup *action_group;
  GList *action_list;
  gboolean visible;
  gpointer tmp;

  ENTER("main window %p, page %p", window, page);
  action_group = gnc_main_window_get_action_group(window,PLUGIN_ACTIONS_NAME);
  if (action_group == NULL) {
    LEAVE("Can't find action group");
    return;
  }

  /* Does the now-visible page want menu-extensions to be visible? */
  if (page) {
    tmp = g_object_get_data(G_OBJECT(page),GNC_PLUGIN_HIDE_MENU_ADDITIONS_NAME);
    visible = !GPOINTER_TO_INT(tmp);
  } else {
    visible = TRUE;
  }

  action_list = gtk_action_group_list_actions(action_group);
  // Use the following line for gtk2.6
  // g_list_foreach(action_list, (GFunc)gtk_action_set_visible, (gpointer)visible);
  g_list_foreach(action_list, (GFunc)our_gtk_action_set_visible,
		 GINT_TO_POINTER(visible));
  g_list_free(action_list);

  LEAVE(" ");
}

/** @} */
/** @} */