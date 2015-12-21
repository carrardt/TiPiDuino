
void main(void)
{
	vec2 p = normalizedWindowCoord();
	gl_FragColor = vec4(1.0-(1.0/128.0),1.0/128.0,41.0/128.0,1.0);
}
