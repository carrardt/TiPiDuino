uniform sampler2D tex;
uniform highp float xstep; 
uniform highp float xs64; 

varying vec2 texcoord;

highp float unpack(vec2 p)
{
	return p.x * 32.0 + p.y; 
}

vec2 pack(highp float x)
{
	highp float h = floor(x); 
	highp float l = x - h;
	return vec2( h / 32.0 , l );
}

void main(void)
{
	vec4 C = texture2D(tex, texcoord );

	highp float Cl = unpack( C.xy );
	highp float Cr = unpack( C.zw );
	if( (Cr+Cl) > 0.0 )
	{
		vec4 L = texture2D(tex, vec2(texcoord.x-xstep,texcoord.y) );
		highp float Ll = unpack( L.xy );
		highp float Lr = unpack( L.zw );
		if( (Lr+Ll) > 0.0 )
		{
			if( (Lr+Cl) >= xs64 )
			{
				Cl = max( Cl , Ll+xs64 );
				Cr = max( Cr , Lr-xs64 );
			}
		}

		vec4 R = texture2D(tex, vec2(texcoord.x+xstep,texcoord.y) );
		highp float Rl = unpack( R.xy );
		highp float Rr = unpack( R.zw );
		if( (Rr+Rl) > 0.0 )
		{
			if( (Rl+Cr) >= xs64 )
			{
				Cl = max( Cl , Rl-xs64 );
				Cr = max( Cr , Rr+xs64 );
			}
		}
	}
	
	gl_FragColor.xy = pack(Cl);
	gl_FragColor.zw = pack(Cr);
}
