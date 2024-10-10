#include "ui.h"

// Function declarations
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

void display_settings_ui() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Settings",
        GTK_WINDOW(main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Settings will be implemented here.");
    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_splash_screen() {
    GtkWidget *splash = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(splash), "Welcome to AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(splash), 400, 200);

    GtkWidget *label = gtk_label_new("Loading AlphaSurf...");
    gtk_container_add(GTK_CONTAINER(splash), label);
    gtk_widget_show_all(splash);

    g_timeout_add(2000, (GSourceFunc)gtk_widget_destroy, splash); // Auto close after 2 seconds
}

void create_new_tab(const std::string &html_content) {
    GtkWidget *tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

    g_signal_connect(web_view, "load-changed", G_CALLBACK([](WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data) {
        if (load_event == WEBKIT_LOAD_FINISHED) {
            std::string url = webkit_web_view_get_uri(web_view);
            add_to_history(url);
            fetch_page_title(web_view, url);
        }
    }), NULL);

    gtk_box_pack_start(GTK_BOX(tab), GTK_WIDGET(web_view), TRUE, TRUE, 0);
    gtk_notebook_append_page(notebook, tab, gtk_label_new("New Tab"));

    // Check for alpha:// URLs
    if (html_content.find("alpha://") == 0) {
        handle_alpha_scheme(html_content);
    } else {
        webkit_web_view_load_uri(web_view, html_content.c_str());
    }
}

void on_new_tab_button_clicked(GtkWidget *widget, gpointer data) {
    create_new_tab("alpha://newtab"); // Use the custom schema for the start page
}

void on_bookmarks_button_clicked(GtkWidget *widget, gpointer data) {
    display_bookmarks_ui();
}

void on_settings_button_clicked(GtkWidget *widget, gpointer data) {
    display_settings_ui();
}

void on_history_button_clicked(GtkWidget *widget, gpointer data) {
    display_history_ui();
}

void handle_alpha_scheme(const std::string &url) {
    if (url == "alpha://newtab") {
        std::string start_page_html = R"(
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Start Page</title>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        margin: 0;
                        padding: 0;
                        background-color: #f0f0f0;
                    }
                    .container {
                        text-align: center;
                        padding: 50px;
                    }
                    h1 {
                        color: # 333;
                    }
                    #search {
                        width: 50%;
                        padding: 10px;
                        font-size: 18px;
                        border: 1px solid #ccc;
                        border-radius: 4px;
                    }
                    #footer {
                        position: fixed;
                        bottom: 10px;
                        width: 100%;
                        text-align: center;
                        color: #aaa;
                    }
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>Welcome to AlphaSurf</h1>
                    <input type="text" id="search" placeholder="Search the web...">
                </div>
                <div id="footer">Made with ♥ by WolfTech Innovations</div>
                <script>
                    document.getElementById('search').addEventListener('keypress', function(event) {
                        if (event.key === 'Enter') {
                            var query = event.target.value;
                            window.open('https://www.google.com/search?q=' + query, '_self');
                        }
                    });
                </script>
            </body>
            </html>
        )";
        create_new_tab(start_page_html);
    } else {
        // Handle other alpha:// URLs here if needed
    }
}
