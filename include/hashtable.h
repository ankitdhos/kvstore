#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <list>
#include <optional>
#include <cstdint>

// A hash table built from scratch (no std::unordered_map) using
// separate chaining for collision resolution and dynamic resizing.
class HashTable {
public:
    explicit HashTable(size_t initial_capacity = 16);

    // Core operations
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);
    bool contains(const std::string& key) const;

    // Introspection (useful for benchmarking / debugging)
    size_t size() const { return count_; }
    size_t capacity() const { return buckets_.size(); }
    double load_factor() const { return static_cast<double>(count_) / buckets_.size(); }

    // Returns the number of buckets that have 2+ entries (collision indicator)
    size_t collision_count() const;

    // Iterate all key-value pairs (needed for persistence snapshotting)
    std::vector<std::pair<std::string, std::string>> all_entries() const;

private:
    struct Entry {
        std::string key;
        std::string value;
    };

    std::vector<std::list<Entry>> buckets_;
    size_t count_;

    static constexpr double MAX_LOAD_FACTOR = 0.75;

    size_t hash(const std::string& key) const;
    void resize();
};

#endif // HASHTABLE_H
