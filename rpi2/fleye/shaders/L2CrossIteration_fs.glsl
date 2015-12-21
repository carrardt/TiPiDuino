#define UNIT (1.0/128.0)

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
	
	// 0.0 means Class 1, 1.0 means void
	float Obj1Class = clamp( sign(0.5-C.x) , 0.0 , 1.0 );
	float Obj2Class = clamp( sign(0.5-C.z) , 0.0 , 1.0 );
	
	if( Tx_p<1.0 )
	{
		vec4 RightNbh = texture2D( tex, vec2(Tx_p,texcoord.y) );

		float Nbh1Class = clamp( sign(0.5-RightNbh.x) , 0.0 , 1.0 );
		if( Nbh1Class==Obj1Class && C.x==RightNbh.x ) C.x += UNIT;

		float Nbh2Class = clamp( sign(0.5-RightNbh.z) , 0.0 , 1.0 );
		if( Nbh2Class==Obj2Class && C.z==RightNbh.z ) C.z += UNIT;		
	}

	if( Ty_p<1.0 )
	{
		vec4 UpNbh = texture2D( tex, vec2(texcoord.x,Ty_p) );

		float Nbh1Class = clamp( sign(0.5-UpNbh.y) , 0.0 , 1.0 );
		if( Nbh1Class==Obj1Class && C.y==UpNbh.y ) C.y += UNIT;

		float Nbh2Class = clamp( sign(0.5-UpNbh.w) , 0.0 , 1.0 );
		if( Nbh2Class==Obj2Class && C.w==UpNbh.w ) C.w += UNIT;		
	}

	gl_FragColor = C;
}
