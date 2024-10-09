#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

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

struct Tab {
    WebKitWebView* web_view;
    GtkWidget* label;
};

std::vector<Tab> tabs;

std::string load_search_engine() {
    std::ifstream infile("settings.txt");
    std::string search_engine;
    if (infile.good()) {
        std::getline(infile, search_engine);
    }
    return search_engine.empty() ? "https://duckduckgo.com/?q=%s" : search_engine; // Default to DuckDuckGo
}

void save_settings(const std::string& homepage, const std::string& search_engine) {
    std::ofstream outfile("settings.txt");
    if (outfile.is_open()) {
        outfile << homepage << "\n" << search_engine << std::endl;
    }
}

void load_url(WebKitWebView* web_view, const gchar* url) {
    webkit_web_view_load_uri(web_view, url);
}

void load_html(WebKitWebView* web_view, const char* html) {
    webkit_web_view_load_html(web_view, html, nullptr);
}

void open_alpha_start(WebKitWebView* web_view) {
    load_html(web_view, START_PAGE_HTML);
}

void on_refresh_button_clicked(WebKitWebView* web_view) {
    const gchar* uri = webkit_web_view_get_uri(web_view);
    if (uri) {
        webkit_web_view_reload(web_view);
    } else {
        open_alpha_start(web_view);
    }
}

void on_home_button_clicked(WebKitWebView* web_view) {
    open_alpha_start(web_view);
}

void on_settings_button_clicked(GtkNotebook* notebook) {
    WebKitWebView* settings_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    load_html(settings_web_view, SETTINGS_PAGE_HTML);
    
    GtkWidget* settings_tab_label = gtk_label_new("Settings");
    gtk_notebook_append_page(notebook, GTK_WIDGET(settings_web_view), settings_tab_label);
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
}

void on_new_tab_button_clicked(GtkNotebook* notebook) {
    WebKitWebView* new_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    open_alpha_start(new_web_view);  // Load start page in new tab

    GtkWidget* tab_label = gtk_label_new("New Tab");
    gtk_notebook_append_page(notebook, GTK_WIDGET(new_web_view), tab_label);
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
    
    // Connect the "destroy" signal to clean up the web view
    g_signal_connect(new_web_view, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}

void perform_search(WebKitWebView* web_view, const gchar* query) {
    std::string search_engine = load_search_engine();
    std::string search_url = search_engine;
    size_t pos = search_url.find("%s");
    if (pos != std::string::npos) {
        search_url.replace(pos, 2, query);
    }
    load_url(web_view, search_url.c_str());
}

void on_uri_requested(WebKitWebView* web_view, const gchar* uri) {
    if (g_str_has_prefix(uri, "alpha://")) {
        if (g_strcmp0(uri, "alpha://start") == 0) {
            open_alpha_start(web_view);
        } else if (g_strcmp0(uri, "alpha://settings") == 0) {
            on_settings_button_clicked(GTK_NOTEBOOK(gtk_widget_get_parent(GTK_WIDGET(web_view))));
        } else if (g_strcmp0(uri, "alpha://search") == 0) {
            const gchar* query = g_strrstr(uri, "q=");
            if (query) {
                query += 2;  // Skip "q="
                perform_search(web_view, query);
            }
        } else {
            load_url(web_view, uri); // For any other alpha:// URL, treat it as a regular web request
        }
    } else {
        load_url(web_view, uri); // Treat as a standard URL for non-alpha schemas
    }
}

void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, GtkEntry* url_entry) {
    if (load_event == WEBKIT_LOAD_FINISHED) {
        const gchar* uri = webkit_web_view_get_uri(web_view);
        gtk_entry_set_text(url_entry, uri);
    }
}

// Function prototype for the toolbar creation
GtkWidget* create_toolbar(GtkNotebook* notebook, GtkEntry* url_entry) {
    GtkWidget* toolbar = gtk_toolbar_new();

    GtkToolItem* refresh_button = gtk_tool_button_new(NULL, "Refresh");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), refresh_button, -1);

    GtkToolItem* home_button = gtk_tool_button_new(NULL, "Home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);

    GtkToolItem* settings_button = gtk_tool_button_new(NULL, "Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), notebook);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), settings_button, -1);

    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), notebook);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);

    // URL Entry
    GtkToolItem* entry_item = gtk_tool_item_new();
    gtk_tool_item_set_expand(entry_item, TRUE);
    gtk_tool_item_set_homogeneous(entry_item, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entry_item, -1);

    // Add the URL entry to the tool item
    gtk_container_add(GTK_CONTAINER(entry_item), url_entry);

    // Connect the entry's activate signal
    g_signal_connect(url_entry, "activate", G_CALLBACK(perform_search), nullptr);

    return toolbar;
}


int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // Create URL entry and toolbar
    GtkWidget* url_entry = gtk_entry_new();
    GtkWidget* toolbar = create_toolbar(GTK_NOTEBOOK(notebook), GTK_ENTRY(url_entry));

    GtkWidget* header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "AlphaSurf");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), toolbar);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

    // Initial tab
    on_new_tab_button_clicked(GTK_NOTEBOOK(notebook));

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
