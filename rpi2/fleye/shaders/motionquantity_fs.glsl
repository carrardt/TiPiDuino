
void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	vec3 A = texture2D(tex, texcoord ).xyz;
	vec3 B = texture2D(texPrev, texcoord ).xyz;
	B = B - A;
	float motion = dot(B,B);
	float L = dot( A , vec3(0.299,0.587,0.114) ); // greyscale value
	gl_FragColor.xyz = vec3( L+motion, L-motion, L-motion ); // tainted red where motion is high
	gl_FragColor.w = 1.0;
}
