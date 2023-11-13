#version 460
#extension GL_EXT_nonuniform_qualifier: enable

struct MeshShaderDataFragment {
    vec3 diffuseColor;
    float opacity;
    int textureId;
    int bumpId;
};

struct FsInput
{
    vec3 fragPosition;
    vec3 fragColor;
    vec3 normal;
    vec2 uv;
    mat3 tbn;
}; 

layout(std430, binding = 3) readonly buffer ssbo {
    MeshShaderDataFragment objects[];
} fssbo;

layout(binding = 2) uniform UniformDataFragment {
    vec3 lightPos;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];
layout(set = 1, binding = 1) uniform sampler2D bumpSampler[];

layout(location = 0) flat in int instanceId;
layout(location = 1) in FsInput fsIn;

layout(location = 0) out vec4 finalColor;

vec3 bumpToNormal(int bumpId)
{
    // src: https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
    // https://stackoverflow.com/questions/22276206/why-does-assimp-fail-to-generate-tangents
    const vec2 size = vec2(2.0, 0.0);
    const ivec3 off = ivec3(-1, 0, 1);

    float intensity = texture(bumpSampler[bumpId], fsIn.uv).x;
    float s11 = intensity;
    float s01 = textureOffset(bumpSampler[bumpId], fsIn.uv, off.xy).x;
    float s21 = textureOffset(bumpSampler[bumpId], fsIn.uv, off.zy).x;
    float s10 = textureOffset(bumpSampler[bumpId], fsIn.uv, off.yx).x;
    float s12 = textureOffset(bumpSampler[bumpId], fsIn.uv, off.yz).x;

    vec3 va = normalize(vec3(size.xy, s21 - s01));
    vec3 vb = normalize(vec3(size.yx, s12 - s10));
    vec4 normal = vec4(cross(va, vb), s11);

    return normalize(normal).xyz;

}

void main() 
{
    vec3 lightPosition = ubo.lightPos;

    vec3 normNormal = vec3(1.0f);
    
    int bumpId = fssbo.objects[instanceId].bumpId;
    if (bumpId >= 0)
    {
        normNormal = bumpToNormal(bumpId);
        normNormal = normalize(fsIn.tbn * normNormal);
        normNormal = normalize(fsIn.normal);
    }
    else
    {
        normNormal = normalize(fsIn.normal);
    }

    vec3 lightDirection = normalize(lightPosition - fsIn.fragPosition);

    // TODO: magic constants now
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
    finalColor.a = 1.f;
    // finalColor = vec4(fsIn.fragColor, 1.f);
}