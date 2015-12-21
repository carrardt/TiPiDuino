
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

vec3 rgb_to_yuv( vec3 c )
{
	return vec3( 0.299*c.x+0.587*c.y+0.114*c.z, 0.500*c.x-0.419*c.y-0.081*c.z, -0.169*c.x-0.331*c.y+0.500*c.z );
}

vec3 yuv_to_rgb( vec3 yuv )
{
	return vec3( yuv.x + 1.403*yuv.z , yuv.x - 0.344*yuv.y - 0.714*yuv.z , yuv.x + 1.770*yuv.y );
}
