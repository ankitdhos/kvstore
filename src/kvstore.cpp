#include "kvstore.h"
#include <fstream>
#include <sstream>
#include <iostream>

KVStore::KVStore(const std::string& snapshot_path)
    : table_(64), snapshot_path_(snapshot_path) {
    load_snapshot();
}

void KVStore::set(const std::string& key, const std::string& value, int ttl_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    table_.set(key, value);
    if (ttl_seconds > 0) {
        ttl_.schedule(key, ttl_seconds);
    }
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return table_.get(key);
}

bool KVStore::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return table_.remove(key);
}

void KVStore::sweep_expired() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto expired = ttl_.pop_expired();
    for (const auto& key : expired) {
        table_.remove(key);
    }
}

void KVStore::save_snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream out(snapshot_path_, std::ios::trunc);
    if (!out) {
        std::cerr << "Failed to open snapshot file for writing: " << snapshot_path_ << "\n";
        return;
    }
    for (const auto& [key, value] : table_.all_entries()) {
        // Simple line-based format: key\tvalue
        // (Not handling embedded tabs/newlines — fine for a demo/benchmark project)
        out << key << '\t' << value << '\n';
    }
}

void KVStore::load_snapshot() {
    std::ifstream in(snapshot_path_);
    if (!in) {
        return; // no snapshot yet, start empty
    }
    std::string line;
    while (std::getline(in, line)) {
        auto tab_pos = line.find('\t');
        if (tab_pos == std::string::npos) continue;
        std::string key = line.substr(0, tab_pos);
        std::string value = line.substr(tab_pos + 1);
        table_.set(key, value);
    }
}
