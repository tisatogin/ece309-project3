
#include "tools/tool_registry.h"

#include <algorithm>
#include <cstdint>
#include <utility>

// Start with 16 empty buckets.
ToolRegistry::ToolRegistry() : buckets_(16) {}

// unique_ptr automatically destroys owned tools.
ToolRegistry::~ToolRegistry() = default;

// FNV-1a string hash function.
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;

    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }

    return static_cast<std::size_t>(h);
}

// Add a new tool to the registry.
bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) {
        return false;
    }

    std::string name = tool->name();

    // Reject duplicates, keeping the original.
    if (find(name) != nullptr) {
        return false;
    }

    // Resize before exceeding a load factor of 0.75.
    if ((count_ + 1) * 4 > buckets_.size() * 3) {
        rehash(buckets_.size() * 2);
    }

    std::size_t index = hash(name) % buckets_.size();

    buckets_[index].push_back(std::move(tool));
    ++count_;

    return true;
}

// Find a tool by name.
Tool* ToolRegistry::find(std::string_view name) const {
    std::size_t index = hash(name) % buckets_.size();

    for (const auto& tool : buckets_[index]) {
        if (tool->name() == name) {
            return tool.get();
        }
    }

    return nullptr;
}

// Return the number of tools.
std::size_t ToolRegistry::size() const noexcept {
    return count_;
}

// Visit every registered tool exactly once.
void ToolRegistry::for_each(
    const std::function<void(const Tool&)>& fn) const {

    for (const auto& bucket : buckets_) {
        for (const auto& tool : bucket) {
            fn(*tool);
        }
    }
}

// Return the number of buckets.
std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

// Calculate the current load factor.
double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(count_) /
           static_cast<double>(buckets_.size());
}

// Return the length of the longest chain.
std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;

    for (const auto& bucket : buckets_) {
        longest = std::max(longest, bucket.size());
    }

    return longest;
}

// Resize and redistribute existing tools.
void ToolRegistry::rehash(std::size_t new_bucket_count) {
    std::vector<std::vector<std::unique_ptr<Tool>>>
        new_buckets(new_bucket_count);

    for (auto& bucket : buckets_) {
        for (auto& tool : bucket) {
            std::size_t index =
                hash(tool->name()) % new_bucket_count;

            new_buckets[index].push_back(std::move(tool));
        }
    }

    buckets_ = std::move(new_buckets);
}
