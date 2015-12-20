
void main(void)
{
	vec2 texcoord = normalizedWindowCoord();
	gl_FragColor = texture2D(tex, vec2(texcoord.x,1.0-texcoord.y) );
}
