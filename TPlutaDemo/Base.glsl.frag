#version 420

in Vertex {
vec4 Color;
} In;

out vec4 out_Color;

void main ( void )
{
out_Color = In.Color;
		
} /*main*/