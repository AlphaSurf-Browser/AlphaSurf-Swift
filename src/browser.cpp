#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <vector>
#include <string>

const char* START_PAGE_HTML = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Alpha Start Page</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f3f4f6;
            color: #333;
        }
        h1 {
            color: #4a90e2;
        }
        #search {
            margin-top: 20px;
        }
        input[type="text"] {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        #clock {
            font-size: 20px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <h1>Welcome to AlphaSurf</h1>
    <div id="search">
        <input type="text" id="search-input" placeholder="Search DuckDuckGo...">
    </div>
    <div id="clock"></div>

    <script>
        function updateClock() {
            const now = new Date();
            document.getElementById('clock').textContent = 'Current time: ' + now.toLocaleTimeString();
        }
        setInterval(updateClock, 1000);
        updateClock();
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
            margin: 0;
            padding: 20px;
            background-color: #f3f4f6;
            color: #333;
        }
        h1 {
            color: #4a90e2;
        }
        label {
            display: block;
            margin: 10px 0 5px;
        }
        input[type="text"] {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        button {
            margin-top: 20px;
            padding: 10px 20px;
            background-color: #4a90e2;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <h1>Settings</h1>
    <label for="homepage">Set Home Page:</label>
    <input type="text" id="homepage" value="alpha://start">
    <button id="save-settings">Save Settings</button>

    <script>
        document.getElementById('save-settings').addEventListener('click', function() {
            const homepage = document.getElementById('homepage').value;
            alert('Settings saved! Home page set to: ' + homepage);
            // Implement saving functionality here
        });
    </script>
</body>
</html>
)";

struct Tab {
    WebKitWebView* web_view;
    GtkWidget* label;
};

std::vector<Tab> tabs;

void load_url(WebKitWebView* web_view, const gchar* url) {
    webkit_web_view_load_uri(web_view, url);
}

void load_start_page(WebKitWebView* web_view) {
    webkit_web_view_load_html(web_view, START_PAGE_HTML, nullptr);
}

void load_settings_page(WebKitWebView* web_view) {
    webkit_web_view_load_html(web_view, SETTINGS_PAGE_HTML, nullptr);
}

void on_refresh_button_clicked(WebKitWebView* web_view) {
    const gchar* uri = webkit_web_view_get_uri(web_view);
    if (uri) {
        webkit_web_view_reload(web_view);
    } else {
        load_start_page(web_view);
    }
}

void on_home_button_clicked(WebKitWebView* web_view) {
    load_start_page(web_view);
}

void on_new_tab_button_clicked(GtkNotebook* notebook) {
    WebKitWebView* new_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    load_start_page(new_web_view);  // Load start page in new tab

    GtkWidget* tab_label = gtk_label_new("New Tab");
    gtk_notebook_append_page(notebook, GTK_WIDGET(new_web_view), tab_label);

    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1);
    
    // Connect the "destroy" signal to clean up the web view
    g_signal_connect(new_web_view, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}

GtkWidget* create_toolbar(GtkNotebook* notebook) {
    GtkWidget* toolbar = gtk_toolbar_new();
    
    GtkToolItem* refresh_button = gtk_tool_button_new(NULL, "Refresh");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), refresh_button, -1);

    GtkToolItem* home_button = gtk_tool_button_new(NULL, "Home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), nullptr);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);

    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), notebook);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);

    return toolbar;
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);

    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
    GtkWidget* toolbar = create_toolbar(notebook);

    // Initial tab with start page
    WebKitWebView* initial_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    load_start_page(initial_web_view);
    
    GtkWidget* initial_tab_label = gtk_label_new("Home");
    gtk_notebook_append_page(notebook, GTK_WIDGET(initial_web_view), initial_tab_label);

    gtk_box_pack_start(GTK_BOX(notebook), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(notebook), GTK_WIDGET(notebook), TRUE, TRUE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_add(GTK_CONTAINER(window), notebook);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
