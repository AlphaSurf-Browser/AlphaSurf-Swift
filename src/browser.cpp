#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-c/json.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>

// HTML for the start page
const char* START_PAGE_HTML = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome to AlphaSurf</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
            text-align: center;
        }
        #search {
            width: 80%;
            padding: 15px;
            font-size: 18px;
            border: 1px solid #ccc;
            border-radius: 4px;
            margin-top: 20px;
            transition: border-color 0.3s;
        }
        #search:focus {
            border-color: #007BFF;
            outline: none;
        }
        #time {
            font-size: 20px;
            margin-top: 20px;
            color: #555;
        }
    </style>
</head>
<body>
    <h1>Welcome to AlphaSurf!</h1>
    <input type="text" id="search" placeholder="Search Google..." />
    <div id="time"></div>
    <script>
        function updateTime() {
            const now = new Date();
            document.getElementById('time').textContent = "Current time: " + now.toLocaleTimeString();
        }
        setInterval(updateTime, 1000);
        updateTime();

        document.getElementById('search').addEventListener('keypress', function (e) {
            if (e.key === 'Enter') {
                const query = this.value;
                if (query) {
                    window.open('https://www.google.com/search?q=' + encodeURIComponent(query), '_self');
                }
            }
        });
    </script>
</body>
</html>
)html";

// Structure to hold tab information
struct TabInfo {
    std::string title;
    std::string url;
    GtkWidget* widget; // WebView widget
};

// Structure for settings
struct Settings {
    std::string homepage = "alpha://start"; // Default homepage via alpha://
    std::string search_engine = "https://www.google.com/search?q="; // Default search engine
};

// Global variables
std::vector<TabInfo> tabs; // Vector to hold tabs
std::map<std::string, std::string> bookmarks; // Map for bookmarks
std::vector<std::pair<std::string, std::string>> history; // Pair of URL and timestamp
GtkWidget* notebook; // Global pointer to the notebook
Settings settings; // Global settings
std::string bookmarks_file = "bookmarks.json"; // Bookmarks file path
std::string settings_file = "settings.json"; // Settings file path

// Function to load bookmarks from a JSON file
void load_bookmarks(const std::string& filename) {
    std::ifstream ifs(filename);
    if (ifs.is_open()) {
        json_object* jobj = json_object_from_file(filename.c_str());
        if (jobj) {
            json_object* bookmarks_array;
            json_object_object_get_ex(jobj, "bookmarks", &bookmarks_array);
            for (int i = 0; i < json_object_array_length(bookmarks_array); ++i) {
                json_object* bookmark = json_object_array_get_idx(bookmarks_array, i);
                std::string title = json_object_get_string(json_object_object_get(bookmark, "title"));
                std::string url = json_object_get_string(json_object_object_get(bookmark, "url"));
                bookmarks[title] = url;
            }
            json_object_put(jobj);
        }
    }
}

// Function to save bookmarks to a JSON file
void save_bookmarks(const std::string& filename) {
    json_object* jobj = json_object_new_object();
    json_object* bookmarks_array = json_object_new_array();

    for (const auto& pair : bookmarks) {
        json_object* bookmark = json_object_new_object();
        json_object_object_add(bookmark, "title", json_object_new_string(pair.first.c_str()));
        json_object_object_add(bookmark, "url", json_object_new_string(pair.second.c_str()));
        json_object_array_add(bookmarks_array, bookmark);
    }

    json_object_object_add(jobj, "bookmarks", bookmarks_array);
    json_object_to_file(filename.c_str(), jobj);
    json_object_put(jobj);
}

// Function to load settings from a JSON file
void load_settings(const std::string& filename) {
    std::ifstream ifs(filename);
    if (ifs.is_open()) {
        json_object* jobj = json_object_from_file(filename.c_str());
        if (jobj) {
            settings.homepage = json_object_get_string(json_object_object_get(jobj, "homepage"));
            settings.search_engine = json_object_get_string(json_object_object_get(jobj, "search_engine"));
            json_object_put(jobj);
        }
    }
}

// Function to save settings to a JSON file
void save_settings(const std::string& filename) {
    json_object* jobj = json_object_new_object();
    json_object_object_add(jobj, "homepage", json_object_new_string(settings.homepage.c_str()));
    json_object_object_add(jobj, "search_engine", json_object_new_string(settings.search_engine.c_str()));
    json_object_to_file(filename.c_str(), jobj);
    json_object_put(jobj);
}

// Function to create the start page with embedded HTML
GtkWidget* create_start_page(WebKitWebView* web_view) {
    webkit_web_view_load_html(web_view, START_PAGE_HTML, nullptr);
    return GTK_WIDGET(web_view);
}

// Function to create a new tab with the start page
void create_new_tab(const std::string& tabName) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Load the start page
    if (settings.homepage == "alpha://start") {
        create_start_page(web_view);
    } else {
        webkit_web_view_load_uri(web_view, settings.homepage.c_str());
    }
    
    TabInfo newTab = { tabName, settings.homepage, GTK_WIDGET(web_view) };
    tabs.push_back(newTab);

    // Append the new tab to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(web_view), gtk_label_new(tabName.c_str()));

    // Set the load changed signal for history management
    g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), nullptr);
}

// Function to handle web view navigation and add to history
void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event) {
    if (load_event == WEBKIT_LOAD_FINISHED) {
        // Get the URL and add it to history with a timestamp
        gchar* uri = webkit_web_view_get_uri(web_view);
        if (uri) {
            std::time_t now = std::time(nullptr);
            std::string timestamp = std::ctime(&now);
            history.emplace_back(uri, timestamp);
            g_free(uri);
        }
    }
}

// Function to create bookmarks manager
void show_bookmarks_manager() {
    GtkWidget* bookmarksWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(bookmarksWindow), "Bookmarks Manager");
    gtk_window_set_default_size(GTK_WINDOW(bookmarksWindow), 400, 300);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(bookmarksWindow), vbox);

    GtkWidget* bookmarksList = gtk_tree_view_new();
    GtkListStore* list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    // Populate bookmarks list
    for (const auto& pair : bookmarks) {
        GtkTreeIter iter;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, pair.first.c_str(), 1, pair.second.c_str(), -1);
    }

    gtk_tree_view_set_model(GTK_TREE_VIEW(bookmarksList), GTK_TREE_MODEL(list_store));
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(bookmarksList), -1, "Title", gtk_cell_renderer_text_new(), "text", 0, nullptr);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(bookmarksList), -1, "URL", gtk_cell_renderer_text_new(), "text", 1, nullptr);

    // Add Bookmark Button
    GtkWidget* addButton = gtk_button_new_with_label("Add Bookmark");
    g_signal_connect(addButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        // Logic to add a new bookmark
        GtkWidget* dialog = gtk_dialog_new_with_buttons("Add Bookmark", nullptr, GTK_DIALOG_MODAL,
            "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, nullptr);
        
        GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget* titleEntry = gtk_entry_new();
        GtkWidget* urlEntry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Title:"), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), titleEntry, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("URL:"), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(content_area), urlEntry, FALSE, FALSE, 0);
        gtk_widget_show_all(dialog);

        // Handle dialog response
        g_signal_connect(dialog, "response", G_CALLBACK([](GtkDialog* dialog, int response_id) {
            if (response_id == GTK_RESPONSE_OK) {
                // Add bookmark logic
                const char* title = gtk_entry_get_text(GTK_ENTRY(titleEntry));
                const char* url = gtk_entry_get_text(GTK_ENTRY(urlEntry));
                if (title && url) {
                    bookmarks[title] = url; // Add bookmark to the map
                    g_print("Added bookmark: %s - %s\n", title, url);
                    save_bookmarks(bookmarks_file); // Save bookmarks
                }
            }
            gtk_widget_destroy(GTK_WIDGET(dialog));
        }), nullptr);
        gtk_widget_show_all(dialog);
    }), nullptr);

    // Edit Bookmark Button
    GtkWidget* editButton = gtk_button_new_with_label("Edit Bookmark");
    g_signal_connect(editButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        // Logic to edit selected bookmark
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(bookmarksList));
        GtkTreeModel* model;
        GtkTreeIter iter;

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            gchar* title;
            gchar* url;
            gtk_tree_model_get(model, &iter, 0, &title, 1, &url, -1);

            GtkWidget* dialog = gtk_dialog_new_with_buttons("Edit Bookmark", nullptr, GTK_DIALOG_MODAL,
                "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, nullptr);

            GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            GtkWidget* titleEntry = gtk_entry_new();
            GtkWidget* urlEntry = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(titleEntry), title);
            gtk_entry_set_text(GTK_ENTRY(urlEntry), url);
            gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Title:"), FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(content_area), titleEntry, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("URL:"), FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(content_area), urlEntry, FALSE, FALSE, 0);
            gtk_widget_show_all(dialog);

            // Handle dialog response
            g_signal_connect(dialog, "response", G_CALLBACK([](GtkDialog* dialog, int response_id) {
                if (response_id == GTK_RESPONSE_OK) {
                    // Edit bookmark logic
                    const char* new_title = gtk_entry_get_text(GTK_ENTRY(titleEntry));
                    const char* new_url = gtk_entry_get_text(GTK_ENTRY(urlEntry));
                    bookmarks.erase(title); // Remove old bookmark
                    bookmarks[new_title] = new_url; // Add updated bookmark
                    g_print("Edited bookmark: %s - %s\n", new_title, new_url);
                    save_bookmarks(bookmarks_file); // Save bookmarks
                }
                gtk_widget_destroy(GTK_WIDGET(dialog));
            }), nullptr);
            gtk_widget_show_all(dialog);
            g_free(title);
            g_free(url);
        }
    }), nullptr);

    // Delete Bookmark Button
    GtkWidget* deleteButton = gtk_button_new_with_label("Delete Bookmark");
    g_signal_connect(deleteButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        // Logic to delete selected bookmark
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(bookmarksList));
        GtkTreeModel* model;
        GtkTreeIter iter;

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            gchar* title;
            gtk_tree_model_get(model, &iter, 0, &title, -1);
            bookmarks.erase(title); // Remove bookmark from map
            g_print("Deleted bookmark: %s\n", title);
            save_bookmarks(bookmarks_file); // Save bookmarks
            gtk_list_store_remove(GTK_LIST_STORE(list_store), &iter); // Remove from list store
            g_free(title);
        }
    }), nullptr);

    gtk_box_pack_start(GTK_BOX(vbox), bookmarksList, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), addButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), editButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), deleteButton, FALSE, FALSE, 0);

    g_signal_connect(bookmarksWindow, "destroy", G_CALLBACK(gtk_widget_destroy), nullptr);
    gtk_widget_show_all(bookmarksWindow);
}

// Function to create history window
void show_history() {
    GtkWidget* historyWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(historyWindow), "History");
    gtk_window_set_default_size(GTK_WINDOW(historyWindow), 400, 300);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(historyWindow), vbox);

    GtkWidget* historyList = gtk_text_view_new();
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(historyList));
    std::ostringstream oss;

    // Populate history
    for (const auto& entry : history) {
        oss << entry.first << " - " << entry.second << "\n";
    }
    gtk_text_buffer_set_text(buffer, oss.str().c_str(), -1);

    // Clear History Button
    GtkWidget* clearButton = gtk_button_new_with_label("Clear History");
    g_signal_connect(clearButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        // Logic to clear history
        history.clear(); // Clear history vector
        g_print("History cleared\n");
    }), nullptr);

    gtk_box_pack_start(GTK_BOX(vbox), historyList, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), clearButton, FALSE, FALSE, 0);

    g_signal_connect(historyWindow, "destroy", G_CALLBACK(gtk_widget_destroy), nullptr);
    gtk_widget_show_all(historyWindow);
}

// Function to create settings window
void show_settings() {
    GtkWidget* settingsWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settingsWindow), "Settings");
    gtk_window_set_default_size(GTK_WINDOW(settingsWindow), 300, 200);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(settingsWindow), vbox);

    GtkWidget* homepageEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(homepageEntry), settings.homepage.c_str());

    GtkWidget* searchEngineEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(searchEngineEntry), settings.search_engine.c_str());

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Homepage URL:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), homepageEntry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Search Engine:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), searchEngineEntry, FALSE, FALSE, 0);

    // Save Settings Button
    GtkWidget* saveButton = gtk_button_new_with_label("Save Settings");
    g_signal_connect(saveButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        // Save settings to JSON
        const char* new_homepage = gtk_entry_get_text(GTK_ENTRY(homepageEntry));
        const char* new_search_engine = gtk_entry_get_text(GTK_ENTRY(searchEngineEntry));
        settings.homepage = new_homepage;
        settings.search_engine = new_search_engine;

        save_settings(settings_file); // Save to file
        g_print("Settings saved\n");
    }), nullptr);

    gtk_box_pack_start(GTK_BOX(vbox), saveButton, FALSE, FALSE, 0);

    g_signal_connect(settingsWindow, "destroy", G_CALLBACK(gtk_widget_destroy), nullptr);
    gtk_widget_show_all(settingsWindow);
}

// Function to create the navigation bar
GtkWidget* create_navigation_bar() {
    GtkWidget* toolbar = gtk_toolbar_new();

    // New Tab Button
    GtkToolItem* newTabButton = gtk_tool_button_new(nullptr, "New Tab");
    g_signal_connect(newTabButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        create_new_tab("New Tab"); // Ensure create_new_tab is defined correctly
    }), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), newTabButton, -1);

    // Bookmarks Button
    GtkToolItem* bookmarksButton = gtk_tool_button_new(nullptr, "Bookmarks");
    g_signal_connect(bookmarksButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        show_bookmarks_manager(); // Ensure this function is defined and takes no parameters
    }), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bookmarksButton, -1);

    // History Button
    GtkToolItem* historyButton = gtk_tool_button_new(nullptr, "History");
    g_signal_connect(historyButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        show_history(); // Ensure this function is defined correctly
    }), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), historyButton, -1);

    // Settings Button
    GtkToolItem* settingsButton = gtk_tool_button_new(nullptr, "Settings");
    g_signal_connect(settingsButton, "clicked", G_CALLBACK([](GtkWidget* widget) {
        show_settings(); // Ensure this function is defined
    }), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), settingsButton, -1);

    return toolbar;
}


// Main function
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    
    // Load bookmarks and settings
    load_bookmarks(bookmarks_file);
    load_settings(settings_file);

    // Create main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    // Create notebook for tabs
    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);
    
    // Create and pack the navigation bar
    GtkWidget* navBar = create_navigation_bar();
    gtk_box_pack_start(GTK_BOX(gtk_vbox_new(FALSE, 0)), navBar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gtk_vbox_new(FALSE, 0)), notebook, TRUE, TRUE, 0);

    // Connect the destroy event to exit the application
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Open a new tab with the start page
    create_new_tab("Start Page");

    // Show everything
    gtk_widget_show_all(window);
    gtk_main();

    // Save bookmarks on exit
    save_bookmarks(bookmarks_file);
    save_settings(settings_file);
    return 0;
}
