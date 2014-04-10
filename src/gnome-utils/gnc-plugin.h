/* 
 * gnc-plugin.h -- A module or plugin which can add more
 *	functionality to gnucash.
 * Copyright (C) 2003 Jan Arne Petersen <jpetersen@uni-bonn.de>
 * Copyright (C) 2003,2005 David Hampton <hampton@employees.org>
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

/** @addtogroup GUI
    @{ */

/** @defgroup WindowsAndPlugin Window/Plugin Structure
 @{ */

/**
    @addtogroup Windows Windows
    @ingroup WindowsAndPlugin
*/

/**
    @addtogroup Plugins Plugins
    @ingroup WindowsAndPlugin
 @{
*/

/**
    @addtogroup MenuPlugins Menu Only Plugins
    @ingroup Plugins
*/

/**
    @addtogroup ContentPlugins Content Plugins
    @ingroup Plugins
*/

/** @} */
/** @} */
/** @} */

/** @addtogroup MenuPlugins
    @{ */
/** @addtogroup MenuPluginBase Common object and functions
    @{ */
/** @file gnc-plugin.h
    @brief Functions for adding plugins to a Gnucash window.
    @author Copyright (C) 2003 Jan Arne Petersen
    @author Copyright (C) 2003,2005 David Hampton <hampton@employees.org>
*/

#ifndef __GNC_PLUGIN_H
#define __GNC_PLUGIN_H

#include "gnc-main-window.h"
#include "gnc-plugin-page.h"
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

/* type macros */
#define GNC_TYPE_PLUGIN            (gnc_plugin_get_type ())
#define GNC_PLUGIN(o)              (G_TYPE_CHECK_INSTANCE_CAST ((o), GNC_TYPE_PLUGIN, GncPlugin))
#define GNC_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GNC_TYPE_PLUGIN, GncPluginClass))
#define GNC_IS_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNC_TYPE_PLUGIN))
#define GNC_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNC_TYPE_PLUGIN))
#define GNC_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNC_PLUGIN, GncPluginClass))

/* typedefs & structures */
typedef struct GncPluginPrivate GncPluginPrivate;

typedef struct {
	GObject parent;
	GncPluginPrivate *priv;
} GncPlugin;

typedef struct {
	GObjectClass parent;
	const gchar *plugin_name;

	const gchar *actions_name;
	GtkActionEntry *actions;
	guint n_actions; 
	const gchar **important_actions;
	const gchar *ui_filename;

	const gchar* gconf_section;
	void (* gconf_notifications) (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);

	/* Virtual Table */
	void (* add_to_window)
         (GncPlugin *plugin, GncMainWindow *window, GQuark type);
	void (* remove_from_window)
         (GncPlugin *plugin, GncMainWindow *window, GQuark type);

	GncPluginPage *(* create_page)
         (GncPlugin *plugin, const gchar *uri);
} GncPluginClass;

/* function prototypes */

/** Get the type of a gnc window plugin.
 *
 *  @return A GType.
 */
GType gnc_plugin_get_type (void);


/** Add the specified plugin from the specified window.  This function
 *  will add the page's user interface from the window, set up gconf
 *  notifications if the page uses gconf, and call the plugin to
 *  perform any plugin specific actions.
 *
 *  @param plugin The plugin to be added.
 *
 *  @param window Add the plugin to this window.
 *
 *  @param type An identifier for the type of window specified.
 */
void gnc_plugin_add_to_window (GncPlugin *plugin,
			       GncMainWindow *window,
			       GQuark type);


/** Remove the specified plugin from the specified window.  This
 *  function will call the plugin to perform any plugin specific
 *  actions, remove any gconf notifications that were set up for the
 *  page, and remove the page's user interface from the window.
 *
 *  @param plugin The plugin to be removed.
 *
 *  @param window The window the plugin should be removed from.
 *
 *  @param type An identifier for the type of window specified.
 */
void gnc_plugin_remove_from_window (GncPlugin *plugin,
				    GncMainWindow *window,
				    GQuark type);


GncPluginPage *gnc_plugin_create_page (GncPlugin *plugin,
				       const gchar *uri);


/** Retrieve the name of a plugin.
 *
 *  @param plugin The plugin whose name should be returned.
 *
 *  @return  short_labels A pointer to a data structure containing
 *  [action name, label string] string pairs.
 */
const gchar *gnc_plugin_get_name (GncPlugin *plugin);


typedef struct {
  const char *action_name;
  const char *label;
} action_short_labels;

/** Add "short" labels to existing actions.  The "short" label is the
 *  string used on toolbar buttons when the action is visible.
 *
 *  @param action_group The group of all actions associated with a
 *  plugin or plugin page.  All actions to me modified must be in this
 *  group.
 *
 *  @param short_labels A pointer to a data structure containing
 *  [action name, label string] string pairs.
 */
void gnc_plugin_init_short_names (GtkActionGroup *action_group,
				  action_short_labels *short_labels);


/** Mark certain actions as "important".  This means that their labels
 *  will appear when the toolbar is set to "Icons and important text"
 *  (e.g. GTK_TOOLBAR_BOTH_HORIZ) mode.
 *
 *  @param action_group The group of all actions associated with a
 *  plugin or plugin page.  All actions to me modified must be in this
 *  group.
 *
 *  @param name A list of actions names to be marked important.  This
 *  list must be NULL terminated.
 */
void gnc_plugin_set_important_actions (GtkActionGroup *action_group,
				       const gchar **names);


/** Update a property on a set of existing GtkActions.  This function
 *  can be easily used to make a list of actions visible, invisible,
 *  sensitive, or insensitive.
 *
 *  @param action_group The group of all actions associated with a
 *  plugin or plugin page.  All actions to be modified must be
 *  contained in this group.
 *
 *  @param action_names A NULL terminated list of actions names that
 *  should modified.
 *
 *  @param property_name The property name to be changed on the
 *  specified actions. The only two GtkAction properties that it makes
 *  sense to modify are "visible" and "sensitive".
 *
 *  @param value A boolean specifying the new state for the specified
 *  property.
 */
void gnc_plugin_update_actions (GtkActionGroup *action_group,
				const gchar **action_names,
				const gchar *property_name,
				gboolean value);


/** Load a new set of actions into an existing UI.
 *
 *  @param ui_merge The existing set of merged actions. This is the ui
 *  that a user sees.  The actions from the ui file will be added to
 *  this ui.
 *
 *  @param action_group The local action group.  The actions from the
 *  ui file will be added to this private action group.
 *
 *  @param filename The name of the ui file to load.  This file name
 *  will be searched for in the ui directory.
 *
 *  @return The merge_id number for the newly merged UI.  If an error
 *  occurred, the return value is 0.
 */
gint gnc_plugin_add_actions (GtkUIManager *ui_merge,
			     GtkActionGroup *action_group,
			     const gchar *filename);
G_END_DECLS

#endif /* __GNC_PLUGIN_H */

/** @} */
/** @} */