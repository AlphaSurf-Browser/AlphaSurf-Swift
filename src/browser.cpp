#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

struct Settings {
    std::string homepageURL;
    std::string defaultSearchEngine;
    bool enableAnimations;
    int defaultZoomLevel;
    bool enableJavaScript;
    bool blockPopups;
    bool enableAdblock;
    bool showBookmarksBar;
    bool clearCacheOnExit;
    bool enableDarkMode;
    bool rememberHistory;
    bool enableExtensions;
    bool enableDeveloperTools;
    bool enableDoNotTrack;
    bool enableWebNotifications;
    std::string preferredLanguage;
    int maxTabsAllowed;

    // Constructor to set default values
    Settings() : homepageURL("alpha://start"), defaultSearchEngine("DuckDuckGo"),
                 enableAnimations(true), defaultZoomLevel(100), enableJavaScript(true),
                 blockPopups(true), enableAdblock(false), showBookmarksBar(false),
                 clearCacheOnExit(false), enableDarkMode(false), rememberHistory(true),
                 enableExtensions(false), enableDeveloperTools(false),
                 enableDoNotTrack(false), enableWebNotifications(false),
                 preferredLanguage("en"), maxTabsAllowed(10) {}
};

// Global settings variable
Settings settings;

// Function declarations
void loadSettings();
void saveSettings();
std::string getStartPageHTML();
std::string getNoInternetPageHTML();
std::string getSettingsPageHTML();
std::string getInstallPageHTML();
void load_url(WebKitWebView* web_view, const std::string& url);
void applySettings(WebKitWebView* web_view);
void installBrowser(const std::string& appName);
void on_window_destroy(GtkWidget*, gpointer);
void on_new_tab_button_clicked(GtkWidget*, gpointer);
void on_save_settings(GtkWidget*, gpointer);

// Function to load settings from a configuration file
void loadSettings() {
    std::ifstream inFile("settings.conf");
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.find("homepageURL=") == 0) settings.homepageURL = line.substr(12);
            else if (line.find("defaultSearchEngine=") == 0) settings.defaultSearchEngine = line.substr(20);
            else if (line.find("enableAnimations=") == 0) settings.enableAnimations = (line.substr(17) == "true");
            else if (line.find("defaultZoomLevel=") == 0) settings.defaultZoomLevel = std::stoi(line.substr(17));
            else if (line.find("enableJavaScript=") == 0) settings.enableJavaScript = (line.substr(17) == "true");
            else if (line.find("blockPopups=") == 0) settings.blockPopups = (line.substr(12) == "true");
            else if (line.find("enableAdblock=") == 0) settings.enableAdblock = (line.substr(14) == "true");
            else if (line.find("showBookmarksBar=") == 0) settings.showBookmarksBar = (line.substr(17) == "true");
            else if (line.find("clearCacheOnExit=") == 0) settings.clearCacheOnExit = (line.substr(17) == "true");
            else if (line.find("enableDarkMode=") == 0) settings.enableDarkMode = (line.substr(15) == "true");
            else if (line.find("rememberHistory=") == 0) settings.rememberHistory = (line.substr(16) == "true");
            else if (line.find("enableExtensions=") == 0) settings.enableExtensions = (line.substr(17) == "true");
            else if (line.find("enableDeveloperTools=") == 0) settings.enableDeveloperTools = (line.substr(21) == "true");
            else if (line.find("enableDoNotTrack=") == 0) settings.enableDoNotTrack = (line.substr(17) == "true");
            else if (line.find("enableWebNotifications=") == 0) settings.enableWebNotifications = (line.substr(21) == "true");
            else if (line.find("preferredLanguage=") == 0) settings.preferredLanguage = line.substr(18);
            else if (line.find("maxTabsAllowed=") == 0) settings.maxTabsAllowed = std::stoi(line.substr(15));
        }
        inFile.close();
    } else {
        std::cout << "No settings file found. Using default settings." << std::endl;
    }
}

// Function to save settings to a configuration file
void saveSettings() {
    std::ofstream outFile("settings.conf");
    if (outFile.is_open()) {
        outFile << "homepageURL=" << settings.homepageURL << "\n";
        outFile << "defaultSearchEngine=" << settings.defaultSearchEngine << "\n";
        outFile << "enableAnimations=" << (settings.enableAnimations ? "true" : "false") << "\n";
        outFile << "defaultZoomLevel=" << settings.defaultZoomLevel << "\n";
        outFile << "enableJavaScript=" << (settings.enableJavaScript ? "true" : "false") << "\n";
        outFile << "blockPopups=" << (settings.blockPopups ? "true" : "false") << "\n";
        outFile << "enableAdblock=" << (settings.enableAdblock ? "true" : "false") << "\n";
        outFile << "showBookmarksBar=" << (settings.showBookmarksBar ? "true" : "false") << "\n";
        outFile << "clearCacheOnExit=" << (settings.clearCacheOnExit ? "true" : "false") << "\n";
        outFile << "enableDarkMode=" << (settings.enableDarkMode ? "true" : "false") << "\n";
        outFile << "rememberHistory=" << (settings.rememberHistory ? "true" : "false") << "\n";
        outFile << "enableExtensions=" << (settings.enableExtensions ? "true" : "false") << "\n";
        outFile << "enableDeveloperTools=" << (settings.enableDeveloperTools ? "true" : "false") << "\n";
        outFile << "enableDoNotTrack=" << (settings.enableDoNotTrack ? "true" : "false") << "\n";
        outFile << "enableWebNotifications=" << (settings.enableWebNotifications ? "true" : "false") << "\n";
        outFile << "preferredLanguage=" << settings.preferredLanguage << "\n";
        outFile << "maxTabsAllowed=" << settings.maxTabsAllowed << "\n";
        outFile.close();
    }
}

// Function to get the current time as a string
std::string getCurrentTime() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return std::string(buf);
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
    std::string settingsHTML = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                    text-align: center;
                    padding: 50px;
                }
                label {
                    display: block;
                    margin: 10px 0;
                }
                input[type="text"], select {
                    width: 200px;
                    padding: 5px;
                    margin: 5px 0;
                }
                input[type="checkbox"] {
                    margin: 10px 5px;
                }
            </style>
        </head>
        <body>
            <h1>Settings - AlphaSurf</h1>
            <form id="settingsForm">
                <label>Homepage URL: <input type="text" id="homepageURL" value=")" + settings.homepageURL + R"("></label>
                <label>Default Search Engine:
                    <select id="defaultSearchEngine">
                        <option value="DuckDuckGo" )" + (settings.defaultSearchEngine == "DuckDuckGo" ? "selected" : "") + R"(>DuckDuckGo</option>
                        <option value="Google" )" + (settings.defaultSearchEngine == "Google" ? "selected" : "") + R"(>Google</option>
                        <option value="Bing" )" + (settings.defaultSearchEngine == "Bing" ? "selected" : "") + R"(>Bing</option>
                    </select>
                </label>
                <label><input type="checkbox" id="enableAnimations" )" + (settings.enableAnimations ? "checked" : "") + R"( /> Enable Animations</label>
                <label>Default Zoom Level: <input type="text" id="defaultZoomLevel" value=")" + std::to_string(settings.defaultZoomLevel) + R"("></label>
                <label><input type="checkbox" id="enableJavaScript" )" + (settings.enableJavaScript ? "checked" : "") + R"( /> Enable JavaScript</label>
                <label><input type="checkbox" id="blockPopups" )" + (settings.blockPopups ? "checked" : "") + R"( /> Block Popups</label>
                <label><input type="checkbox" id="enableAdblock" )" + (settings.enableAdblock ? "checked" : "") + R"( /> Enable Adblock</label>
                <label><input type="checkbox" id="showBookmarksBar" )" + (settings.showBookmarksBar ? "checked" : "") + R"( /> Show Bookmarks Bar</label>
                <label><input type="checkbox" id="clearCacheOnExit" )" + (settings.clearCacheOnExit ? "checked" : "") + R"( /> Clear Cache on Exit</label>
                <label><input type="checkbox" id="enableDarkMode" )" + (settings.enableDarkMode ? "checked" : "") + R"( /> Enable Dark Mode</label>
                <label><input type="checkbox" id="rememberHistory" )" + (settings.rememberHistory ? "checked" : "") + R"( /> Remember History</label>
                <label><input type="checkbox" id="enableExtensions" )" + (settings.enableExtensions ? "checked" : "") + R"( /> Enable Extensions</label>
                <label><input type="checkbox" id="enableDeveloperTools" )" + (settings.enableDeveloperTools ? "checked" : "") + R"( /> Enable Developer Tools</label>
                <label><input type="checkbox" id="enableDoNotTrack" )" + (settings.enableDoNotTrack ? "checked" : "") + R"( /> Enable Do Not Track</label>
                <label><input type="checkbox" id="enableWebNotifications" )" + (settings.enableWebNotifications ? "checked" : "") + R"( /> Enable Web Notifications</label>
                <label>Preferred Language: <input type="text" id="preferredLanguage" value=")" + settings.preferredLanguage + R"("></label>
                <label>Max Tabs Allowed: <input type="text" id="maxTabsAllowed" value=")" + std::to_string(settings.maxTabsAllowed) + R"("></label>
                <button type="button" onclick="saveSettings()">Save Settings</button>
            </form>
            <script>
                function saveSettings() {
                    const settings = {
                        homepageURL: document.getElementById('homepageURL').value,
                        defaultSearchEngine: document.getElementById('defaultSearchEngine').value,
                        enableAnimations: document.getElementById('enableAnimations').checked,
                        defaultZoomLevel: document.getElementById('defaultZoomLevel').value,
                        enableJavaScript: document.getElementById('enableJavaScript').checked,
                        blockPopups: document.getElementById('blockPopups').checked,
                        enableAdblock: document.getElementById('enableAdblock').checked,
                        showBookmarksBar: document.getElementById('showBookmarksBar').checked,
                        clearCacheOnExit: document.getElementById('clearCacheOnExit').checked,
                        enableDarkMode: document.getElementById('enableDarkMode').checked,
                        rememberHistory: document.getElementById('rememberHistory').checked,
                        enableExtensions: document.getElementById('enableExtensions').checked,
                        enableDeveloperTools: document.getElementById('enableDeveloperTools').checked,
                        enableDoNotTrack: document.getElementById('enableDoNotTrack').checked,
                        enableWebNotifications: document.getElementById('enableWebNotifications').checked,
                        preferredLanguage: document.getElementById('preferredLanguage').value,
                        maxTabsAllowed: document.getElementById('maxTabsAllowed').value
                    };
                    // Send settings to the main application to be saved
                    window.open('data:text/plain,' + JSON.stringify(settings), '_self');
                }
            </script>
        </body>
        </html>
    )";
}

// Function to get the HTML for the install page
std::string getInstallPageHTML() {
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
            <h1>Installing AlphaSurf...</h1>
            <p>Please wait while we set everything up.</p>
        </body>
        </html>
    )";
}

// Function to create a new tab and load the specified URL
void load_url(WebKitWebView* web_view, const std::string& url) {
    if (url == "alpha://start") {
        webkit_web_view_load_html(web_view, getStartPageHTML().c_str(), nullptr);
    } else if (url == "alpha://settings") {
        webkit_web_view_load_html(web_view, getSettingsPageHTML().c_str(), nullptr);
    } else {
        webkit_web_view_load_uri(web_view, url.c_str());
    }
}

// Function to handle installation
void installBrowser(const std::string& appName) {
    std::string installDir;

#ifdef _WIN32
    installDir = "C:\\Program Files\\" + appName + "\\";
#else
    installDir = "/usr/local/bin/";
#endif

    // Create the installation directory if it doesn't exist
    fs::create_directories(installDir);

    // Move the application binary to the installation directory
    fs::path currentPath = fs::current_path();
    fs::path appPath = currentPath / appName;

    if (fs::exists(appPath)) {
        fs::rename(appPath, installDir / appName);
    }

    // Save the settings to the installation directory
    saveSettings();
}

// Callback for when the window is destroyed
void on_window_destroy(GtkWidget*, gpointer) {
    saveSettings();
    gtk_main_quit();
}

// Callback for new tab button click
void on_new_tab_button_clicked(GtkWidget*, gpointer) {
    // Open a new tab with the homepage URL
    load_url(web_view, settings.homepageURL);
}

// Callback for saving settings
void on_save_settings(GtkWidget*, gpointer) {
    // Logic to save settings from the HTML form to the settings structure
    // This function would be triggered when the Save Settings button is clicked.
    // The actual logic will be integrated into the JavaScript in getSettingsPageHTML.
}

// Main function
int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    loadSettings();

    // Create main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Create WebView
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

    // Load the install page on first launch
    if (!fs::exists("settings.conf")) {
        webkit_web_view_load_html(web_view, getInstallPageHTML().c_str(), nullptr);
        installBrowser("AlphaSurf");
    } else {
        load_url(web_view, settings.homepageURL); // Load homepage URL
    }

    // Create new tab button
    GtkWidget* newTabButton = gtk_button_new_with_label("New Tab");
    g_signal_connect(newTabButton, "clicked", G_CALLBACK(on_new_tab_button_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(window), newTabButton);

    // Show everything
    gtk_widget_show_all(window);
    gtk_window_set_title(GTK_WINDOW(window), "AlphaSurf");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    gtk_main();
    return 0;
}
