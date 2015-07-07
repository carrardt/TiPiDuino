// return normalized window coordinates in (0,0) - (1,1)
vec2 normalizedWindowCoord()
{
	vec2 normCoord = gl_PointCoord;
	normCoord.y = 1.0 - normCoord.y;
	if( size.x > size.y )
	{
		normCoord.y = 0.5 + ( (normCoord.y-0.5) * ( size.x / size.y ) );
	}
	if( size.y > size.x )
	{
		normCoord.x = 0.5 + ( (normCoord.x-0.5) * ( size.y / size.x ) );
	}
	return normCoord;
}

vec2 pixelWindowCoord()
{
	return gl_FragCoord.xy;
}
