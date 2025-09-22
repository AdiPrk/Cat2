#include <PCH/pch.h>
#include "Engine.h"
#include "Utils/Utils.h"

int main(int argc, char* argv[])
{   
    Dog::ValidateStartingDirectory(argc, argv);

    Dog::EngineSpec specs;
    specs.name = "Woof";
    specs.width = 1280;
    specs.height = 720;
    specs.fps = 120;
    specs.serverAddress = "localhost";
    specs.serverPort = 7777;

    Dog::Engine Engine(specs);
    return Engine.Run("scene");
}