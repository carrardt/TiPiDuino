uniform sampler2D tex;
varying vec2 texcoord;
uniform float xsize;
uniform float ysize;

#define inv64 (1.0/64.0)

vec2 pack(float x)
{
	float h = floor( x * inv64 );
	float l = x - (h*64.0);
	return vec2(h,l)*inv64;
}

void main(void)
{	
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
	
	if( s>0.5 )
	{
		gl_FragColor.xy = pack(winx);
		gl_FragColor.zw = pack(winx+1);
	}
	else
	{
		gl_FragColor = vec4(0.0,0.0,0.0,0.0);
	}
}
