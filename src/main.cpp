#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

struct Bookmark {
    std::string title;
    std::string url;
};

std::vector<Bookmark> bookmarks;
std::vector<std::string> history;
GtkWidget *main_window;
GtkNotebook *notebook;

// Function declarations
void initialize_ui();
void load_bookmarks();
void save_bookmarks();
void add_bookmark(const std::string &title, const std::string &url);
void edit_bookmark(const std::string &old_title, const std::string &new_title, const std::string &url);
void delete_bookmark(const std::string &title);
void display_bookmarks_ui();
void display_settings_ui();
void show_splash_screen();
void create_new_tab(const std::string &html_content);
void on_new_tab_button_clicked(GtkWidget *widget, gpointer data);
void on_bookmarks_button_clicked(GtkWidget *widget, gpointer data);
void on_settings_button_clicked(GtkWidget *widget, gpointer data);
void fetch_page_title(WebKitWebView *web_view, const std::string &url);
void add_bookmark_from_current_tab();
void search_web(const std::string &query);
void add_to_history(const std::string &url);
void display_history_ui();
void on_history_button_clicked(GtkWidget *widget, gpointer data);
std::string get_current_tab_url();
std::string get_current_tab_title();
void handle_alpha_scheme(const std::string &url);

// Main function
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    show_splash_screen();
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    initialize_ui();
    load_bookmarks();
    gtk_widget_show_all(main_window);
    gtk_main();
    return 0;
}

// UI Initialization
void initialize_ui() {
    gtk_window_set_title(GTK_WINDOW(main_window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    
    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(notebook));
    
    GtkToolbar *toolbar = GTK_TOOLBAR(gtk_toolbar_new());
    GtkToolItem *new_tab_button = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    gtk_toolbar_insert(toolbar, new_tab_button, -1);

    GtkToolItem *bookmarks_button = gtk_tool_button_new_from_stock(GTK_STOCK_BOOKMARK);
    g_signal_connect(bookmarks_button, "clicked", G_CALLBACK(on_bookmarks_button_clicked), NULL);
    gtk_toolbar_insert(toolbar, bookmarks_button, -1);

    GtkToolItem *history_button = gtk_tool_button_new_from_stock(GTK_STOCK_INDEX);
    g_signal_connect(history_button, "clicked", G_CALLBACK(on_history_button_clicked), NULL);
    gtk_toolbar_insert(toolbar, history_button, -1);

    GtkToolItem *settings_button = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    gtk_toolbar_insert(toolbar, settings_button, -1);

    gtk_box_pack_start(GTK_BOX(gtk_hbox_new(FALSE, 0)), GTK_WIDGET(toolbar), FALSE, FALSE, 0);
}

// Load Bookmarks
void load_bookmarks() {
    std::ifstream file("bookmarks.json");
    if (file) {
        JsonParser *parser = json_parser_new();
        json_parser_load_from_file(parser, "bookmarks.json", NULL);
        JsonNode *root = json_parser_get_root(parser);
        JsonArray *array = json_node_get_array(root);
        guint length = json_array_get_length(array);
        
        for (guint i = 0; i < length; i++) {
            JsonNode *node = json_array_get_element(array, i);
            JsonObject *object = json_node_get_object(node);
            Bookmark bookmark;
            bookmark.title = json_object_get_string(object, "title");
            bookmark.url = json_object_get_string(object, "url");
            bookmarks.push_back(bookmark);
        }
        g_object_unref(parser);
    }
}

// Save Bookmarks
void save_bookmarks() {
    JsonGenerator *generator = json_generator_new();
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);

    for (const auto& bookmark : bookmarks) {
        JsonNode *bookmark_node = json_node_new(JSON_NODE_OBJECT);
        JsonObject *bookmark_object = json_object_new();
        json_object_set_string(bookmark_object, "title", bookmark.title.c_str());
        json_object_set_string(bookmark_object, "url", bookmark.url.c_str());
        json_node_take_object(bookmark_node, bookmark_object);
        json_array_add_element_element(json_node_get_array(root), bookmark_node);
    }

    json_generator_set_root(generator, root);

    GError *error = NULL;
    gboolean success = json_generator_to_file(generator, "bookmarks.json", &error);
    
    if (!success) {
        g_printerr("Error saving bookmarks: %s\n", error->message);
        g_error_free(error);
    }

    json_node_unref(root);
    g_object_unref(generator);
}

// Add Bookmark
void add_bookmark(const std::string &title, const std::string &url) {
    bookmarks.push_back({title, url});
    save_bookmarks();
}

// Edit Bookmark
void edit_bookmark(const std::string &old_title, const std::string &new_title, const std::string &url) {
    auto it = std::find_if(bookmarks.begin(), bookmarks.end(), [&](const Bookmark &b) {
        return b.title == old_title;
    });
    if (it != bookmarks.end()) {
        it->title = new_title;
        it->url = url;
        save_bookmarks();
    }
}

// Delete Bookmark
void delete_bookmark(const std::string &title) {
    bookmarks.erase(std::remove_if(bookmarks.begin(), bookmarks.end(),
                                    [&](const Bookmark &b) { return b.title == title; }),
                    bookmarks.end());
    save_bookmarks();
}

// Display Bookmarks UI
void display_bookmarks_ui() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Bookmarks",
        GTK_WINDOW(main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL);

    GtkWidget *list = gtk_list_box_new();
    for (const auto& bookmark : bookmarks) {
        GtkWidget *row = gtk_label_new(bookmark.title.c_str());
        gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
        
        // Right-click menu for edit/delete
        g_signal_connect(row, "button-press-event", G_CALLBACK([](GtkWidget *widget, GdkEvent *event) {
            if (event->type == GDK_BUTTON_PRESS && event->button == 3) { // Right-click
                GtkWidget *menu = gtk_menu_new();
                GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");
                GtkWidget *delete_item = gtk_menu_item_new_with_label("Delete");
                
                g_signal_connect(edit_item, "activate", G_CALLBACK([](GtkWidget *widget, gpointer data) {
                    Bookmark *bookmark = (Bookmark*)data;
                    GtkWidget *dialog = gtk_dialog_new_with_buttons(
                        "Edit Bookmark",
                        GTK_WINDOW(main_window),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        "_Save",
                        GTK_RESPONSE_ACCEPT,
                        "_Cancel",
                        GTK_RESPONSE_REJECT,
                        NULL);

                    GtkWidget *entry_title = gtk_entry_new();
                    GtkWidget *entry_url = gtk_entry_new();
                    
                    gtk_entry_set_text(GTK_ENTRY(entry_title), bookmark->title.c_str());
                    gtk_entry_set_text(GTK_ENTRY(entry_url), bookmark->url.c_str());

                    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
                    gtk_box_pack_start(GTK_BOX(content_area), entry_title, TRUE, TRUE, 0);
                    gtk_box_pack_start(GTK_BOX(content_area), entry_url, TRUE, TRUE, 0);
                    gtk_widget_show_all(dialog);
                    
                    int response = gtk_dialog_run(GTK_DIALOG(dialog));
                    if (response == GTK_RESPONSE_ACCEPT) {
                        const char *new_title = gtk_entry_get_text(GTK_ENTRY(entry_title));
                        const char *new_url = gtk_entry_get_text(GTK_ENTRY(entry_url));
                        edit_bookmark(bookmark->title, new_title, new_url);
                    }
                    gtk_widget_destroy(dialog);
                }), (gpointer)&bookmark);

                g_signal_connect(delete_item, "activate", G_CALLBACK([](GtkWidget *widget, gpointer data) {
                    Bookmark *bookmark = (Bookmark*)data;
                    delete_bookmark(bookmark->title);
                }), (gpointer)&bookmark);

                gtk_menu_shell_append(GTK_MENU_SHELL(menu), edit_item);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_item);
                gtk_widget_show_all(menu);
                gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
            }
            return FALSE;
        }), (gpointer)&bookmark);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), list, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Display Settings UI
void display_settings_ui() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Settings",
        GTK_WINDOW(main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL);
    
    // Add settings options here, for now just a placeholder
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Settings go here."), TRUE, TRUE, 0);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Show Splash Screen
void show_splash_screen() {
    GtkWidget *splash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(splash_window), "Welcome to AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(splash_window), 300, 200);
    GtkWidget *label = gtk_label_new("Loading...");
    gtk_container_add(GTK_CONTAINER(splash_window), label);
    gtk_widget_show_all(splash_window);
    gtk_timeout_add(3000, (GSourceFunc)gtk_widget_destroy, splash_window); // Auto-close after 3 seconds
}

// Create New Tab
void create_new_tab(const std::string &html_content) {
    GtkWidget *web_view = webkit_web_view_new();
    gtk_notebook_append_page(notebook, web_view, gtk_label_new("New Tab"));
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html_content.c_str(), NULL);
}

// New Tab Button Clicked
void on_new_tab_button_clicked(GtkWidget *widget, gpointer data) {
    create_new_tab("<html><body><h1>Welcome to AlphaSurf</h1><p>This is your start page.</p></body></html>");
}

// Bookmarks Button Clicked
void on_bookmarks_button_clicked(GtkWidget *widget, gpointer data) {
    display_bookmarks_ui();
}

// Settings Button Clicked
void on_settings_button_clicked(GtkWidget *widget, gpointer data) {
    display_settings_ui();
}

// Add Bookmark from Current Tab
void add_bookmark_from_current_tab() {
    std::string title = get_current_tab_title();
    std::string url = get_current_tab_url();
    if (!title.empty() && !url.empty()) {
        add_bookmark(title, url);
    }
}

// Add to History
void add_to_history(const std::string &url) {
    history.push_back(url);
}

// Fetch Page Title
void fetch_page_title(WebKitWebView *web_view, const std::string &url) {
    gchar *title = webkit_web_view_get_title(web_view);
    if (title) {
        add_bookmark(title, url);
        g_free(title);
    }
}

// Get Current Tab URL
std::string get_current_tab_url() {
    // Assume the current tab is the last one
    GtkWidget *current_web_view = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook));
    if (current_web_view) {
        gchar *url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(current_web_view));
        return url ? std::string(url) : "";
    }
    return "";
}

// Get Current Tab Title
std::string get_current_tab_title() {
    // Assume the current tab is the last one
    GtkWidget *current_web_view = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook));
    if (current_web_view) {
        gchar *title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(current_web_view));
        return title ? std::string(title) : "";
    }
    return "";
}

// Handle Alpha Scheme
void handle_alpha_scheme(const std::string &url) {
    if (url.find("alpha://settings") == 0) {
        display_settings_ui();
    } else if (url.find("alpha://history") == 0) {
        display_history_ui();
    }
}

// Display History UI
void display_history_ui() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "History",
        GTK_WINDOW(main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL);
    
    GtkWidget *list = gtk_list_box_new();
    for (const auto& url : history) {
        GtkWidget *row = gtk_label_new(url.c_str());
        gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), list, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// History Button Clicked
void on_history_button_clicked(GtkWidget *widget, gpointer data) {
    display_history_ui();
}

