#include "Objects/render_texture.h"

#include "Core/e_memory.h"
#include "Core/engine.h"
#include "Core/e_device.h"
#include "Core/e_buffer.h"
#include "Core/e_texture.h"
#include "Core/swapchain.h"

#include <vulkan/vulkan.h>

#include "Tools/e_math.h"
#include "Tools/e_tools.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

void RenderTextureCreateDepthResource(RenderTexture *render, RenderFrame *frame)
{

    VkFormat depthFormat = findDepthFormat();

    TextureCreateImage(render->width, render->height, 1,depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, &frame->depth_texture);
    frame->depth_texture.image_view = TextureCreateImageView(frame->depth_texture.image, VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    ToolsTransitionImageLayout(frame->depth_texture.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

}

void RenderTextureCreateRenderPass(RenderTexture *render, void **render_pass)
{
    TDevice *device = (TDevice *)engine.device;

    if(render->m_format == 0)
    {
        VkSubpassDescription subpassDescription;
        memset(&subpassDescription, 0, sizeof(VkSubpassDescription));

        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Geometry render pass doesn't need any output attachment.
        VkRenderPassCreateInfo renderPassInfo;
        memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));

        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 0;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;

        if(vkCreateRenderPass(device->e_device, &renderPassInfo, NULL, (VkRenderPass *)render_pass) != VK_SUCCESS)
        {
            printf("Error create render pass for render texture.");
            exit(1);
        }

        return;
    }

    VkAttachmentDescription *colorAttachment = AllocateMemory(1, sizeof(VkAttachmentDescription));
    colorAttachment->format = render->m_format;
    colorAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment->initialLayout = render->type == TIGOR_RENDER_TYPE_IMAGE || render->type == TIGOR_RENDER_TYPE_CUBEMAP ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment->finalLayout = render->type == TIGOR_RENDER_TYPE_IMAGE || render->type == TIGOR_RENDER_TYPE_CUBEMAP ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription *depthAttachment = AllocateMemory(1, sizeof(VkAttachmentDescription));
    depthAttachment->format = findDepthFormat();
    depthAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    if(render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH))
    {
        depthAttachment->initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthAttachment->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }else{
        depthAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    VkAttachmentReference *colorAttachmentRef = AllocateMemory(1, sizeof(VkAttachmentReference));
    colorAttachmentRef->attachment = 0;//Номер атачмента
    colorAttachmentRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference *depthAttachmentRef = AllocateMemory(1, sizeof(VkAttachmentReference));
    depthAttachmentRef->attachment = render->type  == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH) ? 0 : 1;//Номер атачмента
    depthAttachmentRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    memset(&subpass, 0, sizeof(VkSubpassDescription));
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    if(render->type != TIGOR_RENDER_TYPE_IMAGE)
        subpass.pDepthStencilAttachment = depthAttachmentRef;

    if(render->type != TIGOR_RENDER_TYPE_DEPTH && !(render->flags & TIGOR_RENDER_FLAG_DEPTH)){
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = colorAttachmentRef;
    }

    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));

    VkSubpassDependency *depency = AllocateMemory( render->type != TIGOR_RENDER_TYPE_DEPTH ? 2 : 1, sizeof(VkSubpassDependency));

    VkAttachmentDescription attachments[] = {*colorAttachment, *depthAttachment};

    if(render->type == TIGOR_RENDER_TYPE_CUBEMAP)
    {
        if(render->flags & TIGOR_RENDER_FLAG_DEPTH)
        {
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = depthAttachment;
        }else{

            renderPassInfo.attachmentCount = 2;
            renderPassInfo.pAttachments = attachments;
        }


    }else if(render->type != TIGOR_RENDER_TYPE_DEPTH){


        depency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        depency[0].dstSubpass = 0;
        depency[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        depency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        depency[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        depency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        depency[1].srcSubpass = 0;
        depency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        depency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depency[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        depency[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        if(render->type == TIGOR_RENDER_TYPE_WINDOW)
        {
            renderPassInfo.attachmentCount = 2;
            renderPassInfo.pDependencies = depency;
            renderPassInfo.pAttachments = attachments;
        }else{
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pDependencies = depency;
            renderPassInfo.pAttachments = colorAttachment;
        }

    }else{

        depency->srcSubpass = VK_SUBPASS_EXTERNAL;
        depency->dstSubpass = 0;
        depency->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        depency->srcAccessMask = 0;
        depency->dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        depency->dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pDependencies = depency;
        renderPassInfo.pAttachments = depthAttachment;
    }

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = render->type == TIGOR_RENDER_TYPE_CUBEMAP ? 0 : 1;


    if(vkCreateRenderPass(device->e_device, &renderPassInfo, NULL, (VkRenderPass *)render_pass) != VK_SUCCESS)
    {
        printf("Error create render pass for render texture.");
        exit(1);
    }

    FreeMemory(colorAttachment);
    FreeMemory(depthAttachment);
    FreeMemory(colorAttachmentRef);
    FreeMemory(depthAttachmentRef);
    FreeMemory(depency);
}

void RenderTextureTransitionLayer(RenderFrame *frame, uint32_t render_type, uint32_t type_layout, uint32_t aspect_mask)
{
    VkImageMemoryBarrier imgBar;

    imgBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBar.pNext = NULL;
    imgBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar.srcAccessMask = 0;
    imgBar.dstAccessMask = 0;
    imgBar.oldLayout = frame->m_currentLayout;
    imgBar.newLayout = type_layout;
    imgBar.image = frame->render_texture.image;

    imgBar.subresourceRange.aspectMask = aspect_mask;//render_type == TIGOR_RENDER_TYPE_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imgBar.subresourceRange.baseMipLevel = 0;
    imgBar.subresourceRange.levelCount = 1;
    imgBar.subresourceRange.baseArrayLayer = 0;

    imgBar.subresourceRange.layerCount = render_type == TIGOR_RENDER_TYPE_CUBEMAP ? 6 : 1;

    void *cmd_buff = beginSingleTimeCommands();

    vkCmdPipelineBarrier(cmd_buff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imgBar);

    endSingleTimeCommands(cmd_buff);
}

void RenderTextureCreateFrames(RenderTexture *render, uint32_t flags)
{
    TDevice *device = (TDevice *)engine.device;
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    for (int i=0;i < render->num_frames;i++) {

        RenderFrame *frame = &render->frames[i];

        if(render->m_format != 0)
        {
            frame->m_currentLayout = render->type == TIGOR_RENDER_TYPE_WINDOW ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

            if(render->type != TIGOR_RENDER_TYPE_WINDOW)
            {
                uint32_t usage = render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH) ?  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT :  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

                TextureCreateImage(render->width, render->height, 1,render->m_format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, flags, &frame->render_texture);

                TextureCreateSampler(&frame->render_texture.sampler, render->m_format, 1);

                if(render->type & TIGOR_RENDER_TYPE_CUBEMAP)
                {
                    frame->shadowCubeMapFaceImageViews = AllocateMemoryP(6, sizeof(VkImageView), render);
                    frame->render_texture.image_view = TextureCreateImageViewCube(frame->render_texture.image, frame->shadowCubeMapFaceImageViews, render->m_format, render->flags & TIGOR_RENDER_FLAG_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
                    frame->framebufers = AllocateMemoryP(6, sizeof(VkFramebuffer), render);
                }else{
                    frame->render_texture.image_view = TextureCreateImageView(frame->render_texture.image, VK_IMAGE_VIEW_TYPE_2D, render->m_format, render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 1);
                    frame->framebufers = AllocateMemoryP(1, sizeof(VkFramebuffer), render);
                }

                RenderTextureCreateDepthResource(render, frame);

                RenderTextureTransitionLayer(frame, render->type, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);

            }else{
                frame->render_texture.image_view = swapchain->swapChainImageViews[i];
                frame->framebufers = AllocateMemoryP(1, sizeof(VkFramebuffer), render);
            }
        }else{
            frame->m_currentLayout = VK_IMAGE_LAYOUT_GENERAL;

            frame->framebufers = AllocateMemoryP(1, sizeof(VkFramebuffer), render);
        }

        VkFramebufferCreateInfo *framebufferInfo = AllocateMemory(1, sizeof(VkFramebufferCreateInfo));
        framebufferInfo->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo->renderPass = render->render_pass;
        framebufferInfo->width = render->width;
        framebufferInfo->height = render->height;
        framebufferInfo->layers = 1;

        if(render->type == TIGOR_RENDER_TYPE_GEOMETRY)
        {
            if(vkCreateFramebuffer(device->e_device, framebufferInfo, NULL, (VkFramebuffer *)&frame->framebufers[0]) != VK_SUCCESS)
            {
                printf("Error create framebuffer for render texture.");
                exit(1);
            }
        }else if(render->type == TIGOR_RENDER_TYPE_WINDOW)
        {
            VkImageView attachments[2];
            attachments[0] = frame->render_texture.image_view;
            attachments[1] = swapchain->depth_texture.image_view;

            framebufferInfo->attachmentCount = 2;
            framebufferInfo->pAttachments = attachments;

            if(vkCreateFramebuffer(device->e_device, framebufferInfo, NULL, (VkFramebuffer *)&frame->framebufers[0]) != VK_SUCCESS)
            {
                printf("Error create framebuffer for render texture.");
                exit(1);
            }

        }else if(render->type == TIGOR_RENDER_TYPE_CUBEMAP){


            if(render->flags & TIGOR_RENDER_FLAG_DEPTH)
            {
                for(int j=0;j < 6;j++)
                {
                    VkImageView attachments[] = { frame->shadowCubeMapFaceImageViews[j] };

                    framebufferInfo->attachmentCount = 1;
                    framebufferInfo->pAttachments = attachments;

                    if(vkCreateFramebuffer(device->e_device, framebufferInfo, NULL, (VkFramebuffer *)&frame->framebufers[j]) != VK_SUCCESS)
                    {
                        printf("Error create framebuffer for render texture.");
                        exit(1);
                    }
                }
            }else{
                VkImageView attachments[2];
                attachments[1] = frame->depth_texture.image_view;

                for(int j=0;j < 6;j++)
                {
                    attachments[0] = frame->shadowCubeMapFaceImageViews[j];

                    framebufferInfo->attachmentCount = 2;
                    framebufferInfo->pAttachments = attachments;

                    if(vkCreateFramebuffer(device->e_device, framebufferInfo, NULL, (VkFramebuffer *)&frame->framebufers[j]) != VK_SUCCESS)
                    {
                        printf("Error create framebuffer for render texture.");
                        exit(1);
                    }
                }
            }

        }else{
            VkImageView attachment[] = { frame->depth_texture.image_view };

            framebufferInfo->attachmentCount = 1;
            framebufferInfo->pAttachments = attachment;

            if(vkCreateFramebuffer(device->e_device, framebufferInfo, NULL, (VkFramebuffer *)&frame->framebufers[0]) != VK_SUCCESS)
            {
                printf("Error create framebuffer for render texture.");
                exit(1);
            }
        }


        FreeMemory(framebufferInfo);
    }
}

void RenderTextureInit(RenderTexture *render, uint32_t type, uint32_t width, uint32_t height, uint32_t flags)
{
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    memset(render, 0, sizeof(RenderTexture));

    switch(type)
    {
        case TIGOR_RENDER_TYPE_DEPTH:
            render->m_format = findDepthFormat();
            break;
        case TIGOR_RENDER_TYPE_CUBEMAP:
            render->m_format = VK_FORMAT_R32_SFLOAT;
            break;
        case TIGOR_RENDER_TYPE_WINDOW:
            render->m_format = swapchain->swapChainImageFormat;
            break;
        case TIGOR_RENDER_TYPE_IMAGE:
            render->m_format = swapchain->swapChainImageFormat;
            break;
        default:
            render->m_format = 0;
            break;
    }

    render->type = type;
    render->mip_levels = floor(log2(e_max(width, height))) + 1;

    if(flags & TIGOR_RENDER_FLAG_DEPTH)
        render->m_format = findDepthFormat();

    render->flags = flags;

    render->currFrame = 0;

    if(type != TIGOR_RENDER_TYPE_WINDOW && type != TIGOR_RENDER_TYPE_GEOMETRY)
    {
        render->height = height;
        render->width = width;
    }else{
        render->height = swapchain->swapChainExtent.height;
        render->width = swapchain->swapChainExtent.width;
    }

    render->persp_view_distance = 500;
    render->persp_view_near = 0.01;
    render->persp_view_angle = 60;

    render->ortg_view_distance = 100;
    render->ortg_view_size = 0.01;

    render->frust_far = 100;
    render->frust_near = 0.1;
    render->frust_side = 0.1;

    render->cascadeSplit = 0;

    render->clear_color = vec3_f( 0.3, 0.01f, 0.01f);
    render->up_vector = vec3_f( 0, 1, 0);

    if(render->type == TIGOR_RENDER_TYPE_IMAGE)
    {
        render->flags |= TIGOR_RENDER_FLAG_ONE_SHOT;
        render->frames = (RenderFrame*) AllocateMemoryP(1, sizeof(RenderFrame), render);
        render->num_frames = 1;
    }else{
        render->frames = (RenderFrame*) AllocateMemoryP(engine.imagesCount, sizeof(RenderFrame), render);
        render->num_frames = engine.imagesCount;
    }

    RenderTextureCreateRenderPass(render, &render->render_pass);

    if(type & TIGOR_RENDER_TYPE_CUBEMAP)
        RenderTextureCreateFrames(render, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    else
        RenderTextureCreateFrames(render, 0);

    return;
}

void RenderTextureReload(RenderTexture *render, uint32_t indx_surf)
{
    if(render->flags & TIGOR_RENDER_FLAG_ONE_SHOT)
        render->flags &= ~(TIGOR_RENDER_FLAG_SHOOTED);
}

void RenderTextureRecreate(RenderTexture *render)
{
    RenderTextureDestroy(render);

    if(render->type == TIGOR_RENDER_TYPE_WINDOW)
        RenderTextureInit(render, render->type, engine.width, engine.height, render->flags);
    else
        RenderTextureInit(render, render->type, render->width, render->height, render->flags);
}

void RenderTextureBeginRendering(RenderTexture *render, void *cmd_buff)
{
    RenderFrame *frame;

    if(render->flags & TIGOR_RENDER_FLAG_ONE_SHOT)
        frame = &render->frames[0];
    else
        frame = &render->frames[engine.imageIndex];

    VkRenderPassBeginInfo *renderBeginInfo = AllocateMemory(1, sizeof(VkRenderPassBeginInfo));
    renderBeginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo->renderPass = render->render_pass;

    if(render->type & TIGOR_RENDER_TYPE_CUBEMAP)
        renderBeginInfo->framebuffer = frame->framebufers[render->currFrame];
    else{
        renderBeginInfo->framebuffer = frame->framebufers[0];
    }
    renderBeginInfo->renderArea.offset.x = 0;
    renderBeginInfo->renderArea.offset.y = 0;
    renderBeginInfo->renderArea.extent.width = render->width;
    renderBeginInfo->renderArea.extent.height = render->height;

    VkClearValue clearValue;
    VkClearValue clearValues[2];

    if(render->m_format != 0)
    {
        if(render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH))
        {

            clearValue.depthStencil.depth = 1.0f;
            clearValue.depthStencil.stencil = 0.0;

            renderBeginInfo->clearValueCount = 1;
            renderBeginInfo->pClearValues = &clearValue;
        }else{

            clearValues[0].color.float32[0] = render->clear_color.x;
            clearValues[0].color.float32[1] = render->clear_color.y;
            clearValues[0].color.float32[2] = render->clear_color.z;
            clearValues[0].color.float32[3] = 1.0f;
            clearValues[1].depthStencil.depth = 1.0f;
            clearValues[1].depthStencil.stencil = 0;

            renderBeginInfo->clearValueCount = 2;
            renderBeginInfo->pClearValues = clearValues;
        }
    }

    vkCmdBeginRenderPass(cmd_buff, renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    FreeMemory(renderBeginInfo);
}

void RenderTextureSetCurrentFrame(RenderTexture *render, uint32_t indx_frame)
{
    render->currFrame = indx_frame;
}

void RenderTextureEndRendering(RenderTexture *render, void *cmd_buff)
{
    vkCmdEndRenderPass(cmd_buff);
}

void RenderTextureDestroy(RenderTexture *render)
{
    TDevice *device = (TDevice *)engine.device;

    for (int j=0;j< render->num_frames;j++) {

        if(render->type != TIGOR_RENDER_TYPE_WINDOW)
        {

            ImageDestroyTexture(&render->frames[j].render_texture);
            ImageDestroyTexture(&render->frames[j].depth_texture);
        }

        if(render->type & TIGOR_RENDER_TYPE_CUBEMAP)
        {
            for(int i=0;i < 6;i++){
                vkDestroyFramebuffer(device->e_device, render->frames[j].framebufers[i], NULL);
                vkDestroyImageView(device->e_device, render->frames[j].shadowCubeMapFaceImageViews[i], NULL);
            }
        }else
            vkDestroyFramebuffer(device->e_device, render->frames[j].framebufers[0], NULL);
            
        FreeMemory(render->frames[j].framebufers);
    }

    FreeMemory(render->frames);

    vkDestroyRenderPass(device->e_device, render->render_pass, NULL);
}
