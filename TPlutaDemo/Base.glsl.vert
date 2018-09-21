#version 420

layout(location=0) in vec4 in_Position;


out Vertex {
vec4 Color;
} Out;

uniform TransBlock {
	mat4 mm, vm, pm, mvpm;
	vec4 eyepos;
} trb;

void main ( void )
{
	gl_Position = trb.mvpm * in_Position; //*/trb.pm * (trb.vm * (trb.mm * in_Position));
	Out.Color = in_Position;
} /*main*/