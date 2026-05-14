#include "../include/shader.hpp"

std::vector<char> Shader::readFile(std::string const & filename) {
    // ate = start reading at the end of the file. This is used to get the file size.
    // binary is to read as binary, avoiding text transformations.
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    // Create a buffer with the file size
    size_t const fileSize {(size_t) file.tellg()};
    std::vector<char> buffer(fileSize);
    // Go to beggining of the file
    file.seekg(0);
    // Read the whole file to the buffer
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

[[nodiscard]] vk::raii::ShaderModule Shader::createShaderModule(std::vector<char> const & code, vk::raii::Device const & device) {
    // Size of type used for code, in bytes.
    // If the type is char, then typeSizeInBytes = 1.
    size_t constexpr typeSizeInBytes {sizeof(*code.data())};

    // The code must be passed as a pointer of type uint32_t
    uint32_t const * const codeReinterpreted = reinterpret_cast<uint32_t const *>(code.data());

    vk::ShaderModuleCreateInfo const shaderModuleCreateInfo {
        // code size is the size in bytes
        .codeSize = code.size() * typeSizeInBytes,
        .pCode = codeReinterpreted
    };

    vk::raii::ShaderModule shaderModule {vk::raii::ShaderModule(device, shaderModuleCreateInfo)};
    return shaderModule;
}