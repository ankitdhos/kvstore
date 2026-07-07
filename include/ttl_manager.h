#ifndef TTL_MANAGER_H
#define TTL_MANAGER_H

#include <string>
#include <queue>
#include <vector>
#include <chrono>

// Tracks key expiry times using a min-heap so the soonest-to-expire
// key is always at the top, avoiding a full table scan on each check.
class TTLManager {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    // Schedule a key to expire after `ttl_seconds` from now.
    void schedule(const std::string& key, int ttl_seconds);

    // Returns all keys whose expiry time has passed (and removes them
    // from internal tracking). Call this periodically to sweep expired keys.
    std::vector<std::string> pop_expired();

    size_t pending_count() const { return heap_.size(); }

private:
    struct Item {
        TimePoint expiry;
        std::string key;

        // For min-heap: smallest expiry time should have highest priority
        bool operator>(const Item& other) const {
            return expiry > other.expiry;
        }
    };

    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> heap_;
};

#endif // TTL_MANAGER_H
