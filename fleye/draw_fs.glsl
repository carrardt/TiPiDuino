uniform sampler2D tex;
varying vec2 texcoord;
uniform float xsize;
uniform float ysize;
uniform float xsize_inv;
uniform float ysize_inv;

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
	// unpack rgba packed values
	int winx = int( texcoord.x * xsize );
	int hwinx = winx/2;
	int winx2 = hwinx*2;
	float tx = float(winx2) * xsize_inv;
	float xp = float( winx - winx2 );
	
	int winy = int( texcoord.y * ysize );
	int hwiny = winy/2;
	int winy2 = hwiny*2;
	float ty = float(winy2) * ysize_inv;
	float yp = float( winy - winy2 );

	float nxp = 1.0-xp;
	float nyp = 1.0-yp;

	vec4 S = texture2D(tex, texcoord );

	float s = dot( S, vec4(nxp*nyp,xp*nyp,nxp*yp,xp*yp) );
    gl_FragColor = vec4( rgblut(s), 1.0 );
}
