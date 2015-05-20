#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
uniform float xstep;
uniform float ystep;

varying vec2 texcoord;

float greenMask(vec2 tcoord)
{
    vec3 p = texture2D(tex, tcoord ).xyz ;
    vec3 pn = normalize(p);
    float psq = dot(p,p);

	float redblue = max( pn.x, pn.z );
	float greenRatio = pn.y / redblue;
	//if( psq > 0.2 ) return 1.0;
	//if( psq > 0.1 && greenRatio>1.0 ) return 1.0-(1.0/greenRatio);
	if( psq > 0.01 && greenRatio>=2.0 ) return 1.0;
	else return 0.0;
}

// sampling matrix :
// A B C D
// E F G H
// I J K L
// M N O P
// we're computing values for F,G,J and K
// stored as respectively R,G,B and A of final color

void main(void)
{
    gl_FragColor.x = greenMask( texcoord + vec2( 0.0		, 0.0 ) );
    gl_FragColor.y = greenMask( texcoord + vec2( xstep		, 0.0 ) );
    gl_FragColor.z = greenMask( texcoord + vec2( 0.0		, ystep ) );
    gl_FragColor.w = greenMask( texcoord + vec2( xstep		, ystep) );
}
