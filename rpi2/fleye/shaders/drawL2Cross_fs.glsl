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
	vec2 texcoord = normalizedWindowCoord();
	texcoord.y = 1.0 - texcoord.y;
	vec4 S = texture2D( tex_l2cross , texcoord );
	float L = dot( texture2D( tex_camera , texcoord ).xyz , vec3(0.299,0.587,0.114) );

	//float d =  clamp( ( min( min(S.x,S.y) , min(S.z,S.w) ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d1 =  clamp( ( max( S.x , S.y ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d2 =  clamp( ( max( S.z , S.w ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	
	vec3 bgColor = vec3(L,L,L);
	if( d1>0.125 ) bgColor = vec3(0.0,0.5,0.0);
	else if( d2>0.125 ) bgColor = vec3(0.5,0.0,0.0);
	
	gl_FragColor.xyz = bgColor ;
	gl_FragColor.w = 1.0;
}
