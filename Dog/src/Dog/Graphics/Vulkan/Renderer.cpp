#include <PCH/pch.h>
#include "Renderer.h"

#include "ECS/Resources/RenderingResource.h"
#include "ECS/Systems/RenderSystem.h"

#include "Core/Synchronization.h"
#include "Core/SwapChain.h"
#include "Core/Device.h"

#include "Pipeline/Pipeline.h"
#include "RenderGraph.h"
#include "Uniform/Uniform.h"
#include "Uniform/ShaderTypes.h"
#include "Uniform/Descriptors.h"

#include "Model/ModelLibrary.h"
#include "Model/Model.h"

namespace Dog
{
    Renderer::Renderer(RenderingResource& rr)
        : renderingResource{ rr }
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::drawFrame()
    {
    }
}
