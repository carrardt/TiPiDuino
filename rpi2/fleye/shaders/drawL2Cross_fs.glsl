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
	vec4 P = texture2D( tex_camera , texcoord );

	//float d =  clamp( ( min( min(S.x,S.y) , min(S.z,S.w) ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d1 =  clamp( ( max( S.x , S.y ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d2 =  clamp( ( max( S.z , S.w ) - 0.5 ) * 2.0 , 0.0 , 1.0 );

	vec2 v0 = texcoord-vec2( obj0Center.x, obj0Center.y );
	float G = clamp( 1.0-dot(v0,v0)*1024.0 , 0.0, 1.0 );

	vec2 v1 = texcoord-vec2( obj1Center.x, obj1Center.y );
	float R = clamp( 1.0-dot(v1,v1)*1024.0 , 0.0, 1.0 );
	
	vec3 bgColor = P.xyz;
	if( d1>0.125 ) bgColor = vec3(0.0,0.5,0.0);
	if( d2>0.125 ) bgColor = vec3(0.5,0.0,0.0);
	
	gl_FragColor.xyz = bgColor + vec3(G*0.5,G,G*0.5) + vec3(R,R*0.5,R*0.5);
	gl_FragColor.w = 1.0;
}
