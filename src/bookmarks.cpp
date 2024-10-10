#include "bookmarks.h"

std::vector<Bookmark> bookmarks;

void load_bookmarks() {
    try {
        std::ifstream file("bookmarks.json");
        if (file) {
            JsonParser *parser = json_parser_new();
            json_parser_load_from_file(parser, "bookmarks.json", NULL);
            JsonNode *root = json_parser_get_root(parser);
            JsonArray *array = json_node_get_array(root);
            guint length = json_array_get_length(array);

            for (guint i = 0; i < length; i++) {
                JsonNode *node = json_array_get_element(array, i);
                JsonObject *object = json_node_get_object(node);
                Bookmark bookmark;
                bookmark.title = json_object_get_string(object, "title");
                bookmark.url = json_object_get_string(object, "url");
                bookmarks.push_back(bookmark);
            }
            g_object_unref(parser);
        }
    } catch (const std::exception& e) {
        g_printerr("Error loading bookmarks: %s\n", e.what());
    }
}

void save_bookmarks() {
    JsonGenerator *generator = json_generator_new();
    JsonNode *root = json_node_new(JSON_NODE_ARRAY);

    for (const auto& bookmark : bookmarks) {
        JsonNode *bookmark_node = json_node_new(JSON_NODE_OBJECT);
        JsonObject *bookmark_object = json_object_new();
        json_object_set_string(bookmark_object, "title", bookmark.title.c_str());
        json_object_set_string(bookmark_object, "url", bookmark.url.c_str());
        json_node_take_object(bookmark_node, bookmark_object);
        json_array_add_element_element(json_node_get_array(root), bookmark_node);
    }

    json_generator_set_root(generator, root);

    GError *error = NULL;
    gboolean success = json_generator_to_file(generator, "bookmarks.json", &error);

    if (!success) {
        g_printerr("Error saving bookmarks: %s\n", error->message);
        g_error_free(error);
    }

    json_node_unref(root);
    g_object_unref(generator);
}

void add_bookmark(const std::string &title, const std::string &url) {
    bookmarks.push_back({title, url});
    save_bookmarks();
}

void edit_bookmark(const std::string &old_title, const std::string &new_title, const std::string &url) {
    auto it = std::find_if(bookmarks.begin(), bookmarks.end(), [&](const Bookmark &b) {
        return b.title == old_title;
    });
    if (it != bookmarks.end()) {
        it->title = new_title;
        it->url = url;
        save_bookmarks();
    }
}

void delete_bookmark(const std::string &title) {
    bookmarks.erase(std::remove_if(bookmarks.begin(), bookmarks.end(),
                                    [&](const Bookmark &b) { return b.title == title; }),
                    bookmarks.end());
    save_bookmarks();
}
