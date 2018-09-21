#version 420

layout(location=0) in vec4 in_Position;
layout(location=1) in vec4 in_Color;

out Vertex {
	vec4 Color;
	vec3 Position;
} Out;

uniform TransBlock {
	mat4 mm, vm, pm, mvpm;
	vec4 eyepos;
} trb;

void main ( void )
{
	vec4 Pos;

	gl_Position = trb.mvpm * in_Position;
	Pos = trb.mm * in_Position;
	Out.Position = Pos.xyz / Pos.w;
	Out.Color = in_Position; //in_Color;
} /*main*/