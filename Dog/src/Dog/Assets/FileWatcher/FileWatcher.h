#pragma once

namespace Dog {

#define WATCH_DIRECTORY(path) \
	FileWatcher<Event::path##FileCreated, Event::path##FileModified, Event::path##FileDeleted> path##Watcher(Assets::path##sPath)

#define STOP_WATCHING(path) \
	path##Watcher.stop();

    // File Watchers look at their respective directories for changes
    // And publish events when they occur.
    // They work on a seperate thread to avoid blocking the main thread.
    template <typename OnCreate, typename OnModify, typename OnDelete>
    class FileWatcher {
    public:
        FileWatcher(const std::string& path)
            : mDirectory(normalize_path(path)), mRunning(true) {
            for (const auto& entry : std::filesystem::directory_iterator(mDirectory)) {
                if (std::filesystem::is_regular_file(entry)) {
                    std::string normalized_path = normalize_path(entry.path().string());

                    mPaths[normalized_path] = std::filesystem::last_write_time(entry);

                    PUBLISH_EVENT(OnCreate, normalized_path);
                }
            }

            start();
        }

        ~FileWatcher() {
            stop();
        }

        void start() {
            mWatchThread = std::thread([this]() { run(); });
        }

        void stop() {
            mRunning = false;
            if (mWatchThread.joinable()) {
                mWatchThread.join();
            }
        }

        static std::string normalize_path(const std::string& path) {
            std::string normalized_path = path;
            std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');
            return normalized_path;
        }

    private:
        std::string mDirectory;
        std::unordered_map<std::string, std::filesystem::file_time_type> mPaths;
        std::thread mWatchThread;
        bool mRunning;

        void run() {
            while (mRunning) {
                std::this_thread::sleep_for(std::chrono::milliseconds(33)); //~30fps
                
                auto it = mPaths.begin();
                while (it != mPaths.end()) {
                    auto normalized_path = normalize_path(it->first);
                    if (!std::filesystem::exists(normalized_path) || !std::filesystem::is_regular_file(normalized_path)) {
                        PUBLISH_EVENT(OnDelete, normalized_path);
                        it = mPaths.erase(it);
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

                for (const auto& entry : std::filesystem::directory_iterator(mDirectory)) {
                    if (std::filesystem::is_regular_file(entry)) {
                        auto path = normalize_path(entry.path().string());
                        if (mPaths.find(path) == mPaths.end()) {
                            PUBLISH_EVENT(OnCreate, path);
                            mPaths[path] = std::filesystem::last_write_time(entry);
                        }
                    }
                }
            }
        }
    };

}
