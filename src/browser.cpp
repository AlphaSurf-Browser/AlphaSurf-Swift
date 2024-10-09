#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

// Start Page and Settings Page HTML content
const char* START_PAGE_HTML = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome to AlphaSurf</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 0;
            padding: 20px;
        }
        h1 {
            color: #007acc;
        }
        #search-button {
            padding: 10px 15px;
            background-color: #007acc;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
        #search-button:hover {
            background-color: #005f99;
        }
    </style>
</head>
<body>
    <h1>Welcome to AlphaSurf</h1>
    <script>
        function loadUrl(url) {
            window.location.href = url;
        }
    </script>
</body>
</html>
)";

const char* SETTINGS_PAGE_HTML = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Settings - AlphaSurf</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 0;
            padding: 20px;
        }
        h1 {
            color: #007acc;
        }
        label {
            display: block;
            margin: 10px 0;
        }
        input[type="text"] {
            margin: 5px 0;
        }
        input[type="submit"] {
            padding: 10px 15px;
            background-color: #007acc;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #005f99;
        }
    </style>
</head>
<body>
    <h1>Settings</h1>
    <form id="settings-form">
        <label for="homepage">Set Homepage:</label>
        <input type="text" id="homepage" placeholder="Enter URL">
        <label for="search-engine">Search Engine URL:</label>
        <input type="text" id="search-engine" placeholder="Enter search engine URL">
        <input type="submit" value="Save Settings">
    </form>
    <script>
        const form = document.getElementById('settings-form');
        form.onsubmit = function(event) {
            event.preventDefault();
            const homepage = document.getElementById('homepage').value;
            const searchEngine = document.getElementById('search-engine').value;
            window.localStorage.setItem('homepage', homepage);
            window.localStorage.setItem('searchEngine', searchEngine);
            alert('Settings saved!');
        };

        // Load existing settings
        document.getElementById('homepage').value = window.localStorage.getItem('homepage') || '';
        document.getElementById('search-engine').value = window.localStorage.getItem('searchEngine') || '';
    </script>
</body>
</html>
)";

// Function prototypes
void on_home_button_clicked(GtkWidget* widget, gpointer data);
void on_new_tab_button_clicked(GtkNotebook* notebook, gpointer data);
void on_refresh_button_clicked(GtkWidget* widget, gpointer data);
void on_settings_button_clicked(GtkWidget* widget, gpointer data);
void perform_search(GtkEntry* entry, gpointer data);
GtkWidget* create_toolbar(GtkNotebook* notebook, GtkEntry* url_entry);
void load_homepage(GtkWidget* webview);
GtkWidget* create_webview(const char* url);

// Main function
int main(int argc, char** argv) {
    gtk_init(&argc, &argv);
    
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(notebook));

    // Add a new tab
    GtkWidget* tab = create_webview("alpha://start");
    gtk_notebook_append_page(notebook, tab, gtk_label_new("Home"));

    // Create the URL entry
    GtkEntry* url_entry = GTK_ENTRY(gtk_entry_new());
    
    // Create the toolbar
    GtkWidget* toolbar = create_toolbar(notebook, url_entry);
    gtk_box_pack_start(GTK_BOX(gtk_fixed_new()), toolbar, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}

// Function to create a webview for a given URL
GtkWidget* create_webview(const char* url) {
    GtkWidget* webview = gtk_label_new("Loading...");
    // Here you would normally create a WebKitWebView
    // This is a placeholder for the WebKitGTK WebView setup
    load_homepage(webview);
    return webview;
}

void load_homepage(GtkWidget* webview) {
    // Load the homepage or the page from local storage
    const gchar* homepage = g_strdup("alpha://start"); // Default homepage

    const gchar* saved_homepage = g_getenv("HOME_PAGE"); // This simulates loading from local storage

    if (saved_homepage && strlen(saved_homepage) > 0) {
        homepage = saved_homepage; // Use saved homepage
    }

    // Simulate loading content
    gtk_label_set_text(GTK_LABEL(webview), homepage);
}

// Create toolbar for the main window
GtkWidget* create_toolbar(GtkNotebook* notebook, GtkEntry* url_entry) {
    GtkWidget* toolbar = gtk_toolbar_new();
    
    // New tab button
    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    g_signal_connect_data(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), notebook, NULL, 0);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);
    
    // Refresh button
    GtkToolItem* refresh_button = gtk_tool_button_new(NULL, "Refresh");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), refresh_button, -1);
    
    // Home button
    GtkToolItem* home_button = gtk_tool_button_new(NULL, "Home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), notebook);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);
    
    // Settings button
    GtkToolItem* settings_button = gtk_tool_button_new(NULL, "Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), settings_button, -1);
    
    // URL entry
    GtkToolItem* entry_item = gtk_tool_item_new();
    gtk_tool_item_set_expand(entry_item, TRUE);
    
    // Set the URL entry widget
    gtk_tool_item_set_widget(entry_item, GTK_WIDGET(url_entry));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entry_item, -1);
    
    // Show the entry item
    gtk_widget_show(GTK_WIDGET(entry_item));
    
    // Connect search functionality
    g_signal_connect(url_entry, "activate", G_CALLBACK(perform_search), notebook);
    
    return toolbar;
}

// Callback function implementations
void on_home_button_clicked(GtkWidget* widget, gpointer data) {
    GtkNotebook* notebook = GTK_NOTEBOOK(data);
    // Load homepage logic here
    GtkWidget* webview = create_webview("alpha://start");
    gtk_notebook_append_page(notebook, webview, gtk_label_new("Home"));
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
}

void on_new_tab_button_clicked(GtkNotebook* notebook, gpointer data) {
    GtkWidget* webview = create_webview("alpha://start");
    gtk_notebook_append_page(notebook, webview, gtk_label_new("New Tab"));
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
}

void on_refresh_button_clicked(GtkWidget* widget, gpointer data) {
    // Logic to refresh the page
    GtkNotebook* notebook = GTK_NOTEBOOK(data);
    gint current_page = gtk_notebook_get_current_page(notebook);
    // Placeholder: refresh current webview
    if (current_page != -1) {
        GtkWidget* webview = gtk_notebook_get_nth_page(notebook, current_page);
        load_homepage(webview); // Simulating a refresh
    }
}

void on_settings_button_clicked(GtkWidget* widget, gpointer data) {
    // Create a new window for settings
    GtkWidget* settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settings_window), "Settings");
    gtk_window_set_default_size(GTK_WINDOW(settings_window), 400, 300);
    
    GtkWidget* content = gtk_label_new("Settings Page (not fully implemented yet)");
    gtk_container_add(GTK_CONTAINER(settings_window), content);
    
    g_signal_connect(settings_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_widget_show_all(settings_window);
}

void perform_search(GtkEntry* entry, gpointer data) {
    const gchar* url = gtk_entry_get_text(entry);
    GtkNotebook* notebook = GTK_NOTEBOOK(data);
    GtkWidget* webview = create_webview(url);
    gtk_notebook_append_page(notebook, webview, gtk_label_new("Search Results"));
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
}
