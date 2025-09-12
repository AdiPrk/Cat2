#include <PCH/pch.h>
//
//#include "TextEditor.h"
//#include "Graphics/Vulkan/Core/SwapChain.h"
//#include "Engine.h"
//
//#include <Ultralight/Ultralight.h>
//#include <AppCore/AppCore.h>
//#include <AppCore/App.h>
//#include <AppCore/Window.h>
//#include <AppCore/Overlay.h>
//#include "Graphics/Vulkan/Texture/Texture.h"
//#include "Graphics/Vulkan/Core/Device.h"
//#include "Input/input.h"
//#include "Input/ultralightKeyMap.h"
//
//
//inline std::string ToUTF8(const ultralight::String& str) {
//    ultralight::String8 utf8 = str.utf8();
//    return std::string(utf8.data(), utf8.length());
//}
//
//inline const char* Stringify(ultralight::MessageSource source) {
//    switch (source) {
//    case ultralight::kMessageSource_XML: return "XML";
//    case ultralight::kMessageSource_JS: return "JS";
//    case ultralight::kMessageSource_Network: return "Network";
//    case ultralight::kMessageSource_ConsoleAPI: return "ConsoleAPI";
//    case ultralight::kMessageSource_Storage: return "Storage";
//    case ultralight::kMessageSource_AppCache: return "AppCache";
//    case ultralight::kMessageSource_Rendering: return "Rendering";
//    case ultralight::kMessageSource_CSS: return "CSS";
//    case ultralight::kMessageSource_Security: return "Security";
//    case ultralight::kMessageSource_ContentBlocker: return "ContentBlocker";
//    case ultralight::kMessageSource_Other: return "Other";
//    default: return "";
//    }
//}
//
//inline const char* Stringify(ultralight::MessageLevel level) {
//    switch (level) {
//    case ultralight::kMessageLevel_Log: return "Log";
//    case ultralight::kMessageLevel_Warning: return "Warning";
//    case ultralight::kMessageLevel_Error: return "Error";
//    case ultralight::kMessageLevel_Debug: return "Debug";
//    case ultralight::kMessageLevel_Info: return "Info";
//    default: return "";
//    }
//}
//
//namespace Dog
//{
//    void TextEditor::Init() {
//        ultralight::Config config;
//        config.resource_path_prefix = "assets/resources/";
//        
//        ultralight::Platform::instance().set_config(config);
//    }
//
//    void TextEditor::InitPlatform() {
//        /// Use the OS's native font loader
//        ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());
//
//        /// Use the OS's native file loader, with a base directory of "."
//        /// All file:/// URLs will load relative to this base directory.
//        ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem("."));
//
//        /// Use the default logger
//        ultralight::Platform::instance().set_logger(ultralight::GetDefaultLogger("ultralight.log"));
//    }
//
//    void TextEditor::CreateRenderer() {
//        /// Create our Renderer (call this only once per application).
//        m_Renderer = ultralight::Renderer::Create();
//    }
//
//    // Function to load HTML from a file and display it
//    void TextEditor::loadHTMLFromFile(ultralight::RefPtr<ultralight::View> view, const std::string& filePath) {
//        std::ifstream file(filePath);
//        if (!file) {
//            std::cerr << "Error: Could not open " << filePath << std::endl;
//            return;
//        }
//
//        std::stringstream buffer;
//        buffer << file.rdbuf();
//        std::string htmlContent = buffer.str();
//
//        view->LoadHTML(htmlContent.c_str());
//    }
//
//    void TextEditor::CreateView() {
//        /// Configure our View, make sure it uses the CPU renderer by
//        /// disabling acceleration.
//        ultralight::ViewConfig view_config;
//        view_config.is_accelerated = false;
//
//        /// Create an HTML view
//        m_View = m_Renderer->CreateView(400, 400, view_config, nullptr);
//
//        m_View->set_view_listener(&m_Logger);
//
//        /// Load HTML asynchronously into the View.
//        loadHTMLFromFile(m_View, "assets/html/index.html");
//
//        /// Get the pixel-buffer Surface for a View.
//        ultralight::Surface* surface = m_View->surface();
//
//        /// Cast it to a BitmapSurface.
//        ultralight::BitmapSurface* bitmap_surface = (ultralight::BitmapSurface*)surface;
//
//        /// Get the underlying bitmap.
//        ultralight::RefPtr<ultralight::Bitmap> bitmap = bitmap_surface->bitmap();
//    }
//
//    TextEditor::TextEditor(Device& device)
//    {
//        for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
//        {
//            m_Textures.push_back(std::make_unique<Texture>(device));
//        }
//
//        InitPlatform();
//        Init();
//        CreateRenderer();
//        CreateView();
//    }
//
//    TextEditor::~TextEditor()
//    {
//    }
//
//    void TextEditor::Update()
//    {
//        static bool firstFrame = true;
//
//        // Mouse events;
//        int mouseX = Input::getMouseSceneX();
//        int mouseY = Input::getMouseSceneY();
//        {
//            ultralight::MouseEvent mouseEvent;
//            mouseEvent.type = ultralight::MouseEvent::kType_MouseMoved;
//            mouseEvent.x = mouseX;
//            mouseEvent.y = mouseY;
//            mouseEvent.button = ultralight::MouseEvent::kButton_None;
//            m_View->FireMouseEvent(mouseEvent);
//        }
//
//        static bool lastLeftClick = false;
//        static bool lastRightClick = false;
//
//        bool leftClick = Input::isMouseDown(Mouse::LEFT);
//        bool rightClick = Input::isMouseDown(Mouse::RIGHT);
//
//        ultralight::MouseEvent mouseMoveEvent;
//        mouseMoveEvent.type = ultralight::MouseEvent::kType_MouseMoved;
//        mouseMoveEvent.x = mouseX;
//        mouseMoveEvent.y = mouseY;
//        mouseMoveEvent.button = ultralight::MouseEvent::kButton_None; // No button for move
//        m_View->FireMouseEvent(mouseMoveEvent);
//
//        if (leftClick && !lastLeftClick) {
//            ultralight::MouseEvent mouseEvent;
//            mouseEvent.type = ultralight::MouseEvent::kType_MouseDown;
//            mouseEvent.button = ultralight::MouseEvent::kButton_Left;
//            mouseEvent.x = mouseX;
//            mouseEvent.y = mouseY;
//            m_View->FireMouseEvent(mouseEvent);
//        }
//        else if (!leftClick && lastLeftClick) {
//            ultralight::MouseEvent mouseEvent;
//            mouseEvent.type = ultralight::MouseEvent::kType_MouseUp;
//            mouseEvent.button = ultralight::MouseEvent::kButton_Left;
//            mouseEvent.x = mouseX;
//            mouseEvent.y = mouseY;
//            m_View->FireMouseEvent(mouseEvent);
//        }
//
//        // Handle right mouse button events
//        if (rightClick && !lastRightClick) {
//            ultralight::MouseEvent mouseEvent;
//            mouseEvent.type = ultralight::MouseEvent::kType_MouseDown;
//            mouseEvent.button = ultralight::MouseEvent::kButton_Right;
//            mouseEvent.x = mouseX;
//            mouseEvent.y = mouseY;
//            m_View->FireMouseEvent(mouseEvent);
//        }
//        else if (!rightClick && lastRightClick) {
//            ultralight::MouseEvent mouseEvent;
//            mouseEvent.type = ultralight::MouseEvent::kType_MouseUp;
//            mouseEvent.button = ultralight::MouseEvent::kButton_Right;
//            mouseEvent.x = mouseX;
//            mouseEvent.y = mouseY;
//            m_View->FireMouseEvent(mouseEvent);
//        }
//
//        // Update last button states
//        lastLeftClick = leftClick;
//        lastRightClick = rightClick;
//
//        // Handle keyboard input
//        bool isShiftPressed = Input::isKeyDown(Key::LEFTSHIFT) || Input::isKeyDown(Key::RIGHTSHIFT);
//        bool isCtrlPressed = Input::isKeyDown(Key::LEFTCONTROL) || Input::isKeyDown(Key::RIGHTCONTROL);
//        bool isAltPressed = Input::isKeyDown(Key::LEFTALT) || Input::isKeyDown(Key::RIGHTALT);
//        bool isSuperPressed = Input::isKeyDown(Key::LEFTSUPER) || Input::isKeyDown(Key::RIGHTSUPER);
//
//        unsigned int mods = 0;
//        if (isShiftPressed) mods |= ultralight::KeyEvent::kMod_ShiftKey;
//        if (isCtrlPressed) mods |= ultralight::KeyEvent::kMod_CtrlKey;
//        if (isAltPressed) mods |= ultralight::KeyEvent::kMod_AltKey;
//        if (isSuperPressed) mods |= ultralight::KeyEvent::kMod_MetaKey;
//
//        for (auto& [k, state] : Input::keyStates) {
//            Key key = static_cast<Key>(k);
//            
//            if (Input::isKeyTriggered(key)) {
//                ultralight::KeyEvent keyEvent;
//                keyEvent.type = ultralight::KeyEvent::kType_RawKeyDown;
//                keyEvent.virtual_key_code = keyMap[key];
//                keyEvent.native_key_code = 0;
//                keyEvent.modifiers = mods;
//                GetKeyIdentifierFromVirtualKeyCode(keyEvent.virtual_key_code, keyEvent.key_identifier);
//                m_View->FireKeyEvent(keyEvent);
//            }
//            if (Input::isKeyReleased(key)) {
//                ultralight::KeyEvent keyEvent;
//                keyEvent.type = ultralight::KeyEvent::kType_KeyUp;
//                keyEvent.virtual_key_code = keyMap[key];
//                keyEvent.native_key_code = 0;
//                keyEvent.modifiers = mods;
//                GetKeyIdentifierFromVirtualKeyCode(keyEvent.virtual_key_code, keyEvent.key_identifier);
//                m_View->FireKeyEvent(keyEvent);
//            }
//        }
//
//        auto& textureLibrary = Engine::Get().GetTextureLibrary();
//        m_Renderer->Update();
//        m_Renderer->Render();
//        ultralight::BitmapSurface* surface = (ultralight::BitmapSurface*)(m_View->surface());
//
//        if (firstFrame) {
//            m_Textures[0]->CopyBitmapToTexture(surface->bitmap());
//            m_Textures[1]->CopyBitmapToTexture(surface->bitmap());
//            textureLibrary.CreateDescriptorSet(*m_Textures[0], true);
//            textureLibrary.CreateDescriptorSet(*m_Textures[1], true);
//            firstFrame = false;
//            return;
//        }
//
//        int frameIndex = Engine::Get().GetRenderer().getFrameIndex();
//        
//        if (!surface->dirty_bounds().IsEmpty()) 
//        {
//            bool madeNewImage = m_Textures[frameIndex]->CopyBitmapToTexture(surface->bitmap());
//            if (madeNewImage)
//            {
//                m_FrameToDisplay = frameIndex;
//                textureLibrary.UpdateDescriptorSet(*m_Textures[frameIndex], true);
//            }
//
//            surface->ClearDirtyBounds();        
//        }
//    }
//
//    // The texture to render
//    Texture& TextEditor::GetTexture()
//    {
//        return *m_Textures[m_FrameToDisplay];
//    }
//
//    void JavascriptLogger::OnAddConsoleMessage(ultralight::View* caller,
//        ultralight::MessageSource source,
//        ultralight::MessageLevel level,
//        const ultralight::String& message,
//        uint32_t line_number,
//        uint32_t column_number,
//        const ultralight::String& source_id) {
//
//        std::cout << "[Console]: [" << Stringify(source) << "] ["
//            << Stringify(level) << "] " << ToUTF8(message);
//
//        if (source == ultralight::kMessageSource_JS) {
//            std::cout << " (" << ToUTF8(source_id) << " @ line " << line_number
//                << ", col " << column_number << ")";
//        }
//
//        std::cout << std::endl;
//
//    }
//}