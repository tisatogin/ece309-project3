
#include "tools/workspace.h"

#include <string>

namespace fs = std::filesystem;

bool Workspace::resolve(
    std::string_view user_path,
    fs::path* out,
    std::string* reason) const {

    // Step 1: Reject empty paths.
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }

    // Step 2: Reject embedded null characters.
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "path contains NUL byte";
        return false;
    }

    // Step 3: Convert the string to a filesystem path.
    fs::path p{std::string(user_path)};

    // Absolute paths are not allowed.
    if (p.is_absolute() || p.has_root_path()) {
        *reason = "absolute paths are not allowed";
        return false;
    }

    // Step 4: Normalize the path.
    fs::path n = p.lexically_normal();

    // Step 5: Reject paths that escape the workspace.
    if (!n.empty() && *n.begin() == "..") {
        *reason = "path escapes workspace";
        return false;
    }

    // Step 6: Construct the final path.
    if (n == ".") {
        *out = root();
    } else {
        *out = root() / n;
    }

    return true;
}
