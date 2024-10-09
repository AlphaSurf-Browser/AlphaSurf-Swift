#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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
        #navbar {
            background-color: #007BFF;
            padding: 10px;
        }
        .navbar-button {
            color: white;
            background-color: transparent;
            border: none;
            padding: 10px;
            cursor: pointer;
        }
        .navbar-button:hover {
            background-color: #0056b3;
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
GtkNotebook *notebook;
GtkEntry *address_bar;

// Bookmarks data structure
std::vector<std::string> bookmarks;

// Load settings and bookmarks from JSON file
void load_settings() {
    // Load bookmarks from a JSON file
    std::ifstream file("bookmarks.json");
    if (file.is_open()) {
        JsonParser *parser = json_parser_new();
        json_parser_load_from_file(parser, "bookmarks.json", NULL);
        JsonNode *root = json_parser_get_root(parser);
        JsonArray *array = json_node_get_array(root);

        for (int i = 0; i < json_array_get_length(array); i++) {
            bookmarks.push_back(json_array_get_string_element(array, i));
        }
        g_object_unref(parser);
    }
}

// Save bookmarks to a JSON file
void save_bookmarks() {
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);
    JsonArray *array = json_array_new();
    
    for (const auto &bookmark : bookmarks) {
        json_array_add_string_element(array, bookmark.c_str());
    }
    
    json_node_take_array(root, array);
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_file(generator, "bookmarks.json", NULL);
    g_object_unref(generator);
}

// Function to add a new bookmark
void add_bookmark(const std::string &url) {
    bookmarks.push_back(url);
    save_bookmarks();
}

// Function to show bookmarks
void show_bookmarks() {
    std::cout << "Bookmarks:\n";
    for (const auto &bookmark : bookmarks) {
        std::cout << bookmark << std::endl;
    }
}

// Function to update the address bar
void update_address_bar(const std::string &url) {
    if (GTK_IS_ENTRY(address_bar)) {
        gtk_entry_set_text(address_bar, url.c_str());
    }
}

// Function to create a new tab
void create_new_tab(const std::string &url) {
    GtkWidget *web_view = webkit_web_view_new();
    
    if (url == "alpha://start") {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), START_PAGE_HTML, nullptr);
    } else {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url.c_str());
    }

    update_address_bar(url);

    GtkWidget *label = gtk_label_new(url.c_str());
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), web_view, label);
}

// Function called when switching tabs
void on_tab_switch(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    std::string current_url = get_current_tab_url();
    update_address_bar(current_url);
}

// Retrieve the current tab URL
std::string get_current_tab_url() {
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    if (current_page) {
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(current_page);
        return webkit_web_view_get_uri(web_view);
    }
    return "";
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
    // Simple settings display
    std::cout << "Settings opened.\n";
}

// Function to handle developer tools button click
void on_dev_tools_button_clicked(GtkWidget *widget) {
    // Logic to open developer tools
    std::cout << "Developer tools opened.\n";
}

// Function to handle history button click
void on_history_button_clicked(GtkWidget *widget) {
    // Logic to show history
    std::cout << "History opened.\n";
}

// Function to handle incognito button click
void on_incognito_button_clicked(GtkWidget *widget) {
    std::cout << "Incognito mode activated.\n";
}

// Initialize the UI components
void initialize_ui() {
    // Create the main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1200, 800);
    
    // Create a vertical box for the main layout
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

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
    
    // Add address bar
    address_bar = GTK_ENTRY(gtk_entry_new());
    gtk_box_pack_start(GTK_BOX(navbar), GTK_WIDGET(address_bar), TRUE, TRUE, 0);

    // Connect button signals
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    g_signal_connect(bookmarks_button, "clicked", G_CALLBACK(on_bookmarks_button_clicked), NULL);
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    g_signal_connect(dev_tools_button, "clicked", G_CALLBACK(on_dev_tools_button_clicked), NULL);
    g_signal_connect(history_button, "clicked", G_CALLBACK(on_history_button_clicked), NULL);
    g_signal_connect(incognito_button, "clicked", G_CALLBACK(on_incognito_button_clicked), NULL);

    // Add navbar to the main box
    gtk_box_pack_start(GTK_BOX(main_box), navbar, FALSE, FALSE, 0);

    // Create notebook for tabs
    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_box_pack_start(GTK_BOX(main_box), GTK_WIDGET(notebook), TRUE, TRUE, 0);

    // Create default tab
    create_new_tab("alpha://start");

    // Connect the tab switch signal
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_tab_switch), NULL);

    // Add the main box to the window
    gtk_container_add(GTK_CONTAINER(main_window), main_box);
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
