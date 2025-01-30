#include <PCH/pch.h>
#include "Engine.h"
#include "Networking/Networking.h"

int main() 
{
    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 120;

    Dog::Engine& Engine = Dog::Engine::Create(specs);

    return Engine.Run("namae");
}