#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float Light;
in float Index;
in float TexId;

uniform sampler2D texture1;
uniform vec2 atlasTexCoord;

// Fog
in float visibility;

void main()
{
	if (TexId == 1 && (Index == 1.0 || Index == 3.0 || Index == 4.0 || Index == 6.0))
	{
		FragColor = texture(texture1, vec2((TexCoord.x / 16) + 0, (TexCoord.y / 16) + 0));
	}
	else
	{
		FragColor = texture(texture1, vec2((TexCoord.x / 16) + 0.0625 * (TexId), (TexCoord.y / 16) + 0));
	}

	FragColor *= Light;
	FragColor = mix(vec4(0.3, 0.7, 0.8, 1.0), FragColor, visibility);
}

