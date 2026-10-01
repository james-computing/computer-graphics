[[vk::binding(0, 0)]]
Sampler2D textures[]; // combined image samplers (texture image + texture sampler)

struct CameraUBO {
    float4x4 pv;
    float3 location; // for reflections
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

struct Vertex {
    float3 position;
    float3 normal;
    float2 textureCoord;
};

[[vk::binding(0, 6)]]
StructuredBuffer<Vertex> vertexBuffer;

struct VSOutput {
    float4 sv_position : SV_Position;
    float3 worldPosition; // for ray query in fragment shader
    float3 normal; // for reflection. Passing the normal from the vexter shader to the fragment shader can be a problem, because of interpolation.
    uint textureID : TEXID;
    float2 textureCoord;
};

[shader("vertex")]
VSOutput vertMain(
    Vertex input,
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
    output.normal = input.normal;
    return output;
}

static const float3 lightDirection = float3(-0.7071067811865475, 0.7071067811865475, 0.0); // points to the light source
static const float TMIN = 0.01;
static const float TMAX = 1e4;

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
    rayDesc.TMin = TMIN;
    rayDesc.TMax = TMAX;

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
        } else {
            rayQuery.Abort();
        }
    }

    bool hit = (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT);

    return hit;
}

void applyReflection(float3 p, float3 normal, inout float4 baseColor) {
    float3 view = normalize(cameraUBO.location - p); // unit vector from p to the camera
    float3 reflected = reflect(-view, normal); // reflected direction

    RayDesc rayDesc;
    rayDesc.Origin =  p;
    rayDesc.Direction = reflected;
    rayDesc.TMin = TMIN;
    rayDesc.TMax = TMAX;

    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery;
    let rayFlags = RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES;
    rayQuery.TraceRayInline(accelerationStructure, rayFlags, 0xFF, rayDesc);

    // Same as inShadow
    while (rayQuery.Proceed()) {
        uint modelID = rayQuery.CandidateRayInstanceCustomIndex(); // id of the model hit by the ray
        uint triangleIndex = rayQuery.CandidatePrimitiveIndex();
        float2 barycentrics = rayQuery.CandidateTriangleBarycentrics(); // barycentric coordinates of the hit

        float2 uv = intersectionUV(modelID, triangleIndex, barycentrics);
        float4 intersectionColor = textures[NonUniformResourceIndex(modelID)].Sample(uv);
        if (intersectionColor.a >= 0.5) {
            rayQuery.CommitNonOpaqueTriangleHit();
        } else {
            rayQuery.Abort();
        }
    }

    bool hit = (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT);

    // Additional logic
    if (hit) {
        // Use committed instead of candidate
        uint modelID = rayQuery.CommittedRayInstanceCustomIndex();
        uint triangleIndex = rayQuery.CommittedPrimitiveIndex();
        float2 barycentrics = rayQuery.CommittedTriangleBarycentrics();

        float2 uv = intersectionUV(modelID, triangleIndex, barycentrics);
        float4 intersectionColor = textures[NonUniformResourceIndex(modelID)].Sample(uv);

        // blend the baseColor with the reflection color
        baseColor.rgb = lerp(baseColor.rgb, intersectionColor.rgb, 0.7);
    }
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

    // reflection
    // I'll consider that only the first texture is reflective.
    // The correct solution would be to get the property from the material.
    if (input.textureID == 0) {
        // getting the normal from the vertex shader may be a problem, because of interpolation
        applyReflection(p, input.normal, baseColor);
    }

    float4 finalColor;
    // Darken color if in shadow
    if (inShadow(p)) {
        finalColor = baseColor * 0.2;
    } else {
        finalColor = baseColor;
    }

    return finalColor;
}