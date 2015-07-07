
void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	gl_FragColor = texture2D(tex, texcoord );
}
