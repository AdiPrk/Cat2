#pragma once

namespace Dog
{
    struct RenderingResource;
    class RenderGraph;
    class Pipeline;

    class Renderer
    {
    public:
        Renderer(RenderingResource& rr);
        ~Renderer();

        void drawFrame();

    private:
        friend RenderGraph;
        RenderingResource& renderingResource;
    };
}
