#pragma once

#include "Allocator.h"

namespace Dog {

    class Window;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device {
    public:
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif

        Device(Window& window);
        ~Device();

        // Not copyable or movable
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        operator VkDevice() const { return device_; }

        VkCommandPool getCommandPool() const { return commandPool; }
        VkDevice getDevice() const { return device_; }
        VkSurfaceKHR getSurface() const { return surface_; }
        VkQueue getGraphicsQueue() const { return graphicsQueue_; }
        VkQueue getPresentQueue() const { return presentQueue_; }
        const VkPhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
        const VkInstance& getInstance() const { return instance; }


        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        uint32_t GetGraphicsFamily() const { return graphicsFamily_; }
        uint32_t GetPresentFamily() const { return presentFamily_; }

        // Buffer Helper Functions
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VkBuffer& buffer,
            VmaAllocation& bufferAllocation);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VmaMemoryUsage memoryUsage,
            VkImage& image,
            VmaAllocation& imageAllocation);

        std::unique_ptr<Allocator>& GetAllocator() { return allocator; }
        VmaAllocator GetVmaAllocator() { return allocator->GetAllocator(); }

        VkPhysicalDeviceProperties properties;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
        void CheckIndirectDrawSupport();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        Window& window;
        VkCommandPool commandPool;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        uint32_t graphicsFamily_ = 0;
        uint32_t presentFamily_ = 0;

        std::unique_ptr<Allocator> allocator;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
            //VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            //VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            //VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            //VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
        };

    };

} // namespace Dog
