#define UNIT (1.0/32.0)

/*
 *   w
 * x * y
 *   z
 */

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec4 C = texture2D( tex, texcoord );
	/*if( C.x < 0.5 )
	{
		discard;
	}*/

	float tx = texcoord.x;
	float ty = texcoord.y;

	float Tx_p = tx + step2i.x;
	float Ty_p = ty + step2i.y;
		
	if( Tx_p<1.0 )
	{
		vec4 nbh = texture2D( tex, vec2(Tx_p,texcoord.y) );

		if( nbh.x>0.0 && nbh.x==C.x )
		{
			C.x += UNIT;
		}

		if( nbh.z>0.0 && nbh.z==C.z )
		{
			C.z += UNIT;
		}
	}

	if( Ty_p<1.0 )
	{
		vec4 nbh = texture2D( tex, vec2(texcoord.x,Ty_p) );

		if( nbh.y>0.0 && nbh.y==C.y )
		{
			C.y += UNIT;
		}

		if( nbh.w>0.0 && nbh.w==C.w )
		{
			C.w += UNIT;
		}
	}

	gl_FragColor = C;
}
