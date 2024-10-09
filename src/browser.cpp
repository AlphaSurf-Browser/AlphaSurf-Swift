#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

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

// Global variables
GtkWidget *main_window;
GtkWidget *notebook;
std::vector<std::string> bookmarks;
std::map<std::string, std::string> settings;
std::map<std::string, std::vector<std::string>> history;
GtkWidget *address_bar; // Address bar for URL input
bool incognito_mode = false; // Incognito mode flag

// Function prototypes
void create_new_tab(const std::string &url);
void load_homepage(GtkWidget *web_view);
void on_tab_switch(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
void load_bookmarks();
void save_bookmarks();
void load_settings();
void save_settings();
void add_bookmark(const std::string &title, const std::string &url);
void remove_bookmark(const std::string &url);
void show_bookmarks();
void clear_history();
void add_to_history(const std::string &title, const std::string &url);
void show_history();
void open_dev_tools();
void on_new_tab_button_clicked(GtkWidget *widget);
void on_bookmarks_button_clicked(GtkWidget *widget);
void on_settings_button_clicked(GtkWidget *widget);
void on_dev_tools_button_clicked(GtkWidget *widget);
void on_history_button_clicked(GtkWidget *widget);
void on_incognito_button_clicked(GtkWidget *widget);
void initialize_ui();
void update_address_bar(const std::string &url);
std::string get_current_tab_url(); // New function to retrieve the current tab URL

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Load bookmarks and settings
    load_bookmarks();
    load_settings();

    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "AlphaSurf Browser");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Initialize the UI
    initialize_ui();

    // Show all widgets
    gtk_widget_show_all(main_window);
    gtk_main();

    // Save bookmarks and settings on exit
    save_bookmarks();
    save_settings();

    return 0;
}

// Function to create a new tab
void create_new_tab(const std::string &url) {
    GtkWidget *web_view = webkit_web_view_new();
    
    // Load the START_PAGE_HTML for the specific URL
    if (url == "alpha://start") {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), START_PAGE_HTML, nullptr);
    } else {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url.c_str());
    }

    // Update the address bar when creating a new tab
    update_address_bar(url);

    GtkWidget *label = gtk_label_new(url.c_str());
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), web_view, label);

    // Connect signal for when the page finishes loading
    g_signal_connect(web_view, "load-changed", G_CALLBACK(add_to_history), (gpointer)url.c_str());
}

// Function to load bookmarks from a JSON file
void load_bookmarks() {
    std::ifstream file("bookmarks.json");
    if (file.is_open()) {
        JsonParser *parser = json_parser_new();
        json_parser_load_from_file(parser, "bookmarks.json", NULL);
        JsonNode *root = json_parser_get_root(parser);
        JsonArray *array = json_node_get_array(root);
        for (int i = 0; i < json_array_get_length(array); i++) {
            const gchar *bookmark = json_array_get_string_element(array, i);
            bookmarks.push_back(std::string(bookmark));
        }
        g_object_unref(parser);
    }
}

// Function to save bookmarks to a JSON file
void save_bookmarks() {
    if (incognito_mode) return; // Do not save bookmarks in incognito mode

    JsonArray *array = json_array_new();
    for (const auto &bookmark : bookmarks) {
        json_array_add_string_element(array, bookmark.c_str());
    }
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);
    json_node_set_array(root, array);

    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_file(generator, "bookmarks.json", NULL);
    
    g_object_unref(generator);
    json_node_free(root);
}

// Function to load settings from a JSON file
void load_settings() {
    std::ifstream file("settings.json");
    if (file.is_open()) {
        JsonParser *parser = json_parser_new();
        json_parser_load_from_file(parser, "settings.json", NULL);
        JsonNode *root = json_parser_get_root(parser);
        JsonObject *object = json_node_get_object(root);
        
        // Load settings
        const gchar *key;
        JsonNode *value;
        GList *keys = json_object_get_keys(object);
        for (GList *iter = keys; iter != NULL; iter = iter->next) {
            key = static_cast<const gchar *>(iter->data);
            value = json_object_get_member(object, key);
            settings[key] = json_node_get_string(value);
        }
        g_object_unref(parser);
    }
}

// Function to save settings to a JSON file
void save_settings() {
    JsonObject *object = json_object_new();
    for (const auto &setting : settings) {
        json_object_set_string_member(object, setting.first.c_str(), setting.second.c_str());
    }
    JsonNode *root = json_node_new(JSON_NODE_OBJECT);
    json_node_set_object(root, object);

    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_file(generator, "settings.json", NULL);
    
    g_object_unref(generator);
    json_node_free(root);
}

// Function to add a bookmark
void add_bookmark(const std::string &title, const std::string &url) {
    if (incognito_mode) return; // Do not add bookmarks in incognito mode
    
    bookmarks.push_back(url);
    std::cout << "Bookmark added: " << title << " (" << url << ")\n";
    save_bookmarks(); // Save immediately
}

// Function to remove a bookmark
void remove_bookmark(const std::string &url) {
    if (incognito_mode) return; // Do not remove bookmarks in incognito mode
    
    bookmarks.erase(std::remove(bookmarks.begin(), bookmarks.end(), url), bookmarks.end());
    std::cout << "Bookmark removed: " << url << "\n";
    save_bookmarks(); // Save immediately
}

// Function to show bookmarks
void show_bookmarks() {
    std::cout << "Bookmarks:\n";
    for (const auto &bookmark : bookmarks) {
        std::cout << bookmark << "\n";
    }
}

// Function to clear history
void clear_history() {
    history.clear();
}

// Function to add a page to history
void add_to_history(const std::string &title, const std::string &url) {
    if (!incognito_mode) {
        history[url].push_back(title);
    }
}

// Function to show history
void show_history() {
    std::cout << "History:\n";
    for (const auto &entry : history) {
        std::cout << entry.first << "\n";
    }
}

// Function to open developer tools
void open_dev_tools() {
    std::cout << "Developer tools opened.\n";
}

// Function to handle new tab button click
void on_new_tab_button_clicked(GtkWidget *widget) {
    create_new_tab("alpha://start"); // Open the start page
}

// Function to handle bookmarks button click
void on_bookmarks_button_clicked(GtkWidget *widget) {
    show_bookmarks();
}

// Function to handle settings button click
void on_settings_button_clicked(GtkWidget *widget) {
    // Logic to show settings
    std::cout << "Settings opened.\n";
}

// Function to handle developer tools button click
void on_dev_tools_button_clicked(GtkWidget *widget) {
    open_dev_tools();
}

// Function to handle history button click
void on_history_button_clicked(GtkWidget *widget) {
    show_history();
}

// Function to handle incognito button click
void on_incognito_button_clicked(GtkWidget *widget) {
    incognito_mode = !incognito_mode; // Toggle incognito mode
    if (incognito_mode) {
        std::cout << "Incognito mode activated.\n";
    } else {
        std::cout << "Incognito mode deactivated.\n";
    }
}

// Function to initialize the UI components
void initialize_ui() {
    // Create a navigation bar
    GtkWidget *navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Create buttons for navigation
    GtkWidget *new_tab_button = gtk_button_new_with_label("New Tab");
    GtkWidget *bookmarks_button = gtk_button_new_with_label("Bookmarks");
    GtkWidget *settings_button = gtk_button_new_with_label("Settings");
    GtkWidget *dev_tools_button = gtk_button_new_with_label("Dev Tools");
    GtkWidget *history_button = gtk_button_new_with_label("History");
    GtkWidget *incognito_button = gtk_button_new_with_label("Incognito");

    // Pack buttons into the navigation bar
    gtk_box_pack_start(GTK_BOX(navbar), new_tab_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), bookmarks_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), settings_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), dev_tools_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), history_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(navbar), incognito_button, TRUE, TRUE, 0);

    // Connect button signals
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    g_signal_connect(bookmarks_button, "clicked", G_CALLBACK(on_bookmarks_button_clicked), NULL);
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    g_signal_connect(dev_tools_button, "clicked", G_CALLBACK(on_dev_tools_button_clicked), NULL);
    g_signal_connect(history_button, "clicked", G_CALLBACK(on_history_button_clicked), NULL);
    g_signal_connect(incognito_button, "clicked", G_CALLBACK(on_incognito_button_clicked), NULL);

    // Add navigation bar to the main window
    gtk_container_add(GTK_CONTAINER(main_window), navbar);

    // Create notebook for tabs
    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(main_window), notebook);

    // Create default tab
    create_new_tab("alpha://start");

    // Create an entry for the address bar
    address_bar = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(navbar), address_bar, TRUE, TRUE, 0);

    // Connect the tab switch signal
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_tab_switch), NULL);
}

// Function to show settings dialog
void show_settings_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Settings", GTK_WINDOW(main_window),
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Add settings options (e.g., homepage)
    GtkWidget *homepage_label = gtk_label_new("Homepage:");
    GtkWidget *homepage_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(homepage_entry), settings["homepage"].c_str());
    gtk_box_pack_start(GTK_BOX(content_area), homepage_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), homepage_entry, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        settings["homepage"] = gtk_entry_get_text(GTK_ENTRY(homepage_entry));
        save_settings(); // Save settings immediately
    }
    gtk_widget_destroy(dialog);
}

// Function to update the address bar
void update_address_bar(const std::string &url) {
    gtk_entry_set_text(GTK_ENTRY(address_bar), url.c_str());
}

// Function to retrieve the current tab URL
std::string get_current_tab_url() {
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    if (current_page) {
        // Assuming web_view is of type WebKitWebView
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(current_page);
        return webkit_web_view_get_uri(web_view); // Get the current URL of the web view
    }
    return "";
}

// Function called when switching tabs
void on_tab_switch(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    // Update the address bar with the URL of the current tab
    std::string current_url = get_current_tab_url();
    update_address_bar(current_url);
}
