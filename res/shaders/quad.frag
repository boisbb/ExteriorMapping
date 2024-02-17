#version 450

layout(binding=0) uniform QuadUniformBuffer
{
	bool depthOnly;
} ubo;

layout (binding=1) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	if (ubo.depthOnly)
	{
		float n = 0.1f;
		float f = 100.f;
		float z = texture(samplerColor, inUV).r;
		float clipDepth = (2.0 * n) / (f + n - z * (f - n));
		outFragColor = vec4(vec3(clipDepth), 1.0);
	}
	else
	{
		outFragColor = texture(samplerColor, inUV);
	}

}