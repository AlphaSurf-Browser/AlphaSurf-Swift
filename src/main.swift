import Cocoa
import WebKit

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
                const query = this.value.trim();
                if (query) {
                    if (query.startsWith('http://') || query.startsWith('https://')) {
                        window.location.href = query;
                    } else {
                        window.location.href = 'https://www.google.com/search?q=' + encodeURIComponent(query);
                    }
                }
            }
        });

        // Load bookmarks from Swift
        function loadBookmarks(bookmarks) {
            const bookmarksDiv = document.getElementById('bookmarks');
            bookmarks.forEach(bookmark => {
                const bookmarkElem = document.createElement('div');
                bookmarkElem.className = 'bookmark';
                bookmarkElem.textContent = bookmark.title || bookmark.url;
                bookmarkElem.onclick = () => window.location.href = bookmark.url;
                bookmarksDiv.appendChild(bookmarkElem);
            });
        }
    </script>
</body>
</html>
"""

// Bookmark structure
struct Bookmark: Codable {
    var url: String
    var title: String
}

// Class to manage the browser
class AlphaSurfBrowser: NSWindowController, WKNavigationDelegate {
    var webView: WKWebView!
    var bookmarks: [Bookmark] = []

    override init(window: NSWindow?) {
        super.init(window: window)
        setupWebView()
        loadInitialPage()
        loadBookmarks()
    }

    // Setup WebView
    private func setupWebView() {
        webView = WKWebView(frame: .zero)
        webView.navigationDelegate = self
        window?.contentView = webView
        window?.makeKeyAndOrderFront(nil)
    }

    // Load the initial page
    private func loadInitialPage() {
        webView.loadHTMLString(startPageHTML, baseURL: nil)
    }

    // Add bookmark
    func addBookmark(url: String, title: String) {
        let newBookmark = Bookmark(url: url, title: title)
        bookmarks.append(newBookmark)
        saveBookmarks()
        
        // Call JavaScript function to add bookmark to the start page
        let js = "loadBookmarks(\(bookmarksToJSON()));"
        webView.evaluateJavaScript(js, completionHandler: nil)
    }

    // Load bookmarks from UserDefaults
    private func loadBookmarks() {
        if let data = UserDefaults.standard.data(forKey: "bookmarks") {
            let decoder = JSONDecoder()
            if let loadedBookmarks = try? decoder.decode([Bookmark].self, from: data) {
                bookmarks = loadedBookmarks
                let js = "loadBookmarks(\(bookmarksToJSON()));"
                webView.evaluateJavaScript(js, completionHandler: nil)
            }
        }
    }

    // Save bookmarks to UserDefaults
    private func saveBookmarks() {
        let encoder = JSONEncoder()
        if let data = try? encoder.encode(bookmarks) {
            UserDefaults.standard.set(data, forKey: "bookmarks")
        }
    }

    // Convert bookmarks to JSON format for JavaScript
    private func bookmarksToJSON() -> String {
        let jsonEncoder = JSONEncoder()
        if let jsonData = try? jsonEncoder.encode(bookmarks),
           let jsonString = String(data: jsonData, encoding: .utf8) {
            return jsonString
        }
        return "[]"
    }

    // WKNavigationDelegate method
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        // Handle page load completion
    }
}

// Main entry point
@main
struct AlphaSurfApp {
    static func main() {
        let app = NSApplication.shared
        let window = NSWindow(contentRect: NSMakeRect(0, 0, 800, 600),
                              styleMask: [.titled, .closable, .resizable],
                              backing: .buffered,
                              defer: false)
        window.title = "AlphaSurf"
        
        let browser = AlphaSurfBrowser(window: window)
        
        // Add example bookmarks for testing
        browser.addBookmark(url: "https://www.apple.com", title: "Apple")
        browser.addBookmark(url: "https://www.google.com", title: "Google")
        
        app.run()
    }
}
