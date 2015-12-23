//#define SCORE_TEST 1

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	texcoord.y = 1.0 - texcoord.y;
	vec4 S = texture2D( tex_l2cross , texcoord );
	float L = dot( texture2D( tex_camera , texcoord ).xyz , vec3(0.299,0.587,0.114) );

	//float d =  clamp( ( min( min(S.x,S.y) , min(S.z,S.w) ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float d1 =  clamp( max( S.x , S.y ) , 0.0 , 0.5 ) ;
	float d2 =  clamp( max( S.z , S.w ) , 0.0 , 0.5 ) ;
	if(d1<0.5) d1=0.0;
	if(d2<0.5) d2=0.0;
		
	L = clamp( d1+d2 , 0.0 , 1.0 );
	
	gl_FragColor.xyz = yuv_to_rgb( vec3(L,d1,d2) ) ;
	gl_FragColor.w = 1.0;
}
