#pragma once

namespace Dog {

#define WATCH_DIRECTORY(path) \
	FileWatcher<Event::path##FileCreated, Event::path##FileModified, Event::path##FileDeleted> path##Watcher(Assets::path##sPath);

#define STOP_WATCHING(path) \
	path##Watcher.stop();

    // File Watchers look at their respective directories for changes
    // And publish events when they occur.
    // They work on a seperate thread to avoid blocking the main thread.
    template <typename OnCreate, typename OnModify, typename OnDelete>
    class FileWatcher {
    public:
        FileWatcher(const std::string& path)
            : directory(normalize_path(path)), running(true) {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (std::filesystem::is_regular_file(entry)) {
                    std::string normalized_path = normalize_path(entry.path().string());

                    paths[normalized_path] = std::filesystem::last_write_time(entry);

                    PUBLISH_EVENT(OnCreate, normalized_path);
                }
            }

            start();
        }

        ~FileWatcher() {
            stop();
        }

        void start() {
            watchThread = std::thread([this]() { run(); });
        }

        void stop() {
            running = false;
            if (watchThread.joinable()) {
                watchThread.join();
            }
        }

        static std::string normalize_path(const std::string& path) {
            std::string normalized_path = path;
            std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');
            return normalized_path;
        }

    private:
        std::string directory;
        std::unordered_map<std::string, std::filesystem::file_time_type> paths;
        std::thread watchThread;
        bool running;

        void run() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(33)); //~30fps
                
                auto it = paths.begin();
                while (it != paths.end()) {
                    auto normalized_path = normalize_path(it->first);
                    if (!std::filesystem::exists(normalized_path) || !std::filesystem::is_regular_file(normalized_path)) {
                        PUBLISH_EVENT(OnDelete, normalized_path);
                        it = paths.erase(it);
                    }
                    else {
                        auto current_file_last_write_time = std::filesystem::last_write_time(normalized_path);
                        if (it->second != current_file_last_write_time) {
                            PUBLISH_EVENT(OnModify, normalized_path);
                            it->second = current_file_last_write_time;
                        }
                        ++it;
                    }
                }

                for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                    if (std::filesystem::is_regular_file(entry)) {
                        auto path = normalize_path(entry.path().string());
                        if (paths.find(path) == paths.end()) {
                            PUBLISH_EVENT(OnCreate, path);
                            paths[path] = std::filesystem::last_write_time(entry);
                        }
                    }
                }
            }
        }
    };

}
