#ifndef KVSTORE_H
#define KVSTORE_H

#include "hashtable.h"
#include "ttl_manager.h"
#include <string>
#include <optional>
#include <mutex>

class KVStore {
public:
    explicit KVStore(const std::string& snapshot_path = "data/snapshot.db");

    void set(const std::string& key, const std::string& value, int ttl_seconds = -1);
    std::optional<std::string> get(const std::string& key);
    bool remove(const std::string& key);

    // Removes any keys whose TTL has expired. Call periodically.
    void sweep_expired();

    // Persistence
    void save_snapshot() const;
    void load_snapshot();

    // Stats for benchmarking
    size_t size() const { return table_.size(); }
    size_t capacity() const { return table_.capacity(); }
    double load_factor() const { return table_.load_factor(); }
    size_t collision_count() const { return table_.collision_count(); }

private:
    HashTable table_;
    TTLManager ttl_;
    std::string snapshot_path_;
    mutable std::mutex mutex_;
};

#endif // KVSTORE_H
