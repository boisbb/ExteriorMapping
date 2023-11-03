#version 460
#extension GL_EXT_nonuniform_qualifier: enable

struct MeshShaderDataFragment {
    vec3 diffuseColor;
    float opacity;
    int textureId;
};

struct FsInput
{
    vec3 fragPosition;
    vec3 fragColor;
    vec3 normal;
    vec2 uv;
}; 

layout(std430, binding = 3) readonly buffer ssbo {
    MeshShaderDataFragment objects[];
} fssbo;

layout(binding = 2) uniform UniformDataFragment {
    vec3 lightPos;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in FsInput fsIn;
layout(location = 5) flat in int instanceId;

layout(location = 0) out vec4 finalColor;

void main() 
{
    vec3 lightPosition = ubo.lightPos;

    vec3 normNormal = normalize(fsIn.normal);
    vec3 lightDirection = normalize(lightPosition - fsIn.fragPosition);

    // TODO: dynamic constants now
    vec3 ambient = 0.3f * vec3(1.f);

    float diffuseCoef = max(dot(lightDirection, normNormal), 0.0f);
    vec3 diffuse = diffuseCoef * vec3(1.f);

    int textureId = fssbo.objects[instanceId].textureId;
    
    float opacity = fssbo.objects[instanceId].opacity;
    if (textureId < 0)
    {
        vec3 diffuseColor = fssbo.objects[instanceId].diffuseColor;
        float opacity = fssbo.objects[instanceId].opacity;
        finalColor = vec4(diffuseColor, opacity);
    }
    else if (textureId >= 0)
    {
        finalColor = texture(texSampler[textureId], fsIn.uv);
        finalColor.a = opacity;
    }

    finalColor.rgb = (ambient + diffuse) * finalColor.rgb;
    // finalColor = vec4(fsIn.fragColor, 1.f);
}