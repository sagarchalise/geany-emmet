/*
 *      demoplugin.c - this file is part of Geany, a fast and lightweight IDE
 *
 *      Copyright 2007-2012 Enrico Tröger <enrico(dot)troeger(at)uvena(dot)de>
 *      Copyright 2007-2012 Nick Treleaven <nick(dot)treleaven(at)btinternet(dot)com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * Demo plugin - example of a basic plugin for Geany. Adds a menu item to the
 * Tools menu.
 *
 * Note: This is not installed by default, but (on *nix) you can build it as follows:
 * cd plugins
 * make demoplugin.so
 *
 * Then copy or symlink the plugins/demoplugin.so file to ~/.config/geany/plugins
 * - it will be loaded at next startup.
 */


#include "geanyplugin.h"	/* plugin API, always comes first */
#include "Scintilla.h"	/* for the SCNotification struct */
#include "jsapi.h"
#include "js/Initialization.h"
static JSClassOps global_ops = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    JS_GlobalObjectTraceHook
};

/* The class of the global object. */
static JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    &global_ops
};

extern "C" {
extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
#if GEANY_API_VERSION < 224
extern GeanyFunctions *geany_functions;
#endif
}
static GtkWidget *emmet_menu = NULL;
GtkWidget *main_window = NULL;
gchar *emmet_file = NULL;
gchar *editor_file = NULL;
JSContext *cx;
gint emmet_action_len = 18;
gint indicate_box = 16;
gint emmet_key_group = 4;
// gchar *highlight_tag = "highlight_tag";
const gchar *emmet_actions[18] = {
  "expand_abbreviation",
  "matching_pair",
  "wrap_with_abbreviation",
  "reflect_css_value",
  "encode_decode_data_url",
  "prev_edit_point",
  "next_edit_point",
  "evaluate_math_expression", 
  "insert_formatted_line_break",
  "balance_inward",
  "balance_outward",
  "merge_lines",
  "remove_tag",
  "select_next_item",
  "select_previous_item",
  "split_join_tag",
  "update_image_size",
  "update_tag"
};




// JS::Handle<JSObject> *global;
gint caret_pos(GeanyDocument *doc){
    return sci_get_current_position(doc->editor->sci);
}
static bool get_caret_pos(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint pos = caret_pos(doc);
    args.rval().set(JS::NumberValue(pos));
    return true;
}
static bool set_caret_pos(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    gint pos = args[0].toNumber();
    GeanyDocument *doc = document_get_current();
    sci_set_current_position(doc->editor->sci, pos, TRUE);
    // args.rval().setNull();
    return true;
}
static bool highlight_matching_tag(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    gint start = args[0].toNumber();
    gint end = args[1].toNumber();
    if(start == end){
        return true;
    }
    GeanyDocument *doc = document_get_current();
    editor_indicator_set_on_range(doc->editor, indicate_box, start, end);
    return true;
}
static bool create_selection(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    gint start;
    GeanyDocument *doc = document_get_current();
    if(args[0].isUndefined()){
        start = caret_pos(doc);
    }
    else{
        start = args[0].toNumber();
    }
    gint end;
    if(args[1].isUndefined()){
        end = start;
    }
    else{
        end = args[1].toNumber();
    }
    sci_set_selection_start(doc->editor->sci, start);
    sci_set_selection_end(doc->editor->sci, end);
    // args.rval().setNull();
    return true;
}
static bool get_selection_start(JSContext *cx, unsigned argc, JS::Value *vp)
{
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint pos;
    if(sci_has_selection(doc->editor->sci)){
        pos = sci_get_selection_start(doc->editor->sci);
    }
    else{
        pos = caret_pos(doc);
    }
    args.rval().set(JS::NumberValue(pos));
    return true;
}
static bool get_selection_end(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint pos;
    if(sci_has_selection(doc->editor->sci)){
        pos = sci_get_selection_end(doc->editor->sci);
    }
    else{
        pos = caret_pos(doc);
    }
    args.rval().set(JS::NumberValue(pos));
    return true;
}
static bool get_line_start(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint line = sci_get_current_line(doc->editor->sci);
    args.rval().set(JS::NumberValue(sci_get_position_from_line(doc->editor->sci, line)));
    return true;
}
static bool get_line_end(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint line = sci_get_current_line(doc->editor->sci);
    args.rval().set(JS::NumberValue(sci_get_line_end_position(doc->editor->sci, line)));
    return true;
}
static bool replace_content(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    gint start = args[1].toNumber();
    GeanyDocument *doc = document_get_current();
    if(sci_has_selection(doc->editor->sci)){
        sci_replace_sel(doc->editor->sci, "");
    }
    editor_insert_snippet(doc->editor, start, JS_EncodeString(cx, args[0].toString()));
    return true;
}
static bool
get_file_path(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // /*
   // * Look in argv for argc actual parameters, set *rval to return a
   // * value to the caller.
   // *
   // * ex. Add two arguments as integer.
   // * args.rval().setInt32(args[0].toNumber() + args[1].toNumber());
   // */
   GeanyDocument *doc = document_get_current();
   args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, doc->file_name, g_utf8_strlen(doc->file_name, -1)+1)));
   return true;
}
static bool
get_content(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
   GeanyDocument *doc = document_get_current();
   args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, sci_get_contents(doc->editor->sci, -1), sci_get_length(doc->editor->sci))));
   return true;
}
static bool
get_selection_content(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
   GeanyDocument *doc = document_get_current();
   if (sci_has_selection(doc->editor->sci)){
        args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, sci_get_selection_contents(doc->editor->sci), sci_get_selected_text_length(doc->editor->sci))));
   }
   else{
       args.rval().setNull();
   }
   return true;
}
static bool
get_syntax(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  GeanyDocument *doc = document_get_current();
   const gchar *val = filetypes_get_display_name(doc->file_type);
    args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, val, strlen(val))));
   return true;
}
static bool
prompt(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  gchar *abbr = dialogs_show_input("Wrap With Abbreviation", GTK_WINDOW(main_window), NULL, NULL);
  args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, abbr, strlen(abbr))));
return true;
}
static bool
get_current_line(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    args.rval().set(JS::NumberValue(sci_get_current_line(doc->editor->sci)));
    return true;
}
static bool
get_current_line_content(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    gint line = sci_get_current_line(doc->editor->sci);
    args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, sci_get_line(doc->editor->sci, line), sci_get_line_length(doc->editor->sci, line))));
    return true;
}
static bool
is_tab_used(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    args.rval().set(JS::BooleanValue((doc->editor->indent_type == GEANY_INDENT_TYPE_TABS)?TRUE:FALSE));
    return true;
}
static bool
get_tab_width(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    args.rval().set(JS::NumberValue(doc->editor->indent_width));
    return true;
}
static bool
get_eol_char(JSContext *cx, unsigned argc, JS::Value *vp)
{
  JS::CallArgs args = CallArgsFromVp(argc, vp);
    GeanyDocument *doc = document_get_current();
    args.rval().set(JS::StringValue(JS_NewStringCopyN(cx, editor_get_eol_char(doc->editor), editor_get_eol_char_len(doc->editor))));
    return true;
}
static JSFunctionSpec my_functions[] = {
  JS_FN("getGeanyFilePath", get_file_path, 0, 0),
  JS_FN("getGeanyCaretPos", get_caret_pos, 0, 0),
  JS_FN("setGeanyCaretPos", set_caret_pos, 1, 0),
  JS_FN("getGeanyCurrentLine", get_current_line, 0, 0),
  JS_FN("createGeanySelection", create_selection, 2, 0),
  JS_FN("getGeanySelectionStart", get_selection_start, 0, 0),
  JS_FN("getGeanySelectionEnd", get_selection_end, 0, 0),
  JS_FN("getGeanyCurrentLineStart", get_line_start, 0, 0),
  JS_FN("getGeanyCurrentLineEnd", get_line_end, 0, 0),
  JS_FN("replaceGeanyContent", replace_content, 4, 0),
  JS_FN("getGeanyContent", get_content, 0, 0),
  JS_FN("getGeanyLineContent", get_current_line_content, 0, 0),
  JS_FN("getGeanySelection", get_selection_content, 0, 0),
  JS_FN("geanyPrompt", prompt, 0, 0),
  JS_FN("highlightGeanyTag", highlight_matching_tag, 2, 0),
  JS_FN("getGeanySyntax", get_syntax, 0, 0),
  JS_FN("isGeanyTabUsed", is_tab_used, 0, 0),
  JS_FN("getGeanyTabWidth", get_tab_width, 0, 0),
  JS_FN("getGeanyEOLChar", get_eol_char, 0, 0),
  // /* etc... */
  JS_FS_END
};
// The error reporter callback.
void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
     fprintf(stderr, "%s:%u:%s\n",
             report->filename ? report->filename : "[no filename]",
             (unsigned int) report->lineno,
             message);
}
static void run_emmet(const gchar *action){
    cx = JS_NewContext(8L * 1024 * 1024);
    if (!cx)
        return;
    if (!JS::InitSelfHostedCode(cx))
        return;
    { 

      JSAutoRequest ar(cx); 

      JS::CompartmentOptions options;
      JS::RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook, options));
      if (!global)
          return;

      JS::RootedValue rval(cx);
      // JS::RootedValue rval1(cx);

      { // Scope for JSAutoCompartment
        JSAutoCompartment ac(cx, global);
        JS_InitStandardClasses(cx, global);
        if (!JS_DefineFunctions(cx, global, my_functions)){
            printf("%d\n", 4);
            return;
        
        }
        JS::CompileOptions opts(cx);
        const char *script = "var self=this;";
        const char *filename = "bootstrap.js";
        int lineno = 1;
        opts.setFileAndLine(filename, lineno);
        if(!JS::Evaluate(cx, opts, script, strlen(script), &rval)){
            printf("%d\n", 50);
        }
        // JS::CompileOptions opts(cx);
        opts.setFile(emmet_file);
        if(!JS::Evaluate(cx, opts, emmet_file, &rval)){
            printf("%d\n", 3);
            return;
        }
        opts.setFile(editor_file);
        if(!JS::Evaluate(cx, opts, editor_file, &rval)){
            printf("%d\n", 2);
            return;
        }
        JS::AutoValueArray<1> argv(cx);
        // argv[0].setNull();
        // printf("%d\n", GPOINTER_TO_INT(gdata));
        argv[0].set(JS::StringValue(JS_NewStringCopyN(cx, action, strlen(action))));
        bool ok = JS_CallFunctionName(cx, global, "runAction", argv, &rval);
        if (!ok){
            printf("%d\n", 1000);
            return;
        }
        if(rval.isString()){
            JSString *str = rval.toString();
            printf("%s\n", JS_EncodeString(cx, str));
        }
      }
    }
    JS_DestroyContext(cx);
    return;
}
static gboolean check_doc(GeanyDocument *doc){
    if(!DOC_VALID(doc)){
        return FALSE;
    }
    if(doc->file_type->id == GEANY_FILETYPES_HTML || doc->file_type->id == GEANY_FILETYPES_XML || doc->file_type->id == GEANY_FILETYPES_CSS || doc->file_type->id == GEANY_FILETYPES_PHP){
        return TRUE;
    }
    return FALSE;
}
static void menu_item_action(G_GNUC_UNUSED GtkMenuItem *menuitem, gpointer gdata){
    GeanyDocument *doc = document_get_current();
    if(!check_doc(doc)){
        return;
    }
    const gchar *action = emmet_actions[GPOINTER_TO_INT(gdata)];
    run_emmet(action);
}
static gboolean demo_init(GeanyPlugin *plugin, gpointer data){
    JS_Init();
    emmet_file = g_build_path(G_DIR_SEPARATOR_S, plugin->geany_data->app->configdir, "plugins", "emmet", "emmet.js", NULL);
    editor_file = g_build_path(G_DIR_SEPARATOR_S, plugin->geany_data->app->configdir, "plugins", "emmet", "editor.js", NULL);
    GtkMenuShell *menubar;
    GtkWidget *sub_menu = NULL;
    GeanyKeyGroup *key_group;
    GList *menubar_children;
    main_window = plugin->geany_data->main_widgets->window;
    menubar = GTK_MENU_SHELL(ui_lookup_widget(main_window, "menubar1"));
    emmet_menu = gtk_menu_item_new_with_label("Emmet");
    sub_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(emmet_menu), sub_menu);
    GtkWidget *menu_items[emmet_action_len] = {
        gtk_menu_item_new_with_label("Expand Abbreviation"),
        gtk_menu_item_new_with_label("Go To Matching Pair"),
        gtk_menu_item_new_with_label("Wrap With Abbreviation"),
        gtk_menu_item_new_with_label("Reflect Css Value"),
        gtk_menu_item_new_with_label("Encode/Decode Data URL"),
        gtk_menu_item_new_with_label("Prev Edit Point"),
        gtk_menu_item_new_with_label("Next Edit Point"),
        gtk_menu_item_new_with_label("Evaluate Math Expression"), 
        gtk_menu_item_new_with_label("Insert Formatted Line Break"),
        gtk_menu_item_new_with_label("Balance Inward"),
        gtk_menu_item_new_with_label("Balance Outward"),
        gtk_menu_item_new_with_label("Merge Lines"),
        gtk_menu_item_new_with_label("Remove Tag"),
        gtk_menu_item_new_with_label("Select Next Item"),
        gtk_menu_item_new_with_label("Select Previous Item"),
        gtk_menu_item_new_with_label("Split Join Tag"),
        gtk_menu_item_new_with_label("Update Image Size"),
        gtk_menu_item_new_with_label("Update Tag")
    };
    key_group = plugin_set_key_group(plugin, "emmet", emmet_key_group, NULL);
    for(gint i=0; i < emmet_action_len; i++){
        g_signal_connect(menu_items[i], "activate", G_CALLBACK(menu_item_action), GINT_TO_POINTER(i));
        gtk_container_add(GTK_CONTAINER(sub_menu), menu_items[i]);
        if(i < emmet_key_group){
                keybindings_set_item(key_group, i, NULL,
                    0, GEANY_PRIMARY_MOD_MASK, emmet_actions[i], gtk_menu_item_get_label(GTK_MENU_ITEM(menu_items[i])), menu_items[i]);
        }
    }
    gtk_widget_set_sensitive(emmet_menu, FALSE);
    gtk_widget_show_all(emmet_menu);
    menubar_children = gtk_container_get_children(GTK_CONTAINER(menubar));
    gtk_menu_shell_insert(menubar, emmet_menu, g_list_length(menubar_children) - 1);
    g_list_free(menubar_children);
    return TRUE;
}

/* Called by Geany to show the plugin's configure dialog. This function is always called after
 * demo_init() was called.
 * You can omit this function if the plugin doesn't need to be configured.
 * Note: parent is the parent window which can be used as the transient window for the created
 *       dialog. */

/* Called by Geany before unloading the plugin.
 * Here any UI changes should be removed, memory freed and any other finalization done.
 * Be sure to leave Geany as it was before demo_init(). */
static void demo_cleanup(GeanyPlugin *plugin, gpointer data)
{
	/* remove the menu item added in demo_init() */
    JS_ShutDown();
    // for(int i=0; i < emmet_action_len; i++){
        // g_free(emmet_actions[i]);
    // }
    // g_free(emmet_actions);
    g_free(emmet_file);
    g_free(editor_file);
    // g_free(highlight_tag);
	gtk_widget_destroy(emmet_menu);
	/* release other allocated strings and objects */
}
static void on_document_action(GObject *object, GeanyDocument *doc,
								 gpointer data)
{
	/* data == GeanyPlugin because the data member of PluginCallback was set to NULL
	 * and this plugin has called geany_plugin_set_data() with the GeanyPlugin pointer as
	 * data */
     if(check_doc(doc)){
        gtk_widget_set_sensitive(emmet_menu, TRUE);
     }
    else{
        gtk_widget_set_sensitive(emmet_menu, FALSE);
    }
}
static void on_document_close(GObject *object, GeanyDocument *doc,
								 gpointer data)
{
	/* data == GeanyPlugin because the data member of PluginCallback was set to NULL
	 * and this plugin has called geany_plugin_set_data() with the GeanyPlugin pointer as
	 * data */
        gtk_widget_set_sensitive(emmet_menu, FALSE);
    // JS_DestroyRuntime(rt);
}
// static gboolean on_editor_notify(GObject *obj, GeanyEditor *editor, SCNotification *nt, gpointer user_data){
    // GeanyDocument *doc = editor->document;
    // if(!check_doc(doc)){
        // return FALSE;
    // };
    // editor_indicator_clear(doc->editor, indicate_box);
    // if(doc->file_type->id != GEANY_FILETYPES_CSS){
        // switch(nt->nmhdr.code){
            // case SCN_UPDATEUI:
            // case SCN_KEY:
            // {
                // run_emmet(highlight_tag);
                // break;
            // }
        // }
    // }
    // return TRUE;
// }
static PluginCallback demo_callbacks[] =
{
	/* Set 'after' (third field) to TRUE to run the callback @a after the default handler.
	 * If 'after' is FALSE, the callback is run @a before the default handler, so the plugin
	 * can prevent Geany from processing the notification. Use this with care. */
	// { "editor-notify", (GCallback) &on_editor_notify, FALSE, NULL },
	{ "document-reload", (GCallback) &on_document_action, FALSE, NULL },
	{ "document-save", (GCallback) &on_document_action, FALSE, NULL },
	{ "document-activate", (GCallback) &on_document_action, FALSE, NULL },
	{ "document-close", (GCallback) &on_document_close, FALSE, NULL },
	{ "document-open", (GCallback) &on_document_action, FALSE, NULL },
	{ NULL, NULL, FALSE, NULL }
};

extern "C" void geany_load_module(GeanyPlugin *plugin)
{
	/* main_locale_init() must be called for your package before any localization can be done */
	main_locale_init(LOCALEDIR, GETTEXT_PACKAGE);
	plugin->info->name = _("Emmet");
	plugin->info->description = _("Emmet Plugin.");
	plugin->info->version = "0.4";
	plugin->info->author =  _("Sagar Chalise");

	plugin->funcs->init = demo_init;
	plugin->funcs->configure = NULL;
	plugin->funcs->help = NULL; /* This demo has no help but it is an option */
	plugin->funcs->cleanup = demo_cleanup;
	plugin->funcs->callbacks = demo_callbacks;

	GEANY_PLUGIN_REGISTER(plugin, 225);
}
