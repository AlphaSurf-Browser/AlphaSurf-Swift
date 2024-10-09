#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>

// Struct to represent a tab
struct Tab {
    std::string url;
    bool is_secure; // Indicates whether the tab is secure (https)
};

// Global variables
std::vector<Tab> tabs;
size_t current_tab_index = 0;

// Function to get the current time in a formatted string
std::string getCurrentTime() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// Function to create the start page HTML
std::string getStartPageHTML() {
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                    background-color: #f5f5f5;
                    margin: 0;
                    padding: 20px;
                    text-align: center;
                }
                h1 {
                    color: #007bff;
                }
                #searchbar {
                    padding: 10px;
                    width: 80%;
                    max-width: 600px;
                    border: 1px solid #007bff;
                    border-radius: 5px;
                    margin-bottom: 20px;
                    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
                }
                #clock {
                    font-size: 1.2em;
                    margin-top: 10px;
                    color: #555;
                }
            </style>
        </head>
        <body>
            <h1>Welcome to AlphaSurf</h1>
            <input id="searchbar" type="text" placeholder="Search DuckDuckGo...">
            <div id="clock">Time: )" + getCurrentTime() + R"(</div>

            <script>
                document.getElementById('searchbar').addEventListener('keypress', function(e) {
                    if (e.key === 'Enter') {
                        const query = this.value;
                        if (query) {
                            window.location.href = 'https://duckduckgo.com/?q=' + encodeURIComponent(query);
                        }
                    }
                });
            </script>
        </body>
        </html>
    )";
}

// Function to get the HTML for the no internet page
std::string getNoInternetPageHTML() {
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                    text-align: center;
                    padding: 50px;
                }
                h1 {
                    color: red;
                }
            </style>
        </head>
        <body>
            <h1>No Internet</h1>
            <p>Please check your connection and try again.</p>
        </body>
        </html>
    )";
}

// Function to get the HTML for the settings page
std::string getSettingsPageHTML() {
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                    text-align: center;
                    padding: 50px;
                }
            </style>
        </head>
        <body>
            <h1>Settings - AlphaSurf</h1>
            <p>Here you can customize your browser settings.</p>
            <button onclick="window.close()">Close Settings</button>
        </body>
        </html>
    )";
}

// Function to load URL and manage tabs
void load_url(WebKitWebView* web_view, const std::string& url) {
    if (url == "alpha://start") {
        webkit_web_view_load_html(web_view, getStartPageHTML().c_str(), nullptr);
    } else if (url == "alpha://settings") {
        webkit_web_view_load_html(web_view, getSettingsPageHTML().c_str(), nullptr);
    } else {
        webkit_web_view_load_uri(web_view, url.c_str());
    }

    // Update the current tab URL
    if (current_tab_index < tabs.size()) {
        tabs[current_tab_index].url = url;
        tabs[current_tab_index].is_secure = url.substr(0, 5) == "https"; // Simple check for HTTPS
    }
}

// Function to create the toolbar UI
GtkWidget* create_toolbar(WebKitWebView* web_view) {
    GtkWidget* toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

    // Back button
    GtkToolItem* back_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
    g_signal_connect(back_button, "clicked", G_CALLBACK([](GtkToolItem* item) {
        if (current_tab_index > 0) {
            current_tab_index--;
            load_url(web_view, tabs[current_tab_index].url);
        }
    }), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), back_button, -1);

    // Forward button
    GtkToolItem* forward_button = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    g_signal_connect(forward_button, "clicked", G_CALLBACK([](GtkToolItem* item) {
        if (current_tab_index < tabs.size() - 1) {
            current_tab_index++;
            load_url(web_view, tabs[current_tab_index].url);
        }
    }), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), forward_button, -1);

    // Refresh button
    GtkToolItem* refresh_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK([](GtkToolItem* item) {
        if (!tabs.empty()) {
            load_url(web_view, tabs[current_tab_index].url);
        }
    }), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), refresh_button, -1);

    // Home button
    GtkToolItem* home_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
    g_signal_connect(home_button, "clicked", G_CALLBACK([](GtkToolItem* item) {
        load_url(web_view, "alpha://start");
    }), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);

    // New Tab button
    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK([](GtkToolItem* item) {
        Tab new_tab{"alpha://start", false};
        tabs.push_back(new_tab);
        current_tab_index = tabs.size() - 1; // Switch to the new tab
        load_url(web_view, "alpha://start");
    }), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);

    return toolbar;
}

// Main function to start the GTK application and WebKit
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Create WebView
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

    // Create and add the toolbar
    GtkWidget* toolbar = create_toolbar(web_view);
    gtk_box_pack_start(GTK_BOX(gtk_vbox_new(FALSE, 0)), toolbar, FALSE, FALSE, 0);

    // Load the start page by default
    load_url(web_view, "alpha://start");

    // Connect signals for window close
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Connect the load changed signal to handle URL changes and updates
    g_signal_connect(web_view, "load-changed", G_CALLBACK([](WebKitWebView* web_view, WebKitLoadEvent load_event) {
        if (load_event == WEBKIT_LOAD_COMMITTED) {
            const gchar* uri = webkit_web_view_get_uri(web_view);
            std::cout << "Current URL: " << uri << std::endl;
        }
    }), nullptr);

    // Connect load error signal to show no internet page
    g_signal_connect(web_view, "load-failed", G_CALLBACK([](WebKitWebView* web_view, WebKitLoadEvent load_event, const gchar* failing_uri, const gchar* error_domain, int error_code) {
        std::cout << "Load failed: " << error_domain << ", " << error_code << std::endl;
        webkit_web_view_load_html(web_view, getNoInternetPageHTML().c_str(), nullptr);
    }), nullptr);

    // Show the window
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
