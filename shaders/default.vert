#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLight;
layout (location = 3) in float aIndex;
layout (location = 4) in float aTexId;

out vec2 TexCoord;
out float Light;
out float Index;
out float TexId;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Fog
out float visibility;
const float density = 0.007;
const float gradient = 8.2;

void main()
{
	vec4 positionRelativeToCam = view * model * vec4(aPos, 1.0);
	float distance = length(positionRelativeToCam.xyz);
	visibility = exp(-pow((distance*density), gradient));
	visibility = clamp(visibility, 0.0, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Light = aLight;
    Index = aIndex;
	TexId = aTexId;
}
