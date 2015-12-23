#define UNIT (1.0/32.0)

/*
 *   w
 * x * y
 *   z
 */
float incrementL2( float c, float nbh )
{
	float nbhOk = clamp( sign(nbh-UNIT*0.5) , 0.0 , 1.0 );
	float same = 1.0 - abs(sign(nbh-c));
	return c + same*nbhOk*UNIT;
}

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec4 C = texture2D( tex, texcoord );

	float tx = texcoord.x;
	float ty = texcoord.y;

	float Tx_p = tx + step2i.x;
	float Ty_p = ty + step2i.y;
		
	if( Tx_p<1.0 )
	{
		vec4 nbh = texture2D( tex, vec2(Tx_p,ty) );
		C.x = incrementL2( C.x, nbh.x);
		C.z = incrementL2( C.z, nbh.z);
	}

	if( Ty_p<1.0 )
	{
		vec4 nbh = texture2D( tex, vec2(tx,Ty_p) );
		C.y = incrementL2( C.y, nbh.y);
		C.w = incrementL2( C.w, nbh.w);
	}

	gl_FragColor = C;
}
