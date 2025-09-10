#include <PCH/pch.h>
#include "PresentPass.h"

#include "../Pipeline/Pipeline.h"
#include "../Core/SwapChain.h"
#include "../Core/Synchronization.h"
namespace Dog
{
    PresentPass::PresentPass(Device& device, SwapChain& swapChain, Synchronizer& sync)
        : device(device)
        , swapChain(swapChain)
        , syncObjects(sync)
    {
        CreateRenderPass();
        CreatePipeline();
    }

    PresentPass::~PresentPass()
    {
        Cleanup();
    }

    void PresentPass::CreateRenderPass()
    {
        //Description of depth attachment
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = swapChain.GetDepthFormat();

        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                                //One sample per pixel
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           //Tells the depthbuffer attachment to clear each time it is loaded
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     //Specifies the contents within the depth buffer render area are not needed after rendering
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                //Specifies that the previous contents within the stencil need not be preserved and will be undefined
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              //Specifies the contents within the stencil render area are not needed after rendering
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      //Inital image contents does not matter since we are clearing them on load
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //Set so the layout after rendering allows read and write access as a depth/stencil attachment

        //Defines the attachment index and the layout while rendering for the subpass (given to subpass below)
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;                                            //Index of this attachment in the swapchain
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //What this attachment is layed out to support

        //Description of color attachment
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChain.GetSwapChainImageFormat();                //Set image format to match what is already being used by the swapchain
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                   //One sample per pixel (more samples used for multisampling)
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;              //Tells the colorbuffer attachment to clear each time it is loaded
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;            //Specifies the contents generated during the render pass and within the render area are written to memory.
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //Specifies that the previous contents within the stencil need not be preserved and will be undefined
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   //Specifies the contents within the stencil render area are not needed after rendering
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;         //Inital image contents does not matter since we are clearing them on load
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;     //Layout used for presenting to screen 

        //Defines the attachment index and the layout while rendering for the subpass (given to subpass below)
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;                                    //Index of this attachment in the swapchain
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //What this attachment is layed out to support

        //A subpass in Vulkan is a phase of rendering that can read from and write to certain framebuffer attachments (color, depth, and stencil buffers)
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //Define that this subpass is for graphics output (instead of something like a compute shader)
        subpass.colorAttachmentCount = 1;                            //Number of color attachments used
        subpass.pColorAttachments = &colorAttachmentRef;             //Refenece to the attachment index and layout for the color attachment
        subpass.pDepthStencilAttachment = &depthAttachmentRef;       //Refenece to the attachment index and layout for the depth attachment

        //Declare a dependency for the subpass (forces thread sync between source and destination)
        VkSubpassDependency dependency = {};
        //Sets the source of this dependency to be before the subpasses start
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        //No specific memory access is being waited on from the source operation.
        dependency.srcAccessMask = 0;
        //The render pass should wait until all color attachment writes and early fragment (depth) tests outside of the render pass have been completed.
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        //Sets the destination of this dependency to be the first subpass
        dependency.dstSubpass = 0;
        //Subpass 0's color attachment output and early fragment tests will only begin after the external stages (color attachment output and early fragment tests) are complete.
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        //These access masks ensure that the operations described by dstStageMask (color and depth/stencil attachment writes) will wait for the completion of the stages in the srcStageMask.
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        //Put attachments into an array
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        //Create the render pass from information above
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;           //Define this structor as a render pass create info
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); //Number of attachments being used
        renderPassInfo.pAttachments = attachments.data();                           //Array of all attachments
        renderPassInfo.subpassCount = 1;                                            //Number of subpasses in this renderpass
        renderPassInfo.pSubpasses = &subpass;                                       //Reference to subpass being used
        renderPassInfo.dependencyCount = 1;                                         //Number of dependencies in this renderpass
        renderPassInfo.pDependencies = &dependency;                                 //Reference to dependency being used

        //Create render pass
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create render pass");
        }
    }

    void PresentPass::CreatePipeline()
    {
        std::vector<Uniform*> unis{}; // temp empty
        pipeline = std::make_unique<Pipeline>(device, unis, mRenderPass, false, "basic_tri.vert", "basic_tri.frag");
    }

    void PresentPass::Execute(VkCommandBuffer cmd)
    {
        pipeline->Bind(cmd);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
    
    void PresentPass::Cleanup()
    {
        vkDestroyRenderPass(device, mRenderPass, nullptr);
    }
}
