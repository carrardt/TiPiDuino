
// return normalized window coordinates in (0,0) - (1,1)
// version based on point sprite specific gl_PointCoord
vec2 normalizedWindowCoordPS()
{
	vec2 normCoord = gl_PointCoord;
	//normCoord.y = 1.0 - normCoord.y;
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

// based on gl_FragCoord
vec2 normalizedWindowCoord()
{
	vec2 normCoord = (gl_FragCoord.xy+vec2(0.5,0.5)) * step;
	//normCoord.y = 1.0 - normCoord.y;
	return normCoord;
}

vec2 pixelWindowCoord()
{
	return gl_FragCoord.xy;
}
