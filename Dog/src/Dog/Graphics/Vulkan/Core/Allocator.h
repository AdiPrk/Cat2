#pragma once

namespace Dog
{
	class Device;

	class Allocator {
	public:
		Allocator(Device& device);
		~Allocator();

		VmaAllocator GetAllocator() { return allocator; }

		/*********************************************************************
		* param:  size: Size of the buffer to create
		* param:  usage: What this buffer will hold
		* param:  memoryUsage: Properties of the buffer's memory
		* param:  buffer: Will get set to created buffer
		* param:  bufferAllocation: Will get set to created buffer's memory
		*
		* brief:  Creates and allocates buffer
		*********************************************************************/
		void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VmaMemoryUsage memoryUsage,
			VkBuffer& buffer,
			VmaAllocation& bufferAllocation);

		void DestroyBuffer(VkBuffer buffer, VmaAllocation bufferAllocation);

		/*********************************************************************
	 * param:  imageInfo: Create info for image to create
	 * param:  memoryUsage: Properties of the memory to be created for
	 *                     wanted image
	 * param:  image:      Will be set to created image
	 * param:  imageMemory: Will be set to created image memory
	 *
	 * brief:  Creates an image from passed create info and then
	 *         allocates and bind memory to the image. Both the image
	 *         and the image memory will be set in passed references.
	 *         Throws errors if any creation or allocation failed.
	 *********************************************************************/
		void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& imageAllocation);

	private:
		VmaAllocator allocator;
	};

}