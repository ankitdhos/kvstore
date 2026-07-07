#include "ttl_manager.h"

void TTLManager::schedule(const std::string& key, int ttl_seconds) {
    TimePoint expiry = Clock::now() + std::chrono::seconds(ttl_seconds);
    heap_.push({expiry, key});
}

std::vector<std::string> TTLManager::pop_expired() {
    std::vector<std::string> expired;
    TimePoint now = Clock::now();

    while (!heap_.empty() && heap_.top().expiry <= now) {
        expired.push_back(heap_.top().key);
        heap_.pop();
    }
    return expired;
}
