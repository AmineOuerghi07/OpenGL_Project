// macOS / Apple OpenGL ~4.1 path — no bindless, no GLSL 460, no SSBO.
// Windows Editor keeps Texture.glsl (#version 460 + GL_ARB_bindless_texture).

#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140) uniform Camera
{
	mat4 u_ViewProjection;
};

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TilingFactor;
flat out float v_TexIndex;
flat out int v_EntityID;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TilingFactor = a_TilingFactor;
	v_TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 410 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TilingFactor;
flat in float v_TexIndex;
flat in int v_EntityID;

uniform sampler2D u_Textures[16];

void main()
{
	vec4 texColor = v_Color * texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor);
	color = texColor;
	color2 = v_EntityID;
}
