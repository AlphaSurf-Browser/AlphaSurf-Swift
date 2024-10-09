#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <fstream>
#include <ctime>

// Path to the configuration file
const std::string CONFIG_FILE = "alphasurf_config.txt";

// Function to load settings from the config file
std::string load_setting(const std::string& key) {
    std::ifstream config(CONFIG_FILE);
    std::string line;
    while (std::getline(config, line)) {
        if (line.find(key) == 0) {
            return line.substr(key.size() + 1);
        }
    }
    return "";
}

// Function to save settings to the config file
void save_setting(const std::string& key, const std::string& value) {
    std::ofstream config(CONFIG_FILE, std::ios_base::app);
    config << key << "=" << value << "\n";
}

// Function to get the current time for the clock on the start page
std::string getCurrentTime() {
    time_t now = time(0);
    tm* localtm = localtime(&now);
    char time_str[10];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", localtm);
    return std::string(time_str);
}

// Function to get the HTML for the start page
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
                    opacity: 0;
                    animation: fadeIn 1s forwards;
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
                @keyframes fadeIn {
                    from { opacity: 0; }
                    to { opacity: 1; }
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

                function updateClock() {
                    const clock = document.getElementById('clock');
                    setInterval(() => {
                        const now = new Date();
                        clock.textContent = 'Time: ' + now.toLocaleTimeString();
                    }, 1000);
                }

                updateClock();
            </script>
        </body>
        </html>
    )";
}

// Function to get the HTML for the settings page
std::string getSettingsPageHTML() {
    std::string homepage = load_setting("homepage");
    std::string search_engine = load_setting("search_engine");
    std::string animations_enabled = load_setting("animations_enabled");
    std::string zoom_level = load_setting("zoom_level");
    std::string javascript_enabled = load_setting("javascript_enabled");
    std::string popup_blocking = load_setting("popup_blocking");
    std::string adblock_enabled = load_setting("adblock_enabled");
    std::string bookmarks_bar = load_setting("bookmarks_bar");
    std::string clear_cache_exit = load_setting("clear_cache_exit");
    std::string dark_mode = load_setting("dark_mode");
    std::string remember_history = load_setting("remember_history");
    std::string extensions_enabled = load_setting("extensions_enabled");
    std::string dev_tools_enabled = load_setting("dev_tools_enabled");
    std::string do_not_track = load_setting("do_not_track");
    std::string notifications_enabled = load_setting("notifications_enabled");
    std::string preferred_language = load_setting("preferred_language");
    std::string max_tabs_allowed = load_setting("max_tabs_allowed");

    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                    background-color: #f5f5f5;
                    text-align: center;
                    padding: 50px;
                    opacity: 0;
                    animation: fadeIn 1s forwards;
                }
                @keyframes fadeIn {
                    from { opacity: 0; }
                    to { opacity: 1; }
                }
                label {
                    display: block;
                    margin-top: 15px;
                }
            </style>
        </head>
        <body>
            <h1>Settings - AlphaSurf</h1>
            <form id="settings-form">
                <label>
                    Homepage URL: 
                    <input type="text" id="homepage" value=")" + homepage + R"(">
                </label>
                <label>
                    Default Search Engine:
                    <select id="search_engine">
                        <option value="duckduckgo" )" + (search_engine == "duckduckgo" ? "selected" : "") + R"(>DuckDuckGo</option>
                        <option value="google" )" + (search_engine == "google" ? "selected" : "") + R"(>Google</option>
                        <option value="bing" )" + (search_engine == "bing" ? "selected" : "") + R"(>Bing</option>
                    </select>
                </label>
                <label>
                    Default Zoom Level: 
                    <input type="number" id="zoom_level" value=")" + zoom_level + R"(" min="50" max="200">%
                </label>
                <label>
                    Enable JavaScript:
                    <input type="checkbox" id="javascript_enabled" )" + (javascript_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Animations: 
                    <input type="checkbox" id="animations" )" + (animations_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Block Popups:
                    <input type="checkbox" id="popup_blocking" )" + (popup_blocking == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Adblock:
                    <input type="checkbox" id="adblock_enabled" )" + (adblock_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Show Bookmarks Bar:
                    <input type="checkbox" id="bookmarks_bar" )" + (bookmarks_bar == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Clear Cache on Exit:
                    <input type="checkbox" id="clear_cache_exit" )" + (clear_cache_exit == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Dark Mode:
                    <input type="checkbox" id="dark_mode" )" + (dark_mode == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Remember History:
                    <input type="checkbox" id="remember_history" )" + (remember_history == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Extensions:
                    <input type="checkbox" id="extensions_enabled" )" + (extensions_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Developer Tools:
                    <input type="checkbox" id="dev_tools_enabled" )" + (dev_tools_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Do Not Track:
                    <input type="checkbox" id="do_not_track" )" + (do_not_track == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Enable Web Notifications:
                    <input type="checkbox" id="notifications_enabled" )" + (notifications_enabled == "true" ? "checked" : "") + R"(>
                </label>
                <label>
                    Preferred Language:
                    <input type="text" id="preferred_language" value=")" + preferred_language + R"(">
                </label>
                <label>
                    Max Tabs Allowed:
                    <input type="number" id="max_tabs_allowed" value=")" + max_tabs_allowed + R"(" min="1" max="100">
                </label>

                <button type="button" onclick="saveSettings()">Save Settings</button>
            </form>

            <script>
                function saveSettings() {
                    const settings = {
                        homepage: document.getElementById('homepage').value,
                        search_engine: document.getElementById('search_engine').value,
                        zoom_level: document.getElementById('zoom_level').value,
                        javascript_enabled: document.getElementById('javascript_enabled').checked,
                        animations: document.getElementById('animations').checked,
                        popup_blocking: document.getElementById('popup_blocking').checked,
                        adblock_enabled: document.getElementById('adblock_enabled').checked,
                        bookmarks_bar: document.getElementById('bookmarks_bar').checked,
                        clear_cache_exit: document.getElementById('clear_cache_exit').checked,
                        dark_mode: document.getElementById('dark_mode').checked,
                        remember_history: document.getElementById('remember_history').checked,
                        extensions_enabled: document.getElementById('extensions_enabled').checked,
                        dev_tools_enabled: document.getElementById('dev_tools_enabled').checked,
                        do_not_track: document.getElementById('do_not_track').checked,
                        notifications_enabled: document.getElementById('notifications_enabled').checked,
                        preferred_language: document.getElementById('preferred_language').value,
                        max_tabs_allowed: document.getElementById('max_tabs_allowed').value
                    };

                    fetch('/save-settings', {
                        method: 'POST',
                        body: JSON.stringify(settings),
                        headers: {
                            'Content-Type': 'application/json'
                        }
                    }).then(response => response.json()).then(data => {
                        alert('Settings saved!');
                    }).catch(error => {
                        alert('Error saving settings: ' + error);
                    });
                }
            </script>
        </body>
        </html>
    )";
}

// Function to load URL or built-in pages
void load_url(WebKitWebView* web_view, const std::string& url) {
    if (url == "alpha://start") {
        webkit_web_view_load_html(web_view, getStartPageHTML().c_str(), nullptr);
    } else if (url == "alpha://settings") {
        webkit_web_view_load_html(web_view, getSettingsPageHTML().c_str(), nullptr);
    } else {
        webkit_web_view_load_uri(web_view, url.c_str());
    }
}

// Main function
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    // Setup screen logic
    if (load_setting("setup_complete") != "true") {
        // Show the setup/install screen on first launch
        WebKitWebView* setup_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
        load_url(setup_web_view, "alpha://setup");
        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(setup_web_view));
        g_signal_connect(setup_web_view, "decide-policy", G_CALLBACK(on_setup_policy_decision), NULL);

        gtk_widget_show_all(window);
        gtk_main();

        return 0;
    }

    // Normal launch with tabs and settings
    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());

    // Create the initial tab with start page or no internet page
    WebKitWebView* initial_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    std::string homepage = load_setting("homepage");
    if (!homepage.empty()) {
        load_url(initial_web_view, homepage);
    } else {
        load_url(initial_web_view, "alpha://start");
    }

    GtkWidget* initial_tab_label = gtk_label_new("Home");
    gtk_notebook_append_page(notebook, GTK_WIDGET(initial_web_view), initial_tab_label);

    // Toolbar with new tab button
    GtkWidget* toolbar = gtk_toolbar_new();
    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(notebook), TRUE, TRUE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
