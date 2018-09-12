#version 420

#define MAX_NLIGHTS 8

in NVertex {
	vec4 Colour;
	vec3 Position;
	vec3 Normal;
} In;

out vec4 out_Colour;

uniform TransBlock {
	mat4 mm, vm, pm, mvpm;
	vec4 eyepos;
} trb;

struct LSPar {
	vec4 ambient;
	vec4 direct;
	vec4 position;
	vec4 attenuation;
};
uniform LSBlock {
	uint nls; /* liczba Ÿróde³ œwiat³a */
	uint mask; /* maska w³¹czonych Ÿróde³ */
	LSPar ls[MAX_NLIGHTS]; /* poszczególne Ÿród³a œwiat³a */
} light;

vec3 posDifference ( vec4 p, vec3 pos, out float dist )
{
	vec3 v;
	if ( p.w != 0.0 ) {
		v = p.xyz/p.w-pos.xyz;
		dist = sqrt ( dot ( v, v ) );
	}
	else
		v = p.xyz;
	return normalize ( v );
} /*posDifference*/

float attFactor ( vec4 att, float dist )
{
	return 1.0/(((att.z*dist)+att.y)*dist+att.x);
} /*attFactor*/

void main ( void )
{
	vec3 normal, lv, vv;
	float d, e, dist;
	uint i, mask;
	vec4 NewColor;

	float prop;
	if (In.Colour.w == 0.0f) out_Colour.xyzw = vec4(0.0,0.0,0.0,0.0);
	else{
	if (In.Colour.x >= 0.8f){
			prop = (In.Colour.x - 0.8f)/(0.2f)*0.5;
			NewColor.xyzw = vec4(1,0,prop,In.Colour.w);	
	}
	else if (In.Colour.x >=0.45f){
		prop = (In.Colour.x - 0.45f)/(0.35f);
		NewColor.xyzw = vec4(1,1 - prop,0,In.Colour.w);
	}
	else if (In.Colour.x >=0.30f){
		prop = (In.Colour.x - 0.30f)/(0.15f)*0.5;
		NewColor.xyzw = vec4(0.5+prop,1,0,In.Colour.w);
	}
	else if (In.Colour.x >=0.22f){
		prop = (In.Colour.x - 0.22f)/(0.08f)*0.5;
		NewColor.xyzw = vec4(prop,1,0,In.Colour.w);
	}
	else if (In.Colour.x >=0.08f) {
		prop = (In.Colour.x - 0.08f)/(0.14f);
		NewColor.xyzw = vec4(0,1,1-prop,In.Colour.w);
	}
	else{
		prop = In.Colour.x/0.08f;
		NewColor.xyzw = vec4(0,prop,1 ,In.Colour.w);
	}

	normal = normalize ( In.Normal );
	vv = posDifference ( trb.eyepos, In.Position, dist );
	e = dot ( vv, normal );
	out_Colour = vec4(0.0);
	for ( i = 0, mask = 0x00000001; i < light.nls; i++, mask <<= 1 )
		if ( (light.mask & mask) != 0 ) {
			out_Colour += light.ls[i].ambient * NewColor;
			lv = posDifference ( light.ls[i].position, In.Position, dist );
			d = dot ( lv, normal );
			if ( e > 0.0 ) {
				if ( d > 0.0 ) {
					if ( light.ls[i].position.w != 0.0 )
						d *= attFactor ( light.ls[i].attenuation, dist );
					out_Colour += (d * light.ls[i].direct) * NewColor;
				}
			}
			else {
				if ( d < 0.0 ) {
					if ( light.ls[i].position.w != 0.0 )
						d *= attFactor ( light.ls[i].attenuation, dist );
					out_Colour -= (d * light.ls[i].direct) * NewColor;
				}
			}
		}
	}
	//out_Colour.a = 1.0;
} /*main*/