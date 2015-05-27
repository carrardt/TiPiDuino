#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
uniform float xstep;
uniform float xsize;

varying vec2 texcoord;

// meilleur, valide jusqu'à 2048. nickel
vec2 pack(highp float x)
{
	x /= 64.0;
	highp float h = floor(x); 
	highp float l = x - h;
	return vec2( h / 32.0 , l );
}

float greenMask(vec2 tcoord)
{
	const float greenThreshold = 0.5;
    vec3 p = texture2D(tex, tcoord ).xyz ;

	float rbMax = max( p.x, p.z );
	float rbMin = min( p.x, p.z );
	float greenDiff = (p.y-rbMin);
	float greenRatio = (p.y - rbMax) / (p.y-rbMin);
	
	//if( psq > 0.2 ) return 1.0;
	//if( psq > 0.1 && greenRatio>1.0 ) return 1.0-(1.0/greenRatio);
	if( greenDiff>0.05 && greenRatio>greenThreshold ) return 1.0;
	else return 0.0;
}

void main(void)
{
    if( greenMask(texcoord) > 0.5 )
    {
		/*
		highp float winx = floor(texcoord.x*xsize);
		gl_FragColor.xy = pack( winx );
		gl_FragColor.zw = pack( winx + 1.0 );
		*/
		gl_FragColor.xy = pack( 0.0);
		gl_FragColor.zw = pack( 1.5 );
	}
	else
	{
		gl_FragColor = vec4(0.0,0.0,0.0,0.0);
	}
}
