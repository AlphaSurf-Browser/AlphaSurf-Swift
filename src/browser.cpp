#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

// Function Prototypes
void on_uri_requested(WebKitWebView* web_view, const gchar* uri);
void on_new_tab_button_clicked(GtkNotebook* notebook, GtkEntry* url_entry);
void on_refresh_button_clicked(GtkWidget* widget, gpointer data);
void on_home_button_clicked(GtkWidget* widget, gpointer data);
void on_settings_button_clicked(GtkWidget* widget, gpointer data);
void perform_search(GtkEntry* entry, gpointer data);

// Initialize WebView
WebKitWebView* create_web_view(const gchar* uri) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    g_signal_connect(web_view, "decide-policy", G_CALLBACK(on_uri_requested), NULL);
    webkit_web_view_load_uri(web_view, uri);
    return web_view;
}

// Function for handling URI requests
void on_uri_requested(WebKitWebView* web_view, const gchar* uri) {
    webkit_web_view_load_uri(web_view, uri);
}

// Function to create a new tab
void on_new_tab_button_clicked(GtkNotebook* notebook, GtkEntry* url_entry) {
    const gchar* url = gtk_entry_get_text(url_entry);
    if (g_strcmp0(url, "") == 0) {
        url = "alpha://start"; // Default start page
    }
    
    WebKitWebView* new_web_view = create_web_view(url);
    GtkWidget* tab_label = gtk_label_new(url);
    
    // Add new tab
    gtk_notebook_append_page(notebook, GTK_WIDGET(new_web_view), tab_label);
    gtk_widget_show(GTK_WIDGET(new_web_view));
    
    // Set URL entry
    g_signal_connect(new_web_view, "load-changed", G_CALLBACK(on_uri_requested), url_entry);
}

// Function for refreshing the current page
void on_refresh_button_clicked(GtkWidget* widget, gpointer data) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    webkit_web_view_reload(web_view);
}

// Function for going to home page
void on_home_button_clicked(GtkWidget* widget, gpointer data) {
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    webkit_web_view_load_uri(web_view, "https://www.startpage.com");
}

// Function for opening settings
void on_settings_button_clicked(GtkWidget* widget, gpointer data) {
    // Open settings window (to be implemented)
    g_print("Settings opened.\n");
}

// Perform search on entry activate
void perform_search(GtkEntry* entry, gpointer data) {
    const gchar* url = gtk_entry_get_text(entry);
    WebKitWebView* web_view = WEBKIT_WEB_VIEW(data);
    if (g_str_has_prefix(url, "http://") || g_str_has_prefix(url, "https://")) {
        webkit_web_view_load_uri(web_view, url);
    } else {
        gchar* search_uri = g_strdup_printf("https://www.startpage.com/search?q=%s", url);
        webkit_web_view_load_uri(web_view, search_uri);
        g_free(search_uri);
    }
}

// Create toolbar
GtkWidget* create_toolbar(GtkNotebook* notebook, GtkEntry* url_entry) {
    GtkWidget* toolbar = gtk_toolbar_new();
    
    GtkToolItem* refresh_button = gtk_tool_button_new(NULL, "Refresh");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), refresh_button, -1);
    
    GtkToolItem* home_button = gtk_tool_button_new(NULL, "Home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), home_button, -1);
    
    GtkToolItem* settings_button = gtk_tool_button_new(NULL, "Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_settings_button_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), settings_button, -1);
    
    GtkToolItem* new_tab_button = gtk_tool_button_new(NULL, "New Tab");
    g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab_button_clicked), notebook, url_entry);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_tab_button, -1);
    
    // URL Entry
    GtkToolItem* entry_item = gtk_tool_item_new();
    gtk_tool_item_set_expand(entry_item, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entry_item, -1);
    
    // Create URL entry field
    gtk_tool_item_set_homogeneous(entry_item, TRUE);
    gtk_widget_show(entry_item);
    
    gtk_container_add(GTK_CONTAINER(entry_item), url_entry);
    
    // Connect the entry's activate signal
    g_signal_connect(url_entry, "activate", G_CALLBACK(perform_search), notebook);
    
    return toolbar;
}

// Main function to setup UI
int main(int argc, char** argv) {
    gtk_init(&argc, &argv);
    
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
    
    GtkEntry* url_entry = GTK_ENTRY(gtk_entry_new());
    GtkWidget* toolbar = create_toolbar(notebook, url_entry);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(notebook), TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(window);
    gtk_widget_show(url_entry);
    
    gtk_main();
    return 0;
}
