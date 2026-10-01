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

// All are model data. Should put into a single descriptor set or binding later.
[[vk::binding(0, 4)]]
StructuredBuffer<uint> indexBuffer;

[[vk::binding(0, 5)]]
StructuredBuffer<uint> indexOffsets;

[[vk::binding(0, 6)]]
StructuredBuffer<VSInput> vertexBuffer;

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

static const float3 lightDirection = float3(-0.7071067811865475, 0.7071067811865475, 0.0); // points to the light source
static const float EPSILON = 0.01;

float2 intersectionUV(uint modelID, uint triangleIndex, float2 barycentrics) {
    uint indexOffset = indexOffsets[NonUniformResourceIndex(modelID)];

    // Compute the indices
    // This requires the index buffer to be a storage buffer also
    uint i0 = indexBuffer[indexOffset + (3 * triangleIndex + 0)];
    uint i1 = indexBuffer[indexOffset + (3 * triangleIndex + 1)];
    uint i2 = indexBuffer[indexOffset + (3 * triangleIndex + 2)];

    // The vertex buffer also needs to be a storage buffer
    float2 uv0 = vertexBuffer[i0].textureCoord;
    float2 uv1 = vertexBuffer[i1].textureCoord;
    float2 uv2 = vertexBuffer[i2].textureCoord;

    // Compute the first barycentric coordinate from the other ones, and rename.
    float w0 = 1.0 - barycentrics.x - barycentrics.y;
    float w1 = barycentrics.x;
    float w2 = barycentrics.y;

    // Compute the uv coordinate by interpolating the uv's for each vertex
    float2 uv = w0 * uv0 + w1 * uv1 + w2 * uv2;

    return uv;
}

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

    while (rayQuery.Proceed()) {
        uint modelID = rayQuery.CandidateRayInstanceCustomIndex(); // id of the model hit by the ray
        uint triangleIndex = rayQuery.CandidatePrimitiveIndex();
        float2 barycentrics = rayQuery.CandidateTriangleBarycentrics(); // barycentric coordinates of the hit

        float2 uv = intersectionUV(modelID, triangleIndex, barycentrics);
        float4 intersectionColor = textures[NonUniformResourceIndex(modelID)].Sample(uv);
        if (intersectionColor.a >= 0.5) {
            rayQuery.CommitNonOpaqueTriangleHit();
        }
    }

    bool hit = (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT);

    return hit;
}

[shader("fragment")]
[require(spvRayQueryKHR, SPV_KHR_non_semantic_info, spvShaderNonUniformEXT, SPV_GOOGLE_user_type, spvDerivativeControl, spvImageQuery, spvImageGatherExtended, spvSparseResidency, spvMinLod, spvFragmentFullyCoveredEXT)]
float4 fragMain(VSOutput input) : SV_Target {
    float4 baseColor = textures[input.textureID].Sample(input.textureCoord);

    // Alpha cut transparency. Discard the fragment if it is too transparent.
    if (baseColor.a < 0.5) {
        discard;
    }

    float3 p = input.worldPosition;

    float4 finalColor;
    // Darken color if in shadow
    if (inShadow(p)) {
        finalColor = baseColor * 0.2;
    } else {
        finalColor = baseColor;
    }

    return finalColor;
}