//#define SCORE_TEST 1

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec4 S = texture2D( tex_l2cross , texcoord );
	float L = dot( texture2D( tex_camera , texcoord ).xyz , vec3(0.299,0.587,0.114) );

	//float d =  clamp( ( min( min(S.x,S.y) , min(S.z,S.w) ) - 0.5 ) * 2.0 , 0.0 , 1.0 );
	float o = S.z;
	float r = S.x;
	float u = S.y;
	float m = max(r,u);
	
	float Cu = (1.0-o)*m;
	float Cv = o*m;
		
	L = clamp( L , 0.0 , 1.0 );
	
	gl_FragColor.xyz = yuv_to_rgb( vec3(L,Cu,Cv) ) ;
	gl_FragColor.w = 1.0;
}
