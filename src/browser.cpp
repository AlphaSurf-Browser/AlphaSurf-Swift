#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "ui.h"
#include "bookmarks.h"
#include "history.h"

// Global variables
std::vector<Bookmark> bookmarks;
std::vector<std::string> history;
GtkWidget *main_window;
GtkNotebook *notebook;

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    show_splash_screen();
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    initialize_ui();
    load_bookmarks();
    gtk_widget_show_all(main_window);
    gtk_main();
    return 0;
}
