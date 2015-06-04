uniform sampler2D tex;
uniform float xstep; 
uniform float ystep;

varying vec2 texcoord;

#define UNIT (1.0/32.0)

/*
 *   w
 * x * y
 *   z
 */

void main(void)
{
	vec4 C = texture2D( tex, texcoord );
	/*if( C.x < 0.5 )
	{
		discard;
	}*/

	if( (texcoord.x+xstep) < 1.0 )
	{
		if( texture2D( tex, vec2(texcoord.x+xstep,texcoord.y) ).x == C.y ) C.y += UNIT;
	}

	if( (texcoord.x-xstep) > 0.0 )
	{
		if( texture2D( tex, vec2(texcoord.x-xstep,texcoord.y) ).y == C.x ) C.x += UNIT;
	}

	if( (texcoord.y+ystep) < 1.0 )
	{
		if( texture2D( tex, vec2(texcoord.x,texcoord.y+ystep) ).z == C.w ) C.w += UNIT;
	}

	if( (texcoord.y-ystep) > 0.0 )
	{
		if( texture2D( tex, vec2(texcoord.x,texcoord.y-ystep) ).w == C.z ) C.z += UNIT;
	}

	gl_FragColor = C;
}
