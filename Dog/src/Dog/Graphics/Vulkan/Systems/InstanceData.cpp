#include <PCH/pch.h>
#include "InstanceData.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"
#include "Graphics/Vulkan/Models/Model.h"
#include "Graphics/Vulkan/Models/ModelLibrary.h"

namespace Dog
{
    void UpdateInstanceData(InstanceData& instanceData)
    {
        /*
        Scene* currentScene = SceneManager::GetCurrentScene();
        entt::registry& registry = currentScene->GetRegistry();

        instanceData.instanceData.clear();

        registry.view<TransformComponent, ModelComponent>().each
        ([&](const auto& entity, const TransformComponent& transform, const ModelComponent& model)
        {
            if (model.ModelIndex == ModelLibrary::INVALID_MODEL_INDEX) return;

            Model* pModel = modelLibrary.GetModelByIndex(model.ModelIndex);
            
            InstancedUniformData instance;

            instance.modelMatrix = transform.mat4();
            instance.normalMatrix = transform.normalMatrix();
            instance.boundingSphere = glm::vec4(1.0f);
            instance.charIndex = 9999;
            instance.animationIndex = 9999;
            instance.followCamera = 1;

            for (int i = 0; i < pModel->meshes.size(); ++i) 
            {
                auto& mesh = pModel->meshes[i];

                instance.textureIndex = model.MaterialOverrides[i].AlbedoTexture;
                instance.meshIndex = mesh.mMeshID;

                instanceData.instanceData.push_back(instance);
            }
        });
        */
    }
}
