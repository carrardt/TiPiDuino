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
	vec4 S = texture2D(tex,texcoord);
	
	float winx = texcoord.x * xsize + 0.5;
	float winy = texcoord.y * ysize + 0.5;
	float xp = fract(winx*0.5)*2.0;
	float yp = fract(winy*0.5)*2.0;
	float nxp = 1.0-xp;
	float nyp = 1.0-yp;

	float s = dot( S, vec4(nxp*nyp,xp*nyp,nxp*yp,xp*yp) );

    gl_FragColor = vec4( rgblut(s), 1.0 );
}
