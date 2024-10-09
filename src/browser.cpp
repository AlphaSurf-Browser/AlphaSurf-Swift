#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

const char* START_PAGE = "alpha://start";
const char* SETTINGS_PAGE = "alpha://settings";

// Function to load the DuckDuckGo search URL
void perform_search(WebKitWebView* web_view, const gchar* query) {
    gchar* search_url = g_strdup_printf("https://www.duckduckgo.com/?q=%s", query);
    webkit_web_view_load_uri(web_view, search_url);
    g_free(search_url);
}

// Function to create the start page with embedded HTML
GtkWidget* create_start_page(WebKitWebView* web_view) {
    const gchar* html_content = R"html(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>AlphaSurf Start Page</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 20px;
                background-color: #f0f0f0;
            }
            .navbar {
                background-color: #333;
                overflow: hidden;
            }
            .navbar a {
                float: left;
                display: block;
                color: white;
                text-align: center;
                padding: 14px 16px;
                text-decoration: none;
            }
            .navbar a:hover {
                background-color: #ddd;
                color: black;
            }
            #search {
                margin-top: 20px;
            }
            #time {
                margin-top: 20px;
                font-size: 18px;
            }
        </style>
    </head>
    <body>
        <div class="navbar">
            <a href="#" id="home-link">Home</a>
            <a href="#" id="settings-link">Settings</a>
        </div>
        <input type="text" id="search" placeholder="Search DuckDuckGo..." />
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
                        window.open('https://www.duckduckgo.com/?q=' + encodeURIComponent(query), '_self');
                    }
                }
            });
        </script>
    </body>
    </html>
    )html";

    webkit_web_view_load_html(web_view, html_content, nullptr);
    return web_view;
}

// Function to create a new tab with the start page
GtkWidget* create_new_tab() {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    create_start_page(web_view);
    return GTK_WIDGET(web_view); // Cast to GtkWidget*
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);
    
    // Create a notebook for tab management
    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // Create the first tab with the start page
    GtkWidget* initial_tab = create_new_tab(); // Use GtkWidget* here
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), initial_tab, gtk_label_new("Tab 1"));

    // Connect the "destroy" signal to exit the application
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show the window
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
