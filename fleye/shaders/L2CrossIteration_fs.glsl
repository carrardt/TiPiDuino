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

	float tx = texcoord.x;
	float ty = texcoord.y;

	float Tx_p = tx + xstep2i;
	float Ty_p = ty + ystep2i;
	
	// 0.0 means Class 1, 1.0 means void
	float ObjClass = clamp( sign(0.5-C.x) , 0.0 , 1.0 );
	
	if( Tx_p<1.0 )
	{
		float nbh = texture2D( tex, vec2(Tx_p,texcoord.y) ).x;
		float NbhClass = clamp( sign(0.5-nbh) , 0.0 , 1.0 );
		if( NbhClass==ObjClass && C.x==nbh ) C.x += UNIT;
	}

	if( Ty_p<1.0 )
	{
		float nbh = texture2D( tex, vec2(texcoord.x,Ty_p) ).y;
		float NbhClass = clamp( sign(0.5-nbh) , 0.0 , 1.0 );
		if( NbhClass==ObjClass && C.y==nbh ) C.y += UNIT;
	}


	gl_FragColor = C;
}
