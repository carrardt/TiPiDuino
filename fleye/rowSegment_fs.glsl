uniform sampler2D tex;
uniform float xsize;
uniform float ysize;
uniform float xstep; // = 2^iteration / xsize

varying vec2 texcoord;

#define inv64 (1.0/64.0)

vec2 pack(float x)
{
	float h = floor( x * inv64 );
	float l = x - (h*64.0);
	return vec2(h,l)*inv64;
}

float unpack(vec2 p)
{
	p *= 64.0;
	return p.x*64.0 + p.y;
}

void main(void)
{
	int txi1 = int( texcoord.x * xsize );
	int txi2 = txi1 + xstep;

	vec4 C = texture2D(tex, texcoord );
	vec4 L = texture2D(tex, vec2(texcoord.x+xstep,texcoord.y) );
	vec4 R = texture2D(tex, vec2(texcoord.x-xstep,texcoord.y) );

	float C_left = unpack(C.xy);
	float C_right = unpack(C.zw);
	float L_left = unpack(L.xy);
	float L_right = unpack(L.zw);
	float R_left = unpack(R.xy);
	float R_right = unpack(R.zw);

	if( L_right >= C_left ) C_left = L_left;
	if( R_left <= C_right ) C_right = R_right;
	
	gl_FragColor.xy = pack( C_left );
	gl_FragColor.zw = pack( C_right );
}
