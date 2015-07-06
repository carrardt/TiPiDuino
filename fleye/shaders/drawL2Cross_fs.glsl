varying vec2 texcoord;

//#define SCORE_TEST 1

#define UNIT (1.0/32.0)

vec3 rgblut(float x)
{
	if( x < 0.3333 )
	{
		x = x * 3.0;
		return vec3(0.1,0.1,x);		
	}
	else if( x < 0.6666 )
	{
		x = (x-0.3333) * 3.0;
		return vec3(x,x,1.0);
	}
	else
	{
		x = (x-0.6666) * 3.0;
		x = 1.0 - x;
		return vec3(1.0,x,x);
	}

	//return vec4(0.25*x,x,0.25*x,1.0);
}

void main(void)
{
	vec4 S = texture2D(tex, texcoord );

	//float d =  clamp( ( min( min(S.x,S.y) , min(S.z,S.w) ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d1 =  clamp( ( max( S.x , S.y ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d2 =  clamp( ( max( S.z , S.w ) - 0.5 ) * 2.0 , 0.0 , 1.0 );

	vec2 v0 = texcoord-vec2( obj0Center.x, obj0Center.y );
	float G = clamp( 1.0-dot(v0,v0)*1024.0 , 0.0, 1.0 );

	vec2 v1 = texcoord-vec2( obj1Center.x, obj1Center.y );
	float R = clamp( 1.0-dot(v1,v1)*1024.0 , 0.0, 1.0 );
	
	gl_FragColor.xyz = vec3(d2*2.0,d1*2.0,d1+d2) + vec3(-G,G,-G) + vec3(R,-R,-R);
	gl_FragColor.w = 1.0;
}
