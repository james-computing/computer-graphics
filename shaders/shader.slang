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
    float3 worldPosition; // for ray query in fragment shader
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
    float4 worldPosition = mul(modelMatrices[instanceIDOffset + instanceID], vertexPosition);

    output.worldPosition = worldPosition.xyz;
    output.sv_position = mul(cameraUBO.pv, worldPosition);

    // Since every model have a single texture, we can use drawID as the textureId.
    // When using more complex models, drawID can be used to read the offset from an array.
    output.textureID = drawID;
    output.textureCoord = input.textureCoord;
    return output;
}

static const float3 lightDirection = float3(-0.7071067811865475, 0.0, 0.7071067811865475);
static const float EPSILON = 0.01;

bool inShadow(float3 p) {
    RayDesc rayDesc;
    rayDesc.Origin = p;
    rayDesc.Direction = lightDirection;
    rayDesc.TMin = EPSILON;
    rayDesc.TMax = 1e4;

    // Initialize a ray query
    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    let rayFlags = RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    rayQuery.TraceRayInline(accelerationStructure, rayFlags, 0xFF, rayDesc);

    rayQuery.Proceed();

    bool hit = (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT);

    return hit;
}

[shader("fragment")]
[require(spvRayQueryKHR)]
float4 fragMain(VSOutput input) : SV_Target {
    float4 baseColor = textures[input.textureID].Sample(input.textureCoord);

    float3 p = input.worldPosition;

    float4 finalColor;
    if (inShadow(p)) {
        finalColor = baseColor * 0.2;
    } else {
        finalColor = baseColor;
    }

    return finalColor;
}