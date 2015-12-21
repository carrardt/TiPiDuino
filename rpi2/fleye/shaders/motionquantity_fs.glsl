
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
