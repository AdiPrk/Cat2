#include <PCH/pch.h>
#include "Engine.h"

int main() {
    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 60; // <- fps is unused

    Dog::Engine& Engine = Dog::Engine::Create(specs);

    try {
        Engine.Run("namae");
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cin.get(); // Wait here
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}