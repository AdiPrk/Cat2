#pragma once

namespace Dog {

	// forward declarations
	class Device;
	class Renderer;
	class TextureLibrary;

	class ImGuiTextureManager {
	public:
		// needs device, renderer, texture manager
		ImGuiTextureManager(Device& device);
		~ImGuiTextureManager();

		VkDescriptorSet CreateDescriptorSet(const VkImageView& imageView, const VkSampler& sampler);

		void AddTexture(const std::string& texturePath, const VkImageView& imageView, const VkSampler& sampler);

		// get descriptor set
		VkDescriptorSet GetDescriptorSet(const std::string& texturePath);


	private:
		Device& device;

		std::unordered_map<std::string, VkDescriptorSet> descriptorMap;
	};

} // namespace Dog