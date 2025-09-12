//#pragma once
//
//namespace Dog
//{
//    class Texture;
//    class Device;
//
//    class JavascriptLogger : public ultralight::ViewListener
//    {
//    public:
//        void OnAddConsoleMessage(ultralight::View* caller,
//            ultralight::MessageSource source,
//            ultralight::MessageLevel level,
//            const ultralight::String& message,
//            uint32_t line_number,
//            uint32_t column_number,
//            const ultralight::String& source_id) override;
//    };
//
//    class TextEditor
//    {
//    public:
//        void Init();
//        void InitPlatform();
//        void CreateRenderer();
//        void loadHTMLFromFile(ultralight::RefPtr<ultralight::View> view, const std::string& filePath);
//        void CreateView();
//
//        TextEditor(Device& device);
//        ~TextEditor();
//
//        void Update();
//
//        // getters
//        ultralight::RefPtr<ultralight::View> GetView() { return m_View; }
//        ultralight::RefPtr<ultralight::Renderer> GetRenderer() { return m_Renderer; }
//        ultralight::RefPtr<ultralight::Bitmap> GetBitmap() { return m_Bitmap; }
//        ultralight::Surface* GetSurface() { return m_Surface; }
//        std::vector<std::unique_ptr<Texture>>& GetTextures() { return m_Textures; }
//        Texture& GetTexture();
//
//
//    private:
//        std::vector<std::unique_ptr<Texture>> m_Textures;
//        ultralight::Surface* m_Surface;
//        ultralight::RefPtr<ultralight::Bitmap> m_Bitmap;
//        ultralight::RefPtr<ultralight::View> m_View;
//        ultralight::RefPtr<ultralight::Renderer> m_Renderer;
//
//        uint32_t m_FrameToDisplay = 0;
//        JavascriptLogger m_Logger;
//    };
//}
