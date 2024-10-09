#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

// HTML Content for the Start Page and Settings Page
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
    <p>Use the address bar to navigate or perform searches.</p>
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

// Function Prototypes
void on_uri_requested(WebKitWebView* web_view, const gchar* uri);
void on_new_tab_button_clicked(GtkNotebook* notebook, GtkEntry* url_entry);
void on_refresh_button_clicked(GtkWidget* widget, gpointer data);
void on_home_button_clicked(GtkWidget* widget, gpointer data);
void on_settings_button_clicked(GtkWidget* widget, gpointer data);
void perform_search(GtkEntry* entry, gpointer data);
void open_settings_window();

// Initialize WebView
WebKitWebView* create_web_view(const gchar* uri) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    g_signal_connect(web_view, "decide-policy", G_CALLBACK(on_uri_requested), NULL);
    webkit_web_view_load_uri(web_view, uri);
    return web_view;
}

// Function for handling URI requests
void on_uri_requested(WebKitWebView* web_view, const gchar* uri) {
    // Check if the URI uses the alpha:// scheme
    if (g_str_has_prefix(uri, "alpha://")) {
        // Handle custom alpha:// scheme URIs
        g_print("Navigating to custom URI: %s\n", uri);
        if (g_strcmp0(uri, "alpha://start") == 0) {
            webkit_web_view_load_html(web_view, START_PAGE_HTML, NULL);
            return;
        } else if (g_strcmp0(uri, "alpha://settings") == 0) {
            webkit_web_view_load_html(web_view, SETTINGS_PAGE_HTML, NULL);
            return;
        }
    }
    // Otherwise, load the URI normally
    webkit_web_view_load_uri(web_view, uri);
}

// Function to create a new tab
void on_new_tab_button_clicked(GtkNotebook* notebook, GtkEntry* url_entry) {
    const gchar* url = gtk_entry_get_text(url_entry);
    if (g_strcmp0(url, "") == 0) {
        url = "alpha://start"; // Default start page
    }
    
    WebKitWebView* new_web_view = create_web_view(url);
    GtkWidget* tab_label = gtk_label_new(url);
    
    // Add new tab
    gtk_notebook_append_page(notebook, GTK_WIDGET(new_web_view), tab_label);
    gtk_widget_show(GTK_WIDGET(new_web_view));
    
    // Set URL entry to listen for load changes
    g_signal_connect(new_web_view, "load-changed", G_CALLBACK(on_uri_requested), url_entry);
}

// Function for refreshing the current page
void on_refresh_button_clicked(GtkWidget* widget, gpointer data) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    webkit_web_view_reload(web_view);
}

// Function for going to home page
void on_home_button_clicked(GtkWidget* widget, gpointer data) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    const gchar* homepage = g_strdup(window.localStorage.getItem('homepage') || 'alpha://start');
    webkit_web_view_load_uri(web_view, homepage);
}

// Function for opening settings
void on_settings_button_clicked(GtkWidget* widget, gpointer data) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    webkit_web_view_load_html(web_view, SETTINGS_PAGE_HTML, NULL);
}

// Perform search on entry activate
void perform_search(GtkEntry* entry, gpointer data) {
    const gchar* url = gtk_entry_get_text(entry);
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    if (g_str_has_prefix(url, "http://") || g_str_has_prefix(url, "https://") || g_str_has_prefix(url, "alpha://")) {
        webkit_web_view_load_uri(web_view, url);
    } else {
        gchar* search_uri = g_strdup_printf("https://www.startpage.com/search?q=%s", url);
        webkit_web_view_load_uri(web_view, search_uri);
        g_free(search_uri);
    }
}

// Create toolbar
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
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);
    
    // Settings button
    GtkToolItem* settings_button = gtk_tool_button_new(NULL, "Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), settings_button, -1);
    
    // URL Entry
    GtkToolItem* entry_item = gtk_tool_item_new();
    gtk_tool_item_set_expand(entry_item, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entry_item, -1);
    
    // Create URL entry field
    gtk_tool_item_set_homogeneous(entry_item, TRUE);
    gtk_widget_show(entry_item);
    
    // Connect search functionality
    g_signal_connect(url_entry, "activate", G_CALLBACK(perform_search), NULL);
    
    return toolbar;
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *url_entry = gtk_entry_new();

    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), TRUE);
    
    // Create initial tab
    on_new_tab_button_clicked(GTK_NOTEBOOK(notebook), url_entry);

    // Create toolbar
    GtkWidget* toolbar = create_toolbar(GTK_NOTEBOOK(notebook), url_entry);
    
    // Add widgets to the main window
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}
