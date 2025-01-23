#include <PCH/pch.h>
#include "Engine.h"

int main() {
    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 60; // <- fps is unused

    Dog::Engine& Engine = Dog::Engine::Create(specs);

    Engine.Run("namae");

    return EXIT_SUCCESS;
}