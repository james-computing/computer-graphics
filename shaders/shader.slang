[[vk::binding(0, 0)]]
Sampler2D textures[]; // combined image samplers (texture image + texture sampler)

struct CameraUBO {
    float4x4 pv;
};

[[vk::binding(0, 1)]]
ConstantBuffer<CameraUBO> cameraUBO;

// Data that is specific to each model instance.
// Up until now only have model matrices.
[[vk::binding(0, 2)]]
StructuredBuffer<float4x4> modelMatrices;

[[vk::binding(0, 3)]]
RaytracingAccelerationStructure accelerationStructure;

struct VSInput {
    float3 position;
    float2 textureCoord;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    uint textureID : TEXID;
    float2 textureCoord;
};

[shader("vertex")]
VSOutput vertMain(
    VSInput input,
    uint drawID: SV_DrawIndex,
    uint instanceID: SV_InstanceID,
    uint instanceIDOffset: SV_StartInstanceLocation // instanceIDOffset is the firstInstance from drawIndexedIndirectDrawCommands
) {
    VSOutput output;
    float4 vertexPosition = float4(input.position, 1.0);

    output.sv_position = mul(cameraUBO.pv, mul(modelMatrices[instanceIDOffset + instanceID], vertexPosition));

    // Since every model have a single texture, we can use drawID as the textureId.
    // When using more complex models, drawID can be used to read the offset from an array.
    output.textureID = drawID;
    output.textureCoord = input.textureCoord;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target {
    return textures[input.textureID].Sample(input.textureCoord);
}