#include <PCH/pch.h>
#include "Engine.h"
#include "Networking/Networking.h"

int main(int argc, char* argv[])
{
    // Check command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "dev") {
            printf("Cannot run launcher directly through Visual Studio. Run the exe from the output directory");
            return -1;
        }
    }

    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 120;
    specs.serverAddress = "localhost";
    specs.serverPort = 7777;

    Dog::Engine& Engine = Dog::Engine::Create(specs);

    return Engine.Run("namae");
}