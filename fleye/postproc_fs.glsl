uniform sampler2D tex;
varying vec2 texcoord;

vec3 rgblut(float x)
{
	if( x <= 0.5 )
	{
		x = x * 2.0;
		return vec3(x,x,1.0);
	}
	else
	{
		x = (1.0-x)*2.0;
		return vec3(1.0,x,x);
	}
}

void main(void)
{
    gl_FragColor = texture2D(tex,texcoord);
}
