#include "history.h"

std::vector<std::string> history;

void add_to_history(const std::string &url) {
    history.push_back(url);
}

void display_history_ui() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "History",
        GTK_WINDOW(main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL);

    GtkWidget *list = gtk_list_box_new();
    for (const auto& url : history) {
        GtkWidget *row = gtk_label_new(url.c_str());
        gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), list, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
