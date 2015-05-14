#extension GL_OES_EGL_image_external : require
uniform samplerExternalOES tex;
uniform float inv_width;
uniform float inv_height;

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

#define KS 1
#define HK 0.0

void main(void)
{
	float s = 1.0;
	for(int y=0;y<KS;y++)
	{
		float Yd = inv_height * (  float(y) - HK + 0.5 );
		for(int x=0;x<KS;x++)
		{
			float Xd = inv_width * ( float(x) - HK + 0.5 );
			s *= greenMask( texcoord + vec2(Xd,Yd) );
		}
	}
	
	/*if( s > 0 ) s=1.0;
	else s=0.0;*/
	//s *= 4.0;
	vec3 col = rgblut(s);
    gl_FragColor = vec4(col.x,col.y,col.z,1.0);
}
