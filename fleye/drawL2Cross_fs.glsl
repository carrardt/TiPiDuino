uniform sampler2D tex;
varying vec2 texcoord;

#define UNIT (1.0/32.0)

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
	//if( abs(left-right)<=(1.0/64.0) && (left+right)>=(1.0/4.0)) gl_FragColor = vec4(1.0,1.0,1.0,1.0);
	//else gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	if( S.x >= 0.5 )
	{
		S -= vec4(0.5,0.5,0.5,0.5);
		float h =  min(S.x,S.y) * 2.0;
		float v =  min(S.z,S.w) * 2.0;
		gl_FragColor = rgblut( min(h,v) );
	}
	else
	{
		gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	}

}
