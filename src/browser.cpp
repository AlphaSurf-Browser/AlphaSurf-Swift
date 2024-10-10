#include <iostream>
#include <fstream>
#include <json/json.h>
#include <webview.h>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

const char* SPLASH_SCREEN_HTML = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AlphaSurf - Splash Screen</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: #764ba2;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            color: white;
        }
        h1 {
            font-size: 48px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }
    </style>
</head>
<body>
    <h1>Welcome to AlphaSurf</h1>
</body>
</html>
)html";

const char* START_PAGE_HTML = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AlphaSurf</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            height: 100vh;
            color: white;
        }
        .container {
            text-align: center;
            padding: 40px;
        }
        h1 {
            font-size: 48px;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
        }
        #search {
            width: 100%;
            padding: 15px;
            font-size: 18px;
            border: none;
            border-radius: 30px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            transition: all 0.3s ease;
        }
        #search::placeholder {
            color: rgba(255, 255, 255, 0.7);
        }
        #search:focus {
            outline: none;
            box-shadow: 0 0 15px rgba(255, 255, 255, 0.5);
            background: rgba(255, 255, 255, 0.3);
        }
        #controls {
            margin: 20px 0;
        }
        button {
            padding: 10px 15px;
            margin: 0 5px;
            border: none;
            border-radius: 5px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        button:hover {
            background: rgba(255, 255, 255, 0.3);
        }
        #bookmarks, #history {
            margin-top: 20px;
        }
        .item {
            cursor: pointer;
            padding: 10px;
            margin: 5px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 10px;
            transition: all 0.3s ease;
        }
        .item:hover {
            background: rgba(255, 255, 255, 0.3);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>AlphaSurf</h1>
        <input type="text" id="search" placeholder="Search or enter URL" />
        <div id="controls">
            <button id="back">Back</button>
            <button id="forward">Forward</button>
            <button id="refresh">Refresh</button>
            <button id="settings">Settings</button>
        </div>
        <div id="bookmarks"></div>
        <div id="history"></div>
    </div>
    <script>
        document.getElementById('search').addEventListener('keypress', function (e) {
            if (e.key === 'Enter') {
                const query = this.value;
                if (query) {
                    if (query.startsWith('http://') || query.startsWith('https://')) {
                        window.location.href = query;
                        window.addHistory(query);
                    } else {
                        window.location.href = 'https://www.google.com/search?q=' + encodeURIComponent(query);
                        window.addHistory('https://www.google.com/search?q=' + encodeURIComponent(query));
                    }
                }
            }
        });

        document.getElementById('back').onclick = () => window.back();
        document.getElementById('forward').onclick = () => window.forward();
        document.getElementById('refresh').onclick = () => window.refresh();
        document.getElementById('settings').onclick = () => window.location.href = 'alpha://settings';
    </script>
</body>
</html>
)html";

// Global variables
std::vector<std::string> bookmarks;
std::vector<std::string> history;

// Function to load bookmarks from JSON file
void load_bookmarks() {
    std::ifstream file("bookmarks.json");
    if (file.is_open()) {
        Json::Value jsonData;
        file >> jsonData;
        for (const auto& bookmark : jsonData["bookmarks"]) {
            bookmarks.push_back(bookmark.asString());
        }
    }
}

// Function to save bookmarks to JSON file
void save_bookmarks() {
    Json::Value jsonData;
    for (const auto& bookmark : bookmarks) {
        jsonData["bookmarks"].append(bookmark);
    }
    std::ofstream file("bookmarks.json");
    file << jsonData;
}

// Function to load history from JSON file
void load_history() {
    std::ifstream file("history.json");
    if (file.is_open()) {
        Json::Value jsonData;
        file >> jsonData;
        for (const auto& entry : jsonData["history"]) {
            history.push_back(entry.asString());
        }
    }
}

// Function to save history to JSON file
void save_history() {
    Json::Value jsonData;
    for (const auto& entry : history) {
        jsonData["history"].append(entry);
    }
    std::ofstream file("history.json");
    file << jsonData;
}

// Function to add a new bookmark
void add_bookmark(const std::string& url) {
    bookmarks.push_back(url);
    save_bookmarks();
}

// Function to show bookmarks
void show_bookmarks(webview::webview& w) {
    std::string bookmarks_html;
    for (const auto& bookmark : bookmarks) {
        bookmarks_html += "<div class='item' onclick=\"window.location.href='" + bookmark + "'\">" + bookmark + "</div>";
    }
    std::string js = "document.getElementById('bookmarks').innerHTML = `" + bookmarks_html + "`;";
    w.eval(js);
}

// Function to show history
void show_history(webview::webview& w) {
    std::string history_html;
    for (const auto& entry : history) {
        history_html += "<div class='item' onclick=\"window.location.href='" + entry + "'\">" + entry + "</div>";
    }
    std::string js = "document.getElementById('history').innerHTML = `" + history_html + "`;";
    w.eval(js);
}

// Function to add a new history entry
void add_history(const std::string& url) {
    history.push_back(url);
    save_history();
}

// Main function
int main() {
    webview::webview w(true, nullptr);

    // Load bookmarks and history at startup
    load_bookmarks();
    load_history();

    // Setup the splash screen
    w.set_title("AlphaSurf - Loading...");
    w.set_size(800, 600, WEBVIEW_HINT_NONE);
    w.navigate("data:text/html," + std::string(SPLASH_SCREEN_HTML));
    
    // Show the splash screen for a short duration
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Load the main start page
    w.navigate("data:text/html," + std::string(START_PAGE_HTML));

    // Handle bookmark addition
    w.bind("addBookmark", [](const std::string& url) {
        add_bookmark(url);
    });

    // Handle history addition
    w.bind("addHistory", [](const std::string& url) {
        add_history(url);
    });

    // Show bookmarks and history in the UI
    show_bookmarks(w);
    show_history(w);

    // Start the webview event loop
    w.run();
    return 0;
}
