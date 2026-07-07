#include "hashtable.h"
#include <functional>

HashTable::HashTable(size_t initial_capacity) : count_(0) {
    buckets_.resize(initial_capacity);
}

size_t HashTable::hash(const std::string& key) const {
    // FNV-1a hash — simple, fast, decent distribution for string keys
    uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return static_cast<size_t>(h) % buckets_.size();
}

void HashTable::set(const std::string& key, const std::string& value) {
    size_t idx = hash(key);
    for (auto& entry : buckets_[idx]) {
        if (entry.key == key) {
            entry.value = value; // update existing
            return;
        }
    }
    buckets_[idx].push_back({key, value});
    count_++;

    if (load_factor() > MAX_LOAD_FACTOR) {
        resize();
    }
}

std::optional<std::string> HashTable::get(const std::string& key) const {
    size_t idx = hash(key);
    for (const auto& entry : buckets_[idx]) {
        if (entry.key == key) {
            return entry.value;
        }
    }
    return std::nullopt;
}

bool HashTable::remove(const std::string& key) {
    size_t idx = hash(key);
    auto& bucket = buckets_[idx];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if (it->key == key) {
            bucket.erase(it);
            count_--;
            return true;
        }
    }
    return false;
}

bool HashTable::contains(const std::string& key) const {
    return get(key).has_value();
}

size_t HashTable::collision_count() const {
    size_t collisions = 0;
    for (const auto& bucket : buckets_) {
        if (bucket.size() > 1) {
            collisions++;
        }
    }
    return collisions;
}

std::vector<std::pair<std::string, std::string>> HashTable::all_entries() const {
    std::vector<std::pair<std::string, std::string>> result;
    result.reserve(count_);
    for (const auto& bucket : buckets_) {
        for (const auto& entry : bucket) {
            result.emplace_back(entry.key, entry.value);
        }
    }
    return result;
}

void HashTable::resize() {
    size_t new_capacity = buckets_.size() * 2;
    std::vector<std::list<Entry>> new_buckets(new_capacity);

    for (const auto& bucket : buckets_) {
        for (const auto& entry : bucket) {
            // Rehash into new bucket array
            uint64_t h = 14695981039346656037ULL;
            for (unsigned char c : entry.key) {
                h ^= c;
                h *= 1099511628211ULL;
            }
            size_t new_idx = static_cast<size_t>(h) % new_capacity;
            new_buckets[new_idx].push_back(entry);
        }
    }

    buckets_ = std::move(new_buckets);
}
