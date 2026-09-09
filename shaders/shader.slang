struct VSInput {
    float3 position;
    //float3 color;
    float2 textureCoord;
};

struct CameraUBO {
    float4x4 pv;
};


struct ObjectUBO {
    float4x4 model;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    float3 color;
    float2 textureCoord;
};

[[vk::binding(0, 0)]]
//ParameterBlock<Sampler2D> texture;
Sampler2D texture;

[[vk::binding(0, 1)]]
//ParameterBlock<ConstantBuffer<CameraUBO>> cameraUBO;
ConstantBuffer<CameraUBO> cameraUBO;

[[vk::binding(0, 2)]]
//ParameterBlock<ConstantBuffer<ObjectUBO>> objectUBO;
ConstantBuffer<ObjectUBO> objectUBO;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float4 vertexPosition = float4(input.position, 1.0);
    output.sv_position = mul(cameraUBO.pv, mul(objectUBO.model, vertexPosition));
    //output.color = input.color;
    output.textureCoord = input.textureCoord;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
    //float3 color = vertIn.color;
    //return float4(color, 1.0);

    return texture.Sample(vertIn.textureCoord);
}