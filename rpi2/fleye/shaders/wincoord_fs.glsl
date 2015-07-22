
void main(void)
{
	vec2 winCoord = pixelWindowCoord();
	float r=0.0,g=0.0,b=0.0;
	if( winCoord.x == 0.0 ) r=1.0;
	else if( winCoord.x == (size.x-1.0) ) g=1.0;
	else if( winCoord.y == 0.0 ) b=1.0;
	else if( winCoord.y == (size.y-1.0) ) { g=b=1.0; }
	else
	{
		float px = winCoord.x - floor(winCoord.x*0.5)*2.0;
		float py = winCoord.y - floor(winCoord.y*0.5)*2.0;
		bool c;
		if( winCoord.x > size.x*0.5 ) c=true;
		else c=false;
		if( winCoord.y > size.y*0.5 ) c=!c;
		if( c ) r=g=b=px;
		else r=g=b=py;
	}
	
	gl_FragColor = vec4(r,g,b,1.0);
}
