#pragma once

namespace Dog {

#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{};  // ignore w
		glm::vec4 color{};     // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
	};

	struct BonesUbo {
		glm::mat4 finalBonesMatrices[100]; // temp hard copy
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VkDescriptorSet globalDescriptorSet;
	};

} // namespace Dog
