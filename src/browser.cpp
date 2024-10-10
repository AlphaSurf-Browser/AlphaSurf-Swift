#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>

// Constants for the start page HTML
const char* START_PAGE_HTML = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome to AlphaSurf</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: white;
        }
        .container {
            text-align: center;
            background: rgba(255, 255, 255, 0.1);
            padding: 40px;
            border-radius: 20px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
        }
        h1 {
            font-size: 48px;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
        }
        #search {
            width: 100%;
            padding: 15px;
            font-size: 18px;
            border: none;
            border-radius: 30px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            transition: all 0.3s ease;
        }
        #search::placeholder {
            color: rgba(255, 255, 255, 0.7);
        }
        #search:focus {
            outline: none;
            box-shadow: 0 0 15px rgba(255, 255, 255, 0.5);
            background: rgba(255, 255, 255, 0.3);
        }
        #time {
            font-size: 24px;
            margin-top: 20px;
            font-weight: 300;
        }
        #bookmarks {
            margin-top: 30px;
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
        }
        .bookmark {
            background: rgba(255, 255, 255, 0.2);
            border-radius: 15px;
            padding: 10px 20px;
            margin: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .bookmark:hover {
            background: rgba(255, 255, 255, 0.3);
            transform: translateY(-3px);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>AlphaSurf</h1>
        <input type="text" id="search" placeholder="Search or enter URL" />
        <div id="time"></div>
        <div id="bookmarks"></div>
    </div>
    <script>
        function updateTime() {
            const now = new Date();
            document.getElementById('time').textContent = now.toLocaleTimeString();
        }
        setInterval(updateTime, 1000);
        updateTime();

        document.getElementById('search').addEventListener('keypress', function (e) {
            if (e.key === 'Enter') {
                const query = this.value;
                if (query) {
                    if (query.startsWith('http://') || query.startsWith('https://')) {
                        window.location.href = query;
                    } else {
                        window.location.href = 'https://www.google.com/search?q=' + encodeURIComponent(query);
                    }
                }
            }
        });

        // Function to add a bookmark to the start page
        function addBookmark(url, title) {
            const bookmarksDiv = document.getElementById('bookmarks');
            const bookmarkElem = document.createElement('div');
            bookmarkElem.className = 'bookmark';
            bookmarkElem.textContent = title || url;
            bookmarkElem.onclick = () => window.location.href = url;
            bookmarksDiv.appendChild(bookmarkElem);
        }
    </script>
</body>
</html>
)html";

// Global variables
GtkWidget *main_window;
GtkNotebook *notebook;
GtkEntry *address_bar;
std::vector<std::string> bookmarks;
std::unordered_set<std::string> ad_block_list = {"example.com", "ads.com"};

// Function prototypes
void load_settings();
void save_bookmarks();
void add_bookmark(const std::string &url, const std::string &title);
void show_bookmarks();
void update_address_bar(const std::string &url);
std::string get_current_tab_url();
void create_new_tab(const std::string &url);
void on_tab_switch(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
void on_new_tab_button_clicked(GtkWidget *widget);
void on_bookmarks_button_clicked(GtkWidget *widget);
void on_settings_button_clicked(GtkWidget *widget);
void initialize_ui();
void show_splash_screen();
void add_bookmark_from_current_page();

// Load settings and bookmarks from JSON file
void load_settings() {
    std::ifstream file("bookmarks.json");
    if (!file.is_open()) {
        // Create bookmarks.json if it doesn't exist
        std::ofstream new_file("bookmarks.json");
        new_file.close();
        return;
    }
    
    JsonParser *parser = json_parser_new();
    json_parser_load_from_file(parser, "bookmarks.json", NULL);
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);

    for (int i = 0; i < json_array_get_length(array); i++) {
        JsonObject *obj = json_array_get_object_element(array, i);
        std::string url = json_object_get_string_member(obj, "url");
        std::string title = json_object_get_string_member(obj, "title");
        bookmarks.push_back(url);
        // Call JavaScript function to add bookmark to start page
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(gtk_notebook_get_nth_page(notebook, 0));
        gchar *js = g_strdup_printf("addBookmark('%s', '%s');", url.c_str(), title.c_str());
        webkit_web_view_run_javascript(web_view, js, NULL, NULL, NULL);
        g_free(js);
    }
    g_object_unref(parser);
}

// Save bookmarks to a JSON file
void save_bookmarks() {
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);
    JsonArray *array = json_array_new();
    
    for (const auto &bookmark : bookmarks) {
        JsonObject *obj = json_object_new();
        json_object_set_string_member(obj, "url", bookmark.c_str());
        json_object_set_string_member(obj, "title", bookmark.c_str());
        json_array_add_object_element(array, obj);
    }
    
    json_node_take_array(root, array);
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_file(generator, "bookmarks.json", NULL);
    g_object_unref(generator);
}

// Function to add a new bookmark
void add_bookmark(const std::string &url, const std::string &title) {
    bookmarks.push_back(url);
    save_bookmarks();
    
    // Add bookmark to start page
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(gtk_notebook_get_nth_page(notebook, 0));
    gchar *js = g_strdup_printf("addBookmark('%s', '%s');", url.c_str(), title.c_str());
    webkit_web_view_run_javascript(web_view, js, NULL, NULL, NULL);
    g_free(js);
}

// Function to add a bookmark from the current page
void add_bookmark_from_current_page() {
    std::string current_url = get_current_tab_url();
    if (!current_url.empty()) {
        // Use the current page's title for the bookmark
        std::string title = "Bookmark: " + current_url; // Placeholder for title
        add_bookmark(current_url, title);
    }
}

// Function to show bookmarks
void show_bookmarks() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Bookmarks",
                                                    GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "Close",
                                                    GTK_RESPONSE_CLOSE,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(content_area), list_box);

    for (const auto &bookmark : bookmarks) {
        GtkWidget *label = gtk_label_new(bookmark.c_str());
        gtk_list_box_insert(GTK_LIST_BOX(list_box), label, -1);
    }

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to update the address bar
void update_address_bar(const std::string &url) {
    gtk_entry_set_text(address_bar, url.c_str());
}

// Function to get the current tab's URL
std::string get_current_tab_url() {
    Widget *current_tab = gtk_notebook_get_current_page(notebook);
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(current_tab);
    return webkit_web_view_get_uri(web_view);
}

// Function to create a new tab
void create_new_tab(const std::string &url) {
    GtkWidget *web_view = webkit_web_view_new();
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url.c_str());
    gtk_notebook_append_page(notebook, web_view, gtk_label_new("New Tab"));
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
    g_signal_connect(web_view, "load-changed", G_CALLBACK(update_address_bar), NULL);
}

// Function for the New Tab button
void on_new_tab_button_clicked(GtkWidget *widget) {
    create_new_tab("alpha://start");
}

// Function for the Bookmarks button
void on_bookmarks_button_clicked(GtkWidget *widget) {
    show_bookmarks();
}

// Function for the Settings button
void on_settings_button_clicked(GtkWidget *widget) {
    create_new_tab("alpha://settings");
}

// Function to initialize the UI
void initialize_ui() {
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    gtk_window_set_icon_from_file(GTK_WINDOW(main_window), "icon.png", NULL);
    
    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(notebook));

    // Address Bar
    address_bar = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(address_bar, "Enter URL or Search");
    g_signal_connect(address_bar, "activate", G_CALLBACK(on_new_tab_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(gtk_header_bar_new()), GTK_WIDGET(address_bar), TRUE, TRUE, 0);

    // Toolbar
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_tool_button_new_from_stock(GTK_STOCK_NEW), -1);
    g_signal_connect(gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), 0), "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_tool_button_new_from_stock(GTK_STOCK_BOOKMARK), -1);
    g_signal_connect(gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), 1), "clicked", G_CALLBACK(on_bookmarks_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES), -1);
    g_signal_connect(gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), 2), "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(toolbar), FALSE, FALSE, 0);

    // Add the first tab (start page)
    create_new_tab("alpha://start");

    // Keyboard Shortcuts
    g_signal_connect(main_window, "key-press-event", G_CALLBACK([](GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
        if (event->keyval == GDK_KEY_d && (event->state & GDK_CONTROL_MASK)) {
            add_bookmark_from_current_page();
            return TRUE;
        } else if (event->keyval == GDK_KEY_t && (event->state & GDK_CONTROL_MASK)) {
            create_new_tab("alpha://start");
            return TRUE;
        }
        return FALSE;
    }), NULL);

    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(main_window);
}

// Function to display the splash screen
void show_splash_screen() {
    GtkWidget *splash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(splash_window), "Welcome to AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(splash_window), 400, 300);
    gtk_window_set_resizable(GTK_WINDOW(splash_window), FALSE);
    
    GtkWidget *label = gtk_label_new("Welcome to AlphaSurf!\nEnjoy your browsing experience.");
    gtk_container_add(GTK_CONTAINER(splash_window), label);

    gtk_widget_show_all(splash_window);
    g_timeout_add_seconds(3, (GSourceFunc)gtk_widget_destroy, splash_window);
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    load_settings();
    show_splash_screen();
    initialize_ui();
    gtk_main();
    return 0;
}
