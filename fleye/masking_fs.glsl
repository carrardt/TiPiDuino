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
	if( psq > 0.01 && greenRatio>1.0 ) return 1.0-(1.0/greenRatio);
	//if( psq > 0.001 && greenRatio>2.0 ) return 1.0;
	else return 0.0;
}

// sampling matrix :
// A B C D
// E F G H
// I J K L
// M N O P
// we're computing values for F,G,J and K
// stored as R,G,B,A of final color

void main(void)
{
	float A = greenMask( texcoord + vec2( -xstep	, -ystep ) );
	float B = greenMask( texcoord + vec2( 0.0		, -ystep ) );
	float C = greenMask( texcoord + vec2( xstep		, -ystep ) );
	float D = greenMask( texcoord + vec2( xstep*2.0	, -ystep ) );
	
	float E = greenMask( texcoord + vec2( -xstep	, 0.0 ) );
	float F = greenMask( texcoord + vec2( 0.0		, 0.0 ) );
	float G = greenMask( texcoord + vec2( xstep		, 0.0 ) );
	float H = greenMask( texcoord + vec2( xstep*2.0	, 0.0 ) );
	
	float I = greenMask( texcoord + vec2( -xstep	, ystep ) );
	float J = greenMask( texcoord + vec2( 0.0		, ystep ) );
	float K = greenMask( texcoord + vec2( xstep		, ystep) );
	float L = greenMask( texcoord + vec2( xstep*2.0	, ystep ) );
	
	float M = greenMask( texcoord + vec2( -xstep	, ystep*2.0 ) );
	float N = greenMask( texcoord + vec2( 0.0		, ystep*2.0 ) );
	float O = greenMask( texcoord + vec2( xstep		, ystep*2.0 ) );
	float P = greenMask( texcoord + vec2( xstep*2.0	, ystep*2.0 ) );
	
    gl_FragColor.x = A*B*C*E*F*G*I*J*K;
    gl_FragColor.y = B*C*D*F*G*H*J*K*L;
    gl_FragColor.z = E*F*G*I*J*K*M*N*O;
    gl_FragColor.w = F*G*H*J*K*L*N*O*P;
}
