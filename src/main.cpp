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

        // Function to load bookmarks (to be implemented)
        function loadBookmarks() {
            // This function will be called from C++ to populate bookmarks
        }

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
std::vector<std::string> history;
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
void on_dev_tools_button_clicked(GtkWidget *widget);
void on_history_button_clicked(GtkWidget *widget);
void on_incognito_button_clicked(GtkWidget *widget);
void initialize_ui();

// Load settings and bookmarks from JSON file
void load_settings() {
    std::ifstream file("bookmarks.json");
    if (file.is_open()) {
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
}

// Save bookmarks to a JSON file
void save_bookmarks() {
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);
    JsonArray *array = json_array_new();
    
    for (const auto &bookmark : bookmarks) {
        JsonObject *obj = json_object_new();
        json_object_set_string_member(obj, "url", bookmark.c_str());
        json_object_set_string_member(obj, "title", bookmark.c_str()); // Using URL as title for simplicity
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
    if (GTK_IS_ENTRY(address_bar)) {
        gtk_entry_set_text(address_bar, url.c_str());
    }
}

// Function to retrieve the current tab URL
std::string get_current_tab_url() {
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    if (current_page) {
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(current_page);
        return webkit_web_view_get_uri(web_view);
    }
    return "";
}

// Function to create a new tab
void create_new_tab(const std::string &url) {
    GtkWidget *web_view = webkit_web_view_new();
    
    if (url == "alpha://start") {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), START_PAGE_HTML, nullptr);
    } else {
        // Check for ad blocking
        for (const auto &ad_domain : ad_block_list) {
            if (url.find(ad_domain) != std::string::npos) {
                std::cout << "Blocked ad URL: " << url << std::endl;
                return; // Don't load the ad URL
            }
        }
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url.c_str());
        history.push_back(url); // Add to history
    }

    update_address_bar(url);

    GtkWidget *label = gtk_label_new("New Tab");
    int page_num = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), web_view, label);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page_num);
    gtk_widget_show_all(GTK_WIDGET(notebook));
}

// Function called when switching tabs
void on_tab_switch(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    std::string current_url = get_current_tab_url();
    update_address_bar(current_url);
}

// Function to handle new tab button click
void on_new_tab_button_clicked(GtkWidget *widget) {
    create_new_tab("alpha://start");
}

// Function to handle bookmarks button click
void on_bookmarks_button_clicked(GtkWidget *widget) {
    show_bookmarks();
}

// Function to handle settings button click
void on_settings_button_clicked(GtkWidget *widget) {
    std::cout << "Settings opened.\n";
}

// Function to handle developer tools button click
void on_dev_tools_button_clicked(GtkWidget *widget) {
    std::cout << "Developer tools opened.\n";
}

// Function to handle history button click
void on_history_button_clicked(GtkWidget *widget) {
    std::cout << "History:\n";
    for (const auto &url : history) {
        std::cout << url << std::endl;
    }
}

// Function to handle incognito button click
void on_incognito_button_clicked(GtkWidget *widget) {
    std::cout << "Incognito mode activated.\n";
}

// Function to handle address bar activation
void on_address_bar_activate(GtkEntry *entry, gpointer user_data) {
    const gchar *url = gtk_entry_get_text(entry);
    if (url && *url) {
        std::string processed_url = url;
        if (processed_url.find("://") == std::string::npos) {
            processed_url = "https://" + processed_url;
        }
        create_new_tab(processed_url);
    }
}

// Initialize the UI components
void initialize_ui() {
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *new_tab_button = gtk_button_new_with_label("New Tab");
    GtkWidget *bookmarks_button = gtk_button_new_with_label("Bookmarks");
    GtkWidget *settings_button = gtk_button_new_with_label("Settings");
    GtkWidget *dev_tools_button = gtk_button_new_with_label("Dev Tools");
    GtkWidget *history_button = gtk_button_new_with_label("History");
    GtkWidget *incognito_button = gtk_button_new_with_label("Incognito");

    gtk_box_pack_start(GTK_BOX(navbar), new_tab_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), bookmarks_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), settings_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), dev_tools_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), history_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), incognito_button, FALSE, FALSE, 0);

    address_bar = GTK_ENTRY(gtk_entry_new());
    gtk_box_pack_start(GTK_BOX(navbar), GTK_WIDGET(address_bar), TRUE, TRUE, 0);

    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    g_signal_connect(bookmarks_button, "clicked", G_CALLBACK(on_bookmarks_button_clicked), NULL);
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    g_signal_connect(dev_tools_button, "clicked", G_CALLBACK(on_dev_tools_button_clicked), NULL);
    g_signal_connect(history_button, "clicked", G_CALLBACK(on_history_button_clicked), NULL);
    g_signal_connect(incognito_button, "clicked", G_CALLBACK(on_incognito_button_clicked), NULL);
    g_signal_connect(address_bar, "activate", G_CALLBACK(on_address_bar_activate), NULL);

    gtk_box_pack_start(GTK_BOX(main_box), navbar, FALSE, FALSE, 0);

    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_box_pack_start(GTK_BOX(main_box), GTK_WIDGET(notebook), TRUE, TRUE, 0);

    create_new_tab("alpha://start");

    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_tab_switch), NULL);

    gtk_container_add(GTK_CONTAINER(main_window), main_box);
}

// Function to handle page load finished
void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data) {
    if (load_event == WEBKIT_LOAD_FINISHED) {
        const gchar *url = webkit_web_view_get_uri(web_view);
        update_address_bar(url);
        
        // Update tab label with page title
        GtkWidget *tab_label = gtk_notebook_get_tab_label(notebook, GTK_WIDGET(web_view));
        if (GTK_IS_LABEL(tab_label)) {
            const gchar *title = webkit_web_view_get_title(web_view);
            gtk_label_set_text(GTK_LABEL(tab_label), title ? title : "Untitled");
        }
    }
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    load_settings();
    initialize_ui();

    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(main_window);

    gtk_main();
    return 0;
}
