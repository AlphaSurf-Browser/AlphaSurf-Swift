import SwiftUI
import WebKit
import SafariServices

class BrowserViewModel: ObservableObject {
    @Published var tabs: [Tab] = [Tab(url: URL(string: "https://www.google.com")!)]
    @Published var selectedTab: Int = 0
    @Published var addressBarText: String = ""
    @Published var bookmarks: [String] = []
    @Published var history: [String] = []
    @Published var isShowingBookmarks = false
    @Published var isShowingHistory = false
    @Published var isShowingSettings = false
    @Published var isIncognitoMode = false
    
    private let adBlockPatterns: [String] = [
        #"^https?:\/\/.*\.doubleclick\.net"#,
        #"^https?:\/\/.*\.googlesyndication\.com"#,
        #"^https?:\/\/.*\.googleadservices\.com"#,
        #"^https?:\/\/.*ad\..*\.com"#,
        #"^https?:\/\/.*ads\..*\.com"#,
        #"^https?:\/\/.*banner\..*\.com"#,
        #"^https?:\/\/.*\.adnxs\.com"#,
        #"^https?:\/\/.*\.outbrain\.com"#,
        #"^https?:\/\/.*\.taboola\.com"#
    ]
    
    private lazy var adBlockRegexes: [NSRegularExpression] = {
        return adBlockPatterns.compactMap { pattern in
            try? NSRegularExpression(pattern: pattern, options: .caseInsensitive)
        }
    }()
    
    func loadPage() {
        guard var urlString = addressBarText.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) else { return }
        
        if !urlString.hasPrefix("http://") && !urlString.hasPrefix("https://") {
            urlString = "https://\(urlString)"
        }
        
        guard let url = URL(string: urlString) else { return }
        
        if shouldBlockAd(url: url) {
            print("Blocked ad URL: \(url.absoluteString)")
            return
        }
        
        loadURL(url)
    }
    
    func loadURL(_ url: URL) {
        tabs[selectedTab].url = url
        addressBarText = url.absoluteString
    }
    
    func shouldBlockAd(url: URL) -> Bool {
        let urlString = url.absoluteString
        return adBlockRegexes.contains { regex in
            regex.firstMatch(in: urlString, options: [], range: NSRange(location: 0, length: urlString.utf16.count)) != nil
        }
    }
    
    func addNewTab() {
        tabs.append(Tab(url: URL(string: "https://www.google.com")!))
        selectedTab = tabs.count - 1
    }
    
    func updateAddressBar(with urlString: String) {
        addressBarText = urlString
    }
    
    func addBookmark(url: String) {
        if !bookmarks.contains(url) {
            bookmarks.append(url)
        }
    }
    
    func addToHistory(url: String) {
        if !isIncognitoMode {
            history.append(url)
        }
    }
    
    func showBookmarks() {
        isShowingBookmarks = true
    }
    
    func showHistory() {
    isShowingHistory = true
    }
    
    func showSettings() {
        isShowingSettings = true
    }
    
    func toggleIncognitoMode() {
        isIncognitoMode.toggle()
    }
}

// ... (rest of the code remains the same)

struct BrowserWebView: UIViewRepresentable {
    let url: URL?
    @ObservedObject var viewModel: BrowserViewModel
    
    func makeUIView(context: Context) -> WKWebView {
        let config = WKWebViewConfiguration()
        config.userContentController.add(context.coordinator, name: "bookmarkHandler")
        
        let webView = WKWebView(frame: .zero, configuration: config)
        webView.navigationDelegate = context.coordinator
        return webView
    }
    
    func updateUIView(_ uiView: WKWebView, context: Context) {
        if let url = url {
            uiView.load(URLRequest(url: url))
        }
    }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator: NSObject, WKNavigationDelegate, WKScriptMessageHandler {
        var parent: BrowserWebView
        
        init(_ parent: BrowserWebView) {
            self.parent = parent
        }
        
        func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction, decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
            if let url = navigationAction.request.url, parent.viewModel.shouldBlockAd(url: url) {
                print("Blocked ad URL: \(url.absoluteString)")
                decisionHandler(.cancel)
                return
            }
            decisionHandler(.allow)
        }
        
        func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
            if let url = webView.url {
                parent.viewModel.updateAddressBar(with: url.absoluteString)
                parent.viewModel.addToHistory(url: url.absoluteString)
            }
        }
        
        func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
            if message.name == "bookmarkHandler", let url = message.body as? String {
                parent.viewModel.addBookmark(url: url)
            }
        }
    }
}
