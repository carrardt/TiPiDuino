uniform sampler2D tex;
varying vec2 texcoord;

highp float unpack(vec2 p)
{
	return exp2( p.x * 16.0 ) * p.y; 
}

vec4 rgblut(float x)
{

	if( x <= 0.5 )
	{
		x = x * 2.0;
		return vec4(x,x,1.0,1.0);
	}
	else
	{
		x = (1.0-x)*2.0;
		return vec4(1.0,x,x,1.0);
	}

	//return vec4(0.25*x,x,0.25*x,1.0);
}

void main(void)
{
	vec4 S = texture2D(tex, texcoord );
	highp float start = unpack(S.xy) ;
	highp float end = unpack(S.zw) ;
	gl_FragColor = rgblut( (end-start)/16.0 );
}
