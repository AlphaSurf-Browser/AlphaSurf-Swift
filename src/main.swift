import Gtk
import WebKit
import GLib
import GIO

// Constants for the start page HTML
let startPageHTML = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome to AlphaSurf</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: white;
        }
        .container {
            text-align: center;
            background: rgba(255, 255, 255, 0.1);
            padding: 40px;
            border-radius: 20px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
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
        #time {
            font-size: 24px;
            margin-top: 20px;
            font-weight: 300;
        }
        #bookmarks {
            margin-top: 30px;
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
        }
        .bookmark {
            background: rgba(255, 255, 255, 0.2);
            border-radius: 15px;
            padding: 10px 20px;
            margin: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .bookmark:hover {
            background: rgba(255, 255, 255, 0.3);
            transform: translateY(-3px);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>AlphaSurf</h1>
        <input type="text" id="search" placeholder="Search or enter URL" />
        <div id="time"></div>
        <div id="bookmarks"></div>
    </div>
    <script>
        function updateTime() {
            const now = new Date();
            document.getElementById('time').textContent = now.toLocaleTimeString();
        }
        setInterval(updateTime, 1000);
        updateTime();

        document.getElementById('search').addEventListener('keypress', function (e) {
            if (e.key === 'Enter') {
                const query = this.value;
                if (query) {
                    if (query.startsWith('http://') || query.startsWith('https://')) {
                        window.location.href = query;
                    } else {
                        window.location.href = 'https://www.google.com/search?q=' + encodeURIComponent(query);
                    }
                }
            }
        });

        // Function to load bookmarks (to be implemented)
        function loadBookmarks() {
            // This function will be called from C++ to populate bookmarks
        }

        // Function to add a bookmark to the start page
        function addBookmark(url, title) {
            const bookmarksDiv = document.getElementById('bookmarks');
            const bookmarkElem = document.createElement('div');
            bookmarkElem.className = 'bookmark';
            bookmarkElem.textContent = title || url;
            bookmarkElem.onclick = () => window.location.href = url;
            bookmarksDiv.appendChild(bookmarkElem);
        }
    </script>
</body>
</html>
"""

// Global variables
var mainWindow: UnsafeMutablePointer<GtkWidget>!
var notebook: UnsafeMutablePointer<GtkNotebook>!
var addressBar: UnsafeMutablePointer<GtkEntry>!
var bookmarks: [String] = []
var history: [String] = []
let adBlockList: Set<String> = ["example.com", "ads.com"]

// Function prototypes
func loadSettings()
func saveBookmarks()
func addBookmark(url: String, title: String)
func showBookmarks()
func updateAddressBar(url: String)
func getCurrentTabURL() -> String
func createNewTab(url: String)
func onTabSwitch(notebook: UnsafeMutablePointer<GtkNotebook>?, page: UnsafeMutablePointer<GtkWidget>?, pageNum: UInt, userData: UnsafeMutableRawPointer?)
func onNewTabButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func onBookmarksButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func onSettingsButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func onDevToolsButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func onHistoryButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func onIncognitoButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?)
func initializeUI()

// Load settings and bookmarks from JSON file
func loadSettings() {
    do {
        let fileUrl = URL(fileURLWithPath: "bookmarks.json")
        let data = try Data(contentsOf: fileUrl)
        let json = try JSONSerialization.jsonObject(with: data, options: []) as! [[String: String]]
        
        for obj in json {
            if let url = obj["url"], let title = obj["title"] {
                bookmarks.append(url)
                // Call JavaScript function to add bookmark to start page
                let webView = WEBKIT_WEB_VIEW(gtk_notebook_get_nth_page(notebook, 0))
                let js = "addBookmark('\(url)', '\(title)');"
                webkit_web_view_run_javascript(webView, js, nil, nil, nil)
            }
        }
    } catch {
        print("Failed to load bookmarks: \(error)")
    }
}

// Save bookmarks to a JSON file
func saveBookmarks() {
    do {
        let bookmarksArray = bookmarks.map { ["url": $0, "title": $0] }
        let jsonData = try JSONSerialization.data(withJSONObject: bookmarksArray, options: .prettyPrinted)
        try jsonData.write(to: URL(fileURLWithPath: "bookmarks.json"))
    } catch {
        print("Failed to save bookmarks: \(error)")
    }
}

// Function to add a new bookmark
func addBookmark(url: String, title: String) {
    bookmarks.append(url)
    saveBookmarks()
    
    // Add bookmark to start page
    let webView = WEBKIT_WEB_VIEW(gtk_notebook_get_nth_page(notebook, 0))
    let js = "addBookmark('\(url)', '\(title)');"
    webkit_web_view_run_javascript(webView, js, nil, nil, nil)
}

// Function to show bookmarks
func showBookmarks() {
    let dialog = gtk_dialog_new_with_buttons("Bookmarks",
                                             GTK_WINDOW(mainWindow),
                                             GTK_DIALOG_MODAL,
                                             "Close",
                                             GTK_RESPONSE_CLOSE,
                                             nil)
    let contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog))
    let listBox = gtk_list_box_new()
    gtk_container_add(GTK_CONTAINER(contentArea), listBox)

    for bookmark in bookmarks {
        let label = gtk_label_new(bookmark)
        gtk_list_box_insert(GTK_LIST_BOX(listBox), label, -1)
    }

    gtk_widget_show_all(dialog)
    gtk_dialog_run(GTK_DIALOG(dialog))
    gtk_widget_destroy(dialog)
}

// Function to update the address bar
func updateAddressBar(url: String) {
    if gtk_is_entry(addressBar) {
        gtk_entry_set_text(addressBar, url)
    }
}

// Function to retrieve the current tab URL
func getCurrentTabURL() -> String {
    let currentPage = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook))
    if let currentPage = currentPage {
        let webView = WEBKIT_WEB_VIEW(currentPage)
        return String(cString: webkit_web_view_get_uri(webView))
    }
    return ""
}

// Function to create a new tab
func createNewTab(url: String) {
    let webView = webkit_web_view_new()
    
    if url == "alpha://start" {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(webView), startPageHTML, nil)
    } else {
        // Check for ad blocking
        for adDomain in adBlockList {
            if url.contains(adDomain) {
                print("Blocked ad URL: \(url)")
                return // Don't load the ad URL
            }
        }
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webView), url)
    }
    
    gtk_notebook_append_page(notebook, webView, gtk_label_new(url))
    gtk_notebook_set_current_page(notebook, gtk_notebook_get_n_pages(notebook) - 1)
    gtk_widget_show(webView)
}

// Function to handle tab switching
func onTabSwitch(notebook: UnsafeMutablePointer<GtkNotebook>?, page: UnsafeMutablePointer<GtkWidget>?, pageNum: UInt, userData: UnsafeMutableRawPointer?) {
    updateAddressBar(getCurrentTabURL())
}

// Function to handle new tab button click
func onNewTabButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?) {
    createNewTab(url: "alpha://start")
}

// Function to handle bookmarks button click
func onBookmarksButtonClicked(widget: UnsafeMutablePointer<GtkWidget>?) {
    showBookmarks()
}

// Initialize the UI
func initializeUI() {
    gtk_init(nil, nil)
    
    // Main window setup
    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL)
    gtk_window_set_title(GTK_WINDOW(mainWindow), "AlphaSurf")
    gtk_window_set_default_size(GTK_WINDOW(mainWindow), 800, 600)
    
    // Notebook setup
    notebook = gtk_notebook_new()
    gtk_container_add(GTK_CONTAINER(mainWindow), notebook)
    
    // Create the first tab with the start page
    createNewTab(url: "alpha://start")
    
    // Connect signal for tab switching
    g_signal_connect(notebook, "switch-page", unsafeBitCast(onTabSwitch as AnyObject, to: GCallback.self), nil)
    
    // Connect signal for window destroy
    g_signal_connect(mainWindow, "destroy", gtk_main_quit, nil)
    
    // Show all widgets
    gtk_widget_show_all(mainWindow)
    gtk_main()
}

// Main function to start the application
func main() {
    loadSettings()
    initializeUI()
}

main()
