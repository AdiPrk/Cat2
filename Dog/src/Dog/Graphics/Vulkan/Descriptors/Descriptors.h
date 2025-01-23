#pragma once

#include "../Core/Device.h"

namespace Dog {

    //Class wrapping VkDescriptorSetLayout's
    class DescriptorSetLayout
    {
    public:

        //Class for building Descriptor set layouts
        class Builder
        {
        public:
            /*********************************************************************
             * param:  Device: Device for this builder to use
             *
             * brief:  Constructs a new builder object
             *********************************************************************/
            Builder(Device& device)
                : mDevice{ device }
            {
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// Helpers ////////////////////////////////////////////////////////////////////////////////

            /*********************************************************************
             * param:  binding: Wanted binding index in the set (like the first
             *                  one made would be 0 probably)
             * param:  descriptorType: What type of descriptor should Vulkan expect
             * param:  stageFlags: What shader stages have access to this binding
             * param:  count: Number of descriptors for this binding (can have
             *                what is basicly an array of discriptors bound)
             * return: A refernce to this builder, for chaining bind calls together
             *
             * brief:  Appends to the map of bindings the information that vulkan
             *         requires for this descriptor
             *********************************************************************/
            Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
            Builder& AddBinding(VkDescriptorSetLayoutBinding binding);

            /*********************************************************************
             * return: Created isntance of the descriptorSetLayout
             *
             * brief:  Final step of the builder, creats an instance of the
             *         DescriptorSetLayout using bound data
             *********************************************************************/
            std::unique_ptr<DescriptorSetLayout> Build() const;

        private:

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// Varibles ///////////////////////////////////////////////////////////////////////////////

            Device& mDevice; //Device descriptors are being made for
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings{}; //Map of building used to build a descriptorSetLayout
        };

        //Delete copy operators
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        /*********************************************************************
         * param:  Device: Device descriptors are being made for
         * param:  bindings: List of bindings to make into this set
         *
         * brief:  Manual constructor (no builder) for a descriptor set
         *         layout object
         *********************************************************************/
        DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        /*********************************************************************
         * brief:  Destroys this Descriptor set layout
         *********************************************************************/
        ~DescriptorSetLayout();

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Getters ////////////////////////////////////////////////////////////////////////////////

        /*********************************************************************
         * return: This DescriptorSetLayout's VkDescriptorSetLayout
         *
         * brief:  Returns this DescriptorSetLayout's VkDescriptorSetLayout
         *********************************************************************/
        VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Varibles ///////////////////////////////////////////////////////////////////////////////

        Device& mDevice;                            //Device descriptors are for
        VkDescriptorSetLayout mDescriptorSetLayout; //Vulkan refence of the descriptor set layout itself
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings; //Map of all bindings in this set layout

        friend class DescriptorWriter;
    };

    //Class wrapping VkDescriptorPool's
    class DescriptorPool
    {
    public:

        //Class for building Descriptor pools
        class Builder
        {
        public:
            /*********************************************************************
             * param:  Device: Device for this builder to use
             *
             * brief:  Constructs a new builder object
             *********************************************************************/
            Builder(Device& device)
                : mDevice{ device }
            {
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// Setters ////////////////////////////////////////////////////////////////////////////////

            /*********************************************************************
             * param:  descriptorType: Type of descriptors to be in this pool
             * param:  count: Number of descriptors to be in this pool
             * return: A refernce to this builder, for chaining bind calls together
             *
             * brief:  Defines the VkDescriptorPoolSize of this pool, pools can have
             *         more than one pool size (and will be able to allocate from
             *         both sizes)
             *********************************************************************/
            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            /*********************************************************************
             * param:  flags: Flags for this pool to use
             * return: A refernce to this builder, for chaining bind calls together
             *
             * brief:  Set the flags for this pool, what they are:
             *         https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolCreateFlagBits.html
             *********************************************************************/
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            /*********************************************************************
             * param:  count: Wanted number of descriptor sets
             * return: A refernce to this builder, for chaining bind calls together
             *
             * brief:  Sets the total number of descriptor sets that can be
             *         allocated from this pool
             *********************************************************************/
            Builder& SetMaxSets(uint32_t count);
            /*********************************************************************
             * return: Created isntance of the descriptorPool
             *
             * brief:  Final step of the builder, creats an instance of the
             *         DescriptorPool using set data
             *********************************************************************/
            std::unique_ptr<DescriptorPool> Build() const;

        private:

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// Varibles ///////////////////////////////////////////////////////////////////////////////

            Device& mDevice;                               //Device descriptors are being made for
            std::vector<VkDescriptorPoolSize> mPoolSizes{}; //Sizes of descriptor to be allocated from this pool
            uint32_t mMaxSets = 1000;                       //Max number of sets that can be allocated from this pool
            VkDescriptorPoolCreateFlags mPoolFlags = 0;     //Flag settings for this pool
        };

        //Delete copy operators
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        /*********************************************************************
         * param:  Device: Device descriptors are being made for
         * param:  maxSets: Max number of sets that can be allocated from this pool
         * param:  poolFlags: Flag settings for this pool
         * param:  poolSizes: Sizes of descriptor to be allocated from this pool
         *
         * brief:  Manual constructor (no builder) for a descriptor pool
         *         object
         *********************************************************************/
        DescriptorPool(Device& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);

        /*********************************************************************
         * brief:  Destroys this description pool object
         *********************************************************************/
        ~DescriptorPool();

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Getters ////////////////////////////////////////////////////////////////////////////////

        const VkDescriptorPool& GetDescriptorPool() const { return mDescriptorPool; }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Helpers ////////////////////////////////////////////////////////////////////////////////

        /*********************************************************************
         * param:  descriptorSetLayout: Layout to use for the set allocating
         * param:  descriptor: Will be set to the descriptor allocated
         * return: True if succesfull, else false
         *
         * brief:  Allocates a descriptor set according to passed layout
         *********************************************************************/
        bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
        /*********************************************************************
         * param:  descriptors: Descriptors to free
         *
         * brief:  Frees passed descriptors
         *********************************************************************/
        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        /*********************************************************************
         * brief:  Return all descriptor sets allocated from this pool to the
         *         pool
         *********************************************************************/
        void ResetPool();

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Varibles ///////////////////////////////////////////////////////////////////////////////

        Device& mDevice;                  //Device descriptors are for
        VkDescriptorPool mDescriptorPool; //Vulkan DescriptorPool object that is being wrapped by this class

        friend class DescriptorWriter;
    };

    class DescriptorWriter
    {
    public:
        /*********************************************************************
         * param:  setLayout: Layout of the sets that will be written
         * param:  pool: Pool to allocate written descriptors from
         *
         * brief:  Creates a descriptor writer object
         *********************************************************************/
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Helpers ////////////////////////////////////////////////////////////////////////////////

        /*********************************************************************
         * param:  binding: Binding index of discriptor writing too
         * param:  bufferInfo: Information about buffer that holds the
         *                     information that the descriptor is describing
         * param:  count: Number of descriptors to write
         * return: A refernce to this writer, for chaining bind calls together
         *
         * brief:  Make a new VkWriteDescriptorSet with passed information and
         *         adds to to writes (writes will be writen on a build call)
         *********************************************************************/
        DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, uint32_t count = 1);
        /*********************************************************************
         * param:  binding: Binding index of discriptor writing too
         * param:  imageInfo: Information about image that holds the
         *                    information that the descriptor is describing
         * return:  A refernce to this writer, for chaining bind calls together
         *
         * brief:  Make a new VkWriteDescriptorSet with passed information and
         *         adds to to writes (writes will be writen on a build call)
         *********************************************************************/
        DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t imageCount = 1);

        /*********************************************************************
         * param:  set: Will be set to vkDescriptor allocated
         * return: True if allocation and update were succcessful, else false
         *
         * brief:  Allocates a set and attempts to update the sets on the gpu
         *********************************************************************/
        bool Build(VkDescriptorSet& set);
        /*********************************************************************
         * param:  set: Set to update on the gpu
         *
         * brief:  Updates this descriptor set on the gpu
         *********************************************************************/
        void Overwrite(VkDescriptorSet& set);

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Varibles ///////////////////////////////////////////////////////////////////////////////

        DescriptorSetLayout& mSetLayout;                    //Layout of the sets being written
        DescriptorPool& mPool;                              //Pool to allocate sets from
        std::vector<VkWriteDescriptorSet> mWritesToPreform; //Writes to preform on build
    };

} // namespace Dog