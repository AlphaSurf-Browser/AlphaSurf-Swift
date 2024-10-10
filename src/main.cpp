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

void load_bookmarks() {
    try {
        std::ifstream file("bookmarks.json");
        if (file) {
            JsonParser *parser = json_parser_new();
            json_parser_load_from_file(parser, "bookmarks.json", NULL);
            JsonNode *root = json_parser_get_root(parser);
            if (root) {
                JsonArray *array = json_node_get_array(root);
                for (guint i = 0; i < json_array_get_length(array); i++) {
                    JsonNode *node = json_array_get_element(array, i);
                    JsonObject *object = json_node_get_object(node);
                    Bookmark bookmark;
                    bookmark.title = json_object_get_string_member(object, "title");
                    bookmark.url = json_object_get_string_member(object, "url");
                    bookmarks.push_back(bookmark);
                }
            }
            g_object_unref(parser);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading bookmarks: " << e.what() << std::endl;
    }
}

void save_bookmarks() {
    try {
        JsonGenerator *generator = json_generator_new();
        JsonNode *root = json_node_alloc();
        json_node_init_array(root);
        for (const Bookmark &bookmark : bookmarks) {
            JsonNode *node = json_node_alloc();
            json_node_init_object(node);
            json_object_set_string_member(json_node_get_object(node), "title", bookmark.title.c_str());
            json_object_set_string_member(json_node_get_object(node), "url", bookmark.url.c_str());
            json_array_add_element(json_node_get_array(root), node);
        }
        std::ofstream file("bookmarks.json");
        json_generator_set_root(generator, root);
        json_generator_to_file(generator, "bookmarks.json", NULL);
        g_object_unref(generator);
        g_object_unref(root);
    } catch (const std::exception& e) {
        std::cerr << "Error saving bookmarks: " << e.what() << std::endl;
    }
}

void add_bookmark(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Bookmark", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_container_add (GTK_CONTAINER(content), grid);
    GtkWidget *title_label = gtk_label_new("Title:");
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 1, 1);
    GtkWidget *title_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), title_entry, 1, 0, 1, 1);
    GtkWidget *url_label = gtk_label_new("URL:");
    gtk_grid_attach(GTK_GRID(grid), url_label, 0, 1, 1, 1);
    GtkWidget *url_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), url_entry, 1, 1, 1, 1);
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        Bookmark bookmark;
        bookmark.title = gtk_entry_get_text(GTK_ENTRY(title_entry));
        bookmark.url = gtk_entry_get_text(GTK_ENTRY(url_entry));
        bookmarks.push_back(bookmark);
        save_bookmarks();
    }
    gtk_widget_destroy(dialog);
    g_object_unref(dialog);
}

void open_bookmark(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Open Bookmark", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *list = gtk_list_box_new();
    gtk_grid_attach(GTK_GRID(grid), list, 0, 0, 1, 1);
    for (const Bookmark &bookmark : bookmarks) {
        GtkWidget *row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), gtk_label_new(bookmark.title.c_str()));
        gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
    }
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *selected_row = gtk_list_box_get_selected_row(GTK_LIST_BOX(list));
        if (selected_row) {
            GtkWidget *label = gtk_bin_get_child(GTK_BIN(selected_row));
            std::string title = gtk_label_get_text(GTK_LABEL(label));
            for (const Bookmark &bookmark : bookmarks) {
                if (bookmark.title == title) {
                    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(notebook), bookmark.url.c_str());
                    break;
                }
            }
        }
    }
    gtk_widget_destroy(dialog);
    g_object_unref(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Alpha Browser");
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    GtkWidget *add_button = gtk_tool_button_new(gtk_image_new_from_icon_name("list-add", GTK_ICON_SIZE_LARGE_TOOLBAR), "Add Bookmark");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), add_button, -1);
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_bookmark), NULL);
    GtkWidget *open_button = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_LARGE_TOOLBAR), "Open Bookmark");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open_button, -1);
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_bookmark), NULL);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(notebook));
    GtkWidget *web_view = webkit_web_view_new    ();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), web_view, gtk_label_new("New Tab"));
    load_bookmarks();
    gtk_widget_show_all(main_window);
    gtk_main();
    return 0;
}
