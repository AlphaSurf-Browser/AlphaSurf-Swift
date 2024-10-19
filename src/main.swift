import SwiftUI
import WebKit

struct ContentView: View {
    @StateObject private var viewModel = BrowserViewModel()
    
    var body: some View {
        NavigationView {
            VStack {
                HStack {
                    TextField("Enter URL or search", text: $viewModel.addressBarText, onCommit: {
                        viewModel.loadPage()
                    })
                    .textFieldStyle(RoundedBorderTextFieldStyle())
                    .disableAutocorrection(true)
                    
                    Button(action: viewModel.loadPage) {
                        Image(systemName: "arrow.right")
                    }
                }
                .padding(.horizontal)
                
                TabView(selection: $viewModel.selectedTab) {
                    ForEach(viewModel.tabs.indices, id: \.self) { index in
                        BrowserWebView(url: viewModel.tabs[index].url, viewModel: viewModel)
                            .tag(index)
                    }
                }
                .tabViewStyle(PageTabViewStyle(indexDisplayMode: .never))
                
                HStack {
                    Button(action: viewModel.addNewTab) {
                        Image(systemName: "plus")
                    }
                    Spacer()
                    Button(action: viewModel.showBookmarks) {
                        Image(systemName: "book")
                    }
                    Spacer()
                    Button(action: viewModel.showHistory) {
                        Image(systemName: "clock")
                    }
                    Spacer()
                    Button(action: viewModel.toggleIncognitoMode) {
                        Image(systemName: viewModel.isIncognitoMode ? "eye.slash.fill" : "eye")
                    }
                }
                .padding()
            }
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Text("AlphaSurf").font(.headline)
                }
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button(action: viewModel.showSettings) {
                        Image(systemName: "gear")
                    }
                }
            }
        }
        .sheet(isPresented: $viewModel.isShowingBookmarks) {
            BookmarksView(bookmarks: $viewModel.bookmarks, onSelectBookmark: { url in
                viewModel.loadURL(url)
                viewModel.isShowingBookmarks = false
            })
        }
        .sheet(isPresented: $viewModel.isShowingHistory) {
            HistoryView(history: $viewModel.history, onSelectHistoryItem: { url in
                viewModel.loadURL(url)
                viewModel.isShowingHistory = false
            })
        }
        .sheet(isPresented: $viewModel.isShowingSettings) {
            SettingsView(viewModel: viewModel)
        }
    }
}

struct BrowserWebView: NSViewRepresentable {
    let url: URL?
    @ObservedObject var viewModel: BrowserViewModel
    
    func makeNSView(context: Context) -> WKWebView {
        let config = WKWebViewConfiguration()
        config.userContentController.add(context.coordinator, name: "bookmarkHandler")
        
        let webView = WKWebView(frame: .zero, configuration: config)
        webView.navigationDelegate = context.coordinator
        return webView
    }
    
    func updateNSView(_ nsView: WKWebView, context: Context) {
        if let url = url {
            nsView.load(URLRequest(url: url))
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
    
    private let adBlockList: Set<String> = ["example.com", "ads.com"]
    
    func loadPage() {
        guard var urlString = addressBarText.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) else { return }
        
        if !urlString.hasPrefix("http://") && !urlString.hasPrefix("https://") {
            urlString = "https://\(urlString)"
        }
        
        guard let url = URL(string: urlString) else { return }
        
        if adBlockList.contains(url.host ?? "") {
            print("Blocked ad URL: \(url.absoluteString)")
            return
        }
        
        loadURL(url)
    }
    
    func loadURL(_ url: URL) {
        tabs[selectedTab].url = url
        addressBarText = url.absoluteString
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

struct Tab {
    var url: URL
}

struct BookmarksView: View {
    @Binding var bookmarks: [String]
    let onSelectBookmark: (URL) -> Void
    
    var body: some View {
        NavigationView {
            List(bookmarks, id: \.self) { bookmark in
                Button(action: {
                    if let url = URL(string: bookmark) {
                        onSelectBookmark(url)
                    }
                }) {
                    Text(bookmark)
                }
            }
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Text("Bookmarks").font(.headline)
                }
            }
        }
    }
}

struct HistoryView: View {
    @Binding var history: [String]
    let onSelectHistoryItem: (URL) -> Void
    
    var body: some View {
        NavigationView {
            List(history.reversed(), id: \.self) { item in
                Button(action: {
                    if let url = URL(string: item) {
                        onSelectHistoryItem(url)
                    }
                }) {
                    Text(item)
                }
            }
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Text("History").font(.headline)
                }
            }
        }
    }
}

struct SettingsView: View {
    @ObservedObject var viewModel: BrowserViewModel
    
    var body: some View {
        NavigationView {
            Form {
                Section(header: Text("General")) {
                    Toggle("Incognito Mode", isOn: $viewModel.isIncognitoMode)
                }
                
                Section(header: Text("Safari Extensions")) {
                    Button("Manage Safari Extensions") {
                        let url = URL(fileURLWithPath: "/Applications/Safari.app") // You can modify this path as necessary.
                        NSWorkspace.shared.open(url)
                    }
                }
            }
            .toolbar {
                ToolbarItem(placement: .principal) {
                    Text("Settings").font(.headline)
                }
            }
        }
    }
}

@main
struct AlphaSurfApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}
