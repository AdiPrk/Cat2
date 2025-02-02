#include <PCH/pch.h>
#include "Engine.h"

#include "Input/KeyboardController.h"
#include "Graphics/Vulkan/Buffers/Buffer.h"
#include "Graphics/Vulkan/Systems/IndirectRenderer.h"
#include "Graphics/Vulkan/Texture/Texture.h"
#include "Graphics/Vulkan/Texture/ImGuiTexture.h"

#include "glslang/Public/ShaderLang.h"

#include "Input/input.h"

#include "Graphics/Vulkan/Window/FrameRate.h"

#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components.h"

#include "Graphics/Editor/Editor.h"
#include "Assets/FileWatcher/FileWatcher.h"

#include "Networking/Networking.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : m_Window(specs.width, specs.height, specs.name)
        , m_Renderer(std::make_unique<Renderer>(m_Window, device))
        , textureLibrary(device)
        , modelLibrary(device, textureLibrary)
        , fps(specs.fps)
    {
        Logger::Init();
        m_Editor = std::make_unique<Editor>();
        m_Networking = std::make_unique<Networking>(specs.serverAddress, specs.serverPort);
    }

    Engine::~Engine() 
    {
        m_Editor->Exit();
    }

    int Engine::Run(const std::string& sceneName) 
    {
        // Init some stuff
        m_Networking->Init();
        m_Editor->Init();
        m_Renderer->Init();

        SceneManager::SetNextScene(sceneName);
        SceneManager::SwapScenes(); // Init the first scene

        WATCH_DIRECTORY(Texture);

        FrameRateController frameRateController(fps);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!m_Window.shouldClose() && m_Running && !m_Editor->ShouldLoadProject()) {
            Input::Update();
            float dt = frameRateController.WaitForNextFrame();

            // Swap scenes if necessary (also does Init/Exit)
            SceneManager::SwapScenes();

            // Update scenes
            SceneManager::Update(dt);

            // Render scenes
            SceneManager::Render(dt, true);
        }

        m_Renderer->Exit();
        m_Networking->Shutdown();

        if (m_Editor->ShouldLoadProject())
        {
            std::string clientDir = "../Client/";
            std::string exeName = "Dog.exe";
            std::string projName = m_Editor->GetProjectToOpen();

            // Convert to an absolute path
            std::filesystem::path absoluteClientDir = std::filesystem::absolute(clientDir);
            std::filesystem::path exePath = absoluteClientDir / exeName;

            // Change working directory
            if (!SetCurrentDirectory(absoluteClientDir.c_str())) {
                std::cerr << "Failed to change directory! Error: " << GetLastError() << std::endl;
                return 1;
            }

            // Create the command line for Dog.exe
            std::string command = "\"" + exePath.string() + "\" -projectdir " + projName;
            std::cout << "Executing: " << command << std::endl;

            // Convert to wide string for CreateProcessW
            std::wstring wideCommand(command.begin(), command.end());

            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi;

            // Launch Dog.exe in a separate process
            if (CreateProcessW(
                NULL,                      // No application name (using command line)
                &wideCommand[0],           // Command line
                NULL,                      // Process handle not inheritable
                NULL,                      // Thread handle not inheritable
                FALSE,                     // Set handle inheritance to FALSE
                CREATE_NEW_CONSOLE,        // Run in a new console window without inheriting input/output
                NULL,                      // Use parent's environment block
                absoluteClientDir.c_str(), // Set working directory
                &si,                       // Pointer to STARTUPINFO structure
                &pi                        // Pointer to PROCESS_INFORMATION structure
            )) {
                // Close process and thread handles (we don't need them)
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            else {
                std::cerr << "Failed to start Dog.exe! Error: " << GetLastError() << std::endl;
            }

            // Exit the current engine cleanly
            _exit(0);
        }

        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog