
vec3 rgb_to_yuv( vec3 c )
{
	return vec3( 0.299*c.x+0.587*c.y+0.114*c.z, 0.500*c.x-0.419*c.y-0.081*c.z, -0.169*c.x-0.331*c.y+0.500*c.z );
}

vec3 yuv_to_rgb( vec3 yuv )
{
	return vec3( yuv.x + 1.403*yuv.z , yuv.x - 0.344*yuv.y - 0.714*yuv.z , yuv.x + 1.770*yuv.y );
}

void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec3 A = texture2D(tex, texcoord ).xyz;
	vec3 B = texture2D(texPrev, texcoord ).xyz;
	B = B - A;
	float motion = dot(B,B);
	float Y = rgb_to_yuv(A).x; // greyscale value
	gl_FragColor.xyz = yuv_to_rgb( vec3(Y,0.0,motion) );
	gl_FragColor.w = 1.0;
}
