#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <json-glib/json-glib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

struct Bookmark {
    std::string title;
    std::string url;
};

void load_bookmarks();
void save_bookmarks();
void add_bookmark(const std::string &title, const std::string &url);
void edit_bookmark(const std::string &old_title, const std::string &new_title, const std::string &url);
void delete_bookmark(const std::string &title);

#endif  // BOOKMARKS_H
