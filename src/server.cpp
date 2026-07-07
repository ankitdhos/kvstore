#include "kvstore.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>

// Parses and executes a single command line like:
//   SET key value [ttl_seconds]
//   GET key
//   DEL key
// Returns the response string to send back to the client.
std::string handle_command(KVStore& store, const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "SET") {
        std::string key, value;
        int ttl = -1;
        iss >> key >> value;
        if (iss >> ttl) {
            store.set(key, value, ttl);
        } else {
            store.set(key, value);
        }
        return "OK\n";
    } else if (cmd == "GET") {
        std::string key;
        iss >> key;
        auto val = store.get(key);
        return val.has_value() ? (*val + "\n") : "NOT_FOUND\n";
    } else if (cmd == "DEL") {
        std::string key;
        iss >> key;
        bool removed = store.remove(key);
        return removed ? "DELETED\n" : "NOT_FOUND\n";
    } else if (cmd == "STATS") {
        std::ostringstream oss;
        oss << "size=" << store.size()
            << " capacity=" << store.capacity()
            << " load_factor=" << store.load_factor()
            << " collision_buckets=" << store.collision_count() << "\n";
        return oss.str();
    } else {
        return "ERROR unknown command\n";
    }
}

static KVStore* g_store_for_signal = nullptr;

void handle_shutdown(int) {
    if (g_store_for_signal) {
        g_store_for_signal->save_snapshot();
        std::cout << "\nSnapshot saved on shutdown.\n";
    }
    std::exit(0);
}

int main(int argc, char* argv[]) {
    int port = 6380; // avoid clashing with real redis on 6379
    if (argc > 1) port = std::atoi(argv[1]);

    KVStore store;
    g_store_for_signal = &store;
    std::signal(SIGINT, handle_shutdown);
    std::signal(SIGTERM, handle_shutdown);

    // Background thread to sweep expired keys AND periodically snapshot to disk
    std::thread sweeper([&store]() {
        int tick = 0;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            store.sweep_expired();
            tick++;
            if (tick % 5 == 0) { // snapshot every 5 seconds
                store.save_snapshot();
            }
        }
    });
    sweeper.detach();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port << "\n";
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "KVStore server listening on port " << port << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;

        char buffer[4096];
        while (true) {
            ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';

            std::string line(buffer);
            // Strip trailing newline/carriage return
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                line.pop_back();
            }
            if (line.empty()) continue;

            std::string response = handle_command(store, line);
            write(client_fd, response.c_str(), response.size());
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
