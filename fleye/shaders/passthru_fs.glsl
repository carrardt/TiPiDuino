
void main(void)
{
	vec2 tc = gl_PointCoord;
	gl_FragColor = texture2D(tex, tc );
}
