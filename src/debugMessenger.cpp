#include "../include/debugMessenger.hpp"

VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugMessenger::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT        severity,
    vk::DebugUtilsMessageTypeFlagsEXT               type,
    vk::DebugUtilsMessengerCallbackDataEXT const *  pCallBackData,
    void *                                          pUserData
) {
    std::cerr << "\nvalidation layer:\n" <<
                    "\ttype " << vk::to_string(type) << '\n' <<
                    "\tmsg: " << pCallBackData->pMessage << std::endl;
    if (type >= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation) {
        throw std::runtime_error("Vulkan error!");
    }
    return vk::False;
}

void DebugMessenger::setup(vk::raii::Instance const & instance) {
    vk::DebugUtilsMessageSeverityFlagsEXT constexpr severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );

    vk::DebugUtilsMessageTypeFlagsEXT constexpr messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );

    vk::DebugUtilsMessengerCreateInfoEXT const debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &debugCallback
    };

    // try catch?
    vkraii = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}