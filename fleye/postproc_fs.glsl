uniform sampler2D tex;
varying vec2 texcoord;
uniform float xsize;
uniform float ysize;

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
/*
	int wx = int(texcoord.x*xsize);
	int wy = int(texcoord.y*ysize);
	int tx = int(texcoord.x*xsize*0.5);
	int ty = int(texcoord.y*ysize*0.5);
*/	
	vec4 S = texture2D(tex,texcoord);
	float xp = fract(texcoord.x*xsize*0.5) * 2.0;
	float yp = fract(texcoord.y*ysize*0.5) * 2.0;
	
	float s = dot( S , vec4( (1.0-xp)*(1.0-yp), xp*(1.0-yp), (1.0-xp)*yp, xp*yp ) );
    	gl_FragColor = vec4( rgblut(s), 1.0 );
}
