
void main(void)
{
	vec2 p = normalizedWindowCoord();
	gl_FragColor = vec4(p.x,0.0,1.0,0.5);
}
