#include <PCH/pch.h>
#include "Engine.h"
#include "Networking/Networking.h"

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        // no project dir provided, exit
        printf("No project directory provided. Use the launcher to run the engine. Press 'Enter' to exit.\n");
        std::cin.get();
        return -1;
    }

    // Parse command-line arguments
    std::string projectDir;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-projectdir" && i + 1 < argc) {
            projectDir = argv[i + 1]; // Assign the next argument as the directory
            i++; // Skip the next argument since it's consumed
        }
    }

    if (projectDir != "Dev") {
        std::cout << "Project Directory: " << projectDir << std::endl;
        SetCurrentDirectoryA(projectDir.c_str());
    }
    else {
        std::cout << "Not dev - Project Directory: " << projectDir << std::endl;
    }

    // pritn full path of current dir
    // char full[_MAX_PATH];
    // if (_fullpath(full, projectDir.c_str(), _MAX_PATH) != NULL)
    //     printf("Full path is: %s\n", full);
    // else
    //     printf("Invalid path\n");

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