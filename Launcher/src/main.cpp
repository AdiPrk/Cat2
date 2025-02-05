#include <PCH/pch.h>
#include "Engine.h"
#include "Networking/Networking.h"
#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = __argc;
    char** argv = __argv;

    // Check for "dev" mode
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "dev") {
            MessageBoxA(nullptr, "Cannot run launcher directly through Visual Studio. Run the exe from the output directory.", "Error", MB_OK | MB_ICONERROR);
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