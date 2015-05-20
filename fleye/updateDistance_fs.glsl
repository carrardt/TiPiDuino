uniform sampler2D tex;
varying vec2 texcoord;
uniform float xstep;
uniform float ystep;

/*
storage:
A.x A.y 
A.z A.w


stencil:
  A
B C D
  E
		 A.x A.y
		 A.z A.w
		 
B.x B.y	 C.x C.y  D.x D.y
B.z B.w  C.z C.w  D.z D.w
		 
		 E.x E.y
		 E.z E.w
*/

float updateDistance( float a, float b, float c, float d, float e )
{
	if(c==1.0)
	{
		c = min( min(a,b) , min(d,e) ) + (1.0/64.0);
		if( c > 1.0 ) c = 1.0;
	}
	return c;
}

void main(void)
{
	vec4 A = texture2D(tex, texcoord + vec2(0.0,-ystep) );
	vec4 B = texture2D(tex, texcoord + vec2(-xstep,0.0) );
	vec4 C = texture2D(tex, texcoord + vec2(0.0,0.0) );
	vec4 D = texture2D(tex, texcoord + vec2(xstep,0.0) );
	vec4 E = texture2D(tex, texcoord + vec2(0.0,ystep) );

    gl_FragColor.x = updateDistance( A.z, B.y, C.x, C.y, C.z );
    gl_FragColor.y = updateDistance( A.w, C.x, C.y, D.x, C.w );
    gl_FragColor.z = updateDistance( C.x, B.w, C.z, C.w, E.x );
    gl_FragColor.w = updateDistance( C.y, C.z, C.w, D.z, E.y );
}
