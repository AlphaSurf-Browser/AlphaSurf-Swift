#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

// Basic client class for handling browser events
class SimpleClient : public CefClient, public CefLifeSpanHandler, public CefLoadHandler {
public:
    SimpleClient() {}

    // CefClient methods:
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    // Called after a browser is created
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        browsers_.push_back(browser); // Add the new browser instance to the list
    }

    // Called when a browser is closing
    bool DoClose(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        return false; // Allow the browser to close
    }

    // Remove the browser from the list when closed
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        browsers_.erase(std::remove(browsers_.begin(), browsers_.end(), browser), browsers_.end());
        if (browsers_.empty()) {
            CefQuitMessageLoop(); // Quit the application when all browsers are closed
        }
    }

    // Handle page load errors (e.g., no internet connection)
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override {
        std::string errorHtml = R"(
            <html><body>
            <h1 style="color:red;">No Internet</h1>
            <p>Please check your connection and try again.</p>
            </body></html>
        )";
        frame->LoadString(errorHtml, failedUrl);
    }

    // Update the connection type icon (lock or unlock) based on the security of the page
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override {
        std::string url = browser->GetMainFrame()->GetURL();
        std::string lockStatus = (url.find("https://") == 0) ? "🔒 Secure" : "🔓 Not Secure";
        browser->GetMainFrame()->ExecuteJavaScript(
            "document.getElementById('lock-icon').innerText = '" + lockStatus + "';",
            browser->GetMainFrame()->GetURL(), 0);
    }

private:
    std::vector<CefRefPtr<CefBrowser>> browsers_; // List of open browser tabs
    IMPLEMENT_REFCOUNTING(SimpleClient);
};

// Load start page with DuckDuckGo search and a clock
std::string GetStartPageHTML() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    background-color: white;
                    display: flex;
                    flex-direction: column;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    margin: 0;
                    color: black;
                    box-shadow: 0px 4px 15px rgba(0, 0, 0, 0.1);
                }
                #searchbar {
                    padding: 10px;
                    width: 300px;
                    border: 1px solid #ddd;
                    border-radius: 5px;
                    margin-bottom: 20px;
                    box-shadow: 0px 2px 5px rgba(0, 0, 0, 0.2);
                }
                #clock {
                    font-size: 2em;
                    margin-top: 20px;
                }
            </style>
        </head>
        <body>
            <div id="content">
                <form action="https://duckduckgo.com/" method="get">
                    <input id="searchbar" type="text" name="q" placeholder="Search DuckDuckGo...">
                    <input type="submit" value="Search">
                </form>
                <div id="clock">Time: )" + std::string(buffer) + R"(</div>
            </div>
        </body>
        </html>
    )";
}

// Load custom URL scheme handler (e.g., alpha://start, alpha://settings)
class AlphaSchemeHandler : public CefResourceHandler {
public:
    bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override {
        std::string url = request->GetURL();
        if (url.find("alpha://start") != std::string::npos) {
            data_ = GetStartPageHTML(); // Load start page HTML
        } else if (url.find("alpha://settings") != std::string::npos) {
            data_ = R"(
                <html><body>
                <h1>Settings Page</h1>
                <p>Here you can customize your browser settings.</p>
                </body></html>
            )";
        }
        callback->Continue();
        return true;
    }

    void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override {
        response->SetMimeType("text/html");
        response->SetStatus(200);
        response_length = data_.size();
    }

    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override {
        size_t size = std::min(data_.size(), static_cast<size_t>(bytes_to_read));
        memcpy(data_out, data_.c_str(), size);
        bytes_read = size;
        return size > 0;
    }

private:
    std::string data_;
    IMPLEMENT_REFCOUNTING(AlphaSchemeHandler);
};

// Register the alpha:// scheme
void RegisterAlphaSchemeHandlerFactory() {
    CefRegisterSchemeHandlerFactory("alpha", "", new AlphaSchemeHandler());
}

int main(int argc, char* argv[]) {
    // CEF initialization
    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefApp> app;

    // Initialize CEF settings
    CefSettings settings;
    CefInitialize(main_args, settings, app, nullptr);

    // Register the custom scheme for alpha://
    RegisterAlphaSchemeHandlerFactory();

    // Create browser window
    CefWindowInfo window_info;
    window_info.SetAsPopup(nullptr, "Alpha Browser");

    CefBrowserSettings browser_settings;
    CefRefPtr<SimpleClient> client = new SimpleClient();

    // Load the start page (alpha://start)
    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
        window_info, client, "alpha://start", browser_settings, nullptr, nullptr);

    // Run CEF message loop
    CefRunMessageLoop();

    // Shutdown CEF
    CefShutdown();
    return 0;
}
