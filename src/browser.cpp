#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

const std::string CONFIG_FILE = "alphasurf_config.txt";
const int MAX_DEFAULT_TABS = 10;  // Default max tabs allowed
int maxTabsAllowed = MAX_DEFAULT_TABS;  // Global tab count limiter

std::vector<WebKitWebView*> openTabs;  // Store open tabs
// Define the on_setup_policy_decision function
gboolean on_setup_policy_decision(WebKitWebView* web_view, WebKitPolicyDecision* decision, WebKitPolicyDecisionType type, gpointer user_data) {
    // Check the type of decision being made
    if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationPolicyDecision* navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitNavigationAction* action = webkit_navigation_policy_decision_get_navigation_action(navigation_decision);
        WebKitURIRequest* request = webkit_navigation_action_get_request(action);
        const gchar* uri = webkit_uri_request_get_uri(request);

        // Example: Allow everything, but you can add conditional logic to allow/block certain requests
        g_print("Navigating to: %s\n", uri);

        // Accept the decision to proceed with navigation
        webkit_policy_decision_use(decision);
        return TRUE;
    }

    // Default policy decision if not handled
    return FALSE;
}
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

// Load max tabs allowed
void loadMaxTabsAllowed() {
    std::string maxTabs = load_setting("max_tabs_allowed");
    if (!maxTabs.empty()) {
        maxTabsAllowed = std::stoi(maxTabs);
    }
}

// Check if new tab can be opened
bool canOpenNewTab() {
    return openTabs.size() < maxTabsAllowed;
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

// Function to handle new tab creation
void addNewTab(GtkNotebook* notebook, const std::string& url) {
    if (canOpenNewTab()) {
        WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
        openTabs.push_back(web_view);  // Add the new tab to the vector

        GtkWidget* tab_label = gtk_label_new(("Tab " + std::to_string(openTabs.size())).c_str());
        gtk_notebook_append_page(notebook, GTK_WIDGET(web_view), tab_label);
        gtk_widget_show_all(GTK_WIDGET(web_view));

        load_url(web_view, url);  // Load URL (homepage or start page) in the new tab
    } else {
        g_print("Max tabs limit reached!\n");
    }
}

// Load a URL or a built-in page
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

    // Load max tabs setting
    loadMaxTabsAllowed();

    // Create the notebook for tabs
    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());

    // Create the initial tab
    WebKitWebView* initial_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    std::string homepage = load_setting("homepage");
    load_url(initial_web_view, homepage.empty() ? "alpha://start" : homepage);
    GtkWidget* initial_tab_label = gtk_label_new("Home");
    gtk_notebook_append_page(notebook, GTK_WIDGET(initial_web_view), initial_tab_label);
    openTabs.push_back(initial_web_view);  // Add the initial tab to openTabs

    // Toolbar for new tabs
    GtkWidget* toolbar = gtk_toolbar_new();
    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);

    // Event listener for adding new tabs
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK([](GtkWidget*, gpointer data) {
        addNewTab(GTK_NOTEBOOK(data), "alpha://start");
    }), notebook);

    // Box layout to hold toolbar and notebook
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(notebook), TRUE, TRUE, 0);

    // Exit clean-up
    g_signal_connect(window, "destroy", G_CALLBACK([](GtkWidget*, gpointer) {
        gtk_main_quit();
    }), nullptr);

    // Finalize the window and start the GTK main loop
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
