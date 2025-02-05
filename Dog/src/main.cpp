#include <PCH/pch.h>
#include "Engine.h"
#include "Networking/Networking.h"

void PrintAndExit(const std::string& message) {
    std::cerr << message << "\nPress 'Enter' to exit.\n";
    std::cin.get();
    exit(EXIT_FAILURE);
}

std::string GetProjectDirectory(int argc, char* argv[]) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "-projectdir") {
            return argv[i + 1]; // Return the next argument as the project directory
        }
    }
    return ""; // Return empty string if not found
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        PrintAndExit("No project directory provided. Use the launcher to run the engine.");
    }

    std::string projectDir = GetProjectDirectory(argc, argv);
    if (projectDir.empty()) {
        PrintAndExit("Invalid or missing '-projectdir' argument.");
    }

    std::cout << "Project Directory: " << projectDir << '\n';

    if (projectDir != "Dev") {
        FreeConsole();
        if (!SetCurrentDirectoryA(projectDir.c_str())) {
            PrintAndExit("Failed to set project directory.");
        }
    }

    char cwd[_MAX_PATH];
    if (_getcwd(cwd, _MAX_PATH)) {
        std::cout << "Current working directory is: " << cwd << '\n';
    }
    else {
        PrintAndExit("Failed to get current working directory.");
    }

    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 120;
    //specs.serverAddress = "localhost";
    specs.serverPort = 7777;

    Dog::Engine& Engine = Dog::Engine::Create(specs);

    return Engine.Run("namae");
}