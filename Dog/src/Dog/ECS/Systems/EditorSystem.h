#pragma once

#include "ISystem.h"

namespace Dog
{
    class EditorSystem : public ISystem
    {
    public:
        EditorSystem() : ISystem("EditorSystem") {};
        ~EditorSystem() {}

        void Init();
        void FrameStart();
        void Update(float dt);
        void FrameEnd();
        void Exit();

    private:
        void InitImGui();
        void RenderImGui(VkCommandBuffer cmd);

        void RenderSceneWindow();
        void RenderEntitiesWindow();
        void RenderInspectorWindow();
    };
}