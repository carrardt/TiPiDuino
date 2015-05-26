uniform sampler2D tex;
uniform float xstep; 
uniform float xsteppix; 

varying vec2 texcoord;

highp float unpack(vec2 p)
{
	return exp2( p.x * 16.0 ) * p.y; 
}

void main(void)
{
	vec4 C = texture2D(tex, texcoord );
	
	highp float Cl = unpack( C.xy );
	highp float Cr = unpack( C.zw );
	if( (Cr-Cl) > 0.0 )
	{
		if(texcoord.x>=xstep)
		{
			vec4 L = texture2D(tex, vec2(texcoord.x-xstep,texcoord.y) );
			highp float Ll = unpack( L.xy );
			highp float Lr = unpack( L.zw );
			if( (Lr-Ll) > 0.0 )
			{
				if( (Lr+Cl) >= xsteppix )
				{
					C.xy = L.xy;
				}
			}
		}
		if((texcoord.x+xstep)<=1.0)
		{
			vec4 R = texture2D(tex, vec2(texcoord.x+xstep,texcoord.y) );
			highp float Rl = unpack( R.xy );
			highp float Rr = unpack( R.zw );
			if( (Rr-Rl) > 0.0 )
			{
				if( (Rl+Cr) >= xsteppix )
				{
					C.zw = R.zw;
				}
			}
		}
	}
	
	gl_FragColor = C;
}
