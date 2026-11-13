#version 450

#ifndef FE_FIXED_TEXTURE_ARRAY
#extension GL_EXT_nonuniform_qualifier : require
#endif

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in float v_TilingFactor;
layout(location = 3) flat in float v_TexIndex;
layout(location = 4) flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

#ifdef FE_FIXED_TEXTURE_ARRAY
layout(set = 0, binding = 1) uniform sampler2D u_Textures[16];
#define FE_TEXTURE(index) u_Textures[index]
#else
// Descriptor indexing: partially-bound runtime array sized by the host.
layout(set = 0, binding = 1) uniform sampler2D u_Textures[];
#define FE_TEXTURE(index) u_Textures[nonuniformEXT(index)]
#endif

void main()
{
	int textureIndex = int(v_TexIndex + 0.5);
	o_Color = v_Color * texture(FE_TEXTURE(textureIndex), v_TexCoord * v_TilingFactor);
	o_EntityID = v_EntityID;
}
