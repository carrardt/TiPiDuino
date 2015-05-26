uniform sampler2D tex;
varying vec2 texcoord;
uniform float xsize;
uniform float ysize;
uniform float ccmd_inv;

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
	const float inv64 = 1.0/64.0;
	// unpack rgba packed values
	int winx = int( texcoord.x * xsize );
	int hwinx = winx/2;
	int winx2 = hwinx*2;
	float xp = float( winx - winx2 );

	int winy = int( texcoord.y * ysize );
	int hwiny = winy/2;
	int winy2 = hwiny*2;
	float yp = float( winy - winy2 );

	float nxp = 1.0-xp;
	float nyp = 1.0-yp;

	vec4 S = texture2D(tex, texcoord );

	float s = dot( S, vec4(nxp*nyp,xp*nyp,nxp*yp,xp*yp) );
	if( s < inv64 ) gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	else if(s>(1.0-inv64)) gl_FragColor = vec4( 0.25,1.0,0.25, 1.0 );
	else gl_FragColor = rgblut(s*64.0*ccmd_inv);

	//gl_FragColor = rgblut(s);
}
