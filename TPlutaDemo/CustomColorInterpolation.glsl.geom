#version 420

layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;

in Vertex {
	vec4 Color;
	vec3 Position;
} In[];

out NVertex {
	vec4 Color;
	vec3 Position;
	vec3 Normal;
} Out;

void main ( void )
{
	int i;
	vec3 v1, v2, nv;
	v1 = In[1].Position - In[0].Position;
	v2 = In[2].Position - In[0].Position;
	nv = normalize ( cross ( v1, v2 ) );
	for ( i = 0; i < 3; i++ ) {
		gl_Position = gl_in[i].gl_Position;
		Out.Position = In[i].Position;
		Out.Normal = nv;
		Out.Color = In[i].Color;
		EmitVertex ();
	}
	EndPrimitive ();
} /*main*/