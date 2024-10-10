#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

void initialize_ui();
void display_bookmarks_ui();
void display_settings_ui();
void show_splash_screen();
void create_new_tab(const std::string &html_content);
void on_new_tab_button_clicked(GtkWidget *widget, gpointer data);
void on_bookmarks_button_clicked(GtkWidget *widget, gpointer data);
void on_settings_button_clicked(GtkWidget *widget, gpointer data);
void on_history_button_clicked(GtkWidget *widget, gpointer data);
void handle_alpha_scheme(const std::string &url);

#endif  // UI_H
