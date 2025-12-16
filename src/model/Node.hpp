#ifndef ARCANE_LOCK_NODE_HPP
#define ARCANE_LOCK_NODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace ArcaneLock {

// Forward declarations
struct Folder;
struct Entry;

// A node in the tree can be either a Folder or an Entry.
using Node = std::variant<Folder, Entry>;

// Represents a password entry
struct Entry {
    std::string title;
    std::string username;
    std::string password;
    std::string url;
    std::string notes;
};

// Represents a folder that can contain other nodes
struct Folder {
    std::string name;
    std::vector<std::unique_ptr<Node>> children;
    bool is_open = true; // Added for UI state
};

} // namespace ArcaneLock

#endif // ARCANE_LOCK_NODE_HPP
