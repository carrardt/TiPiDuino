void main(void)
{
	vec2 normCoord = normalizedWindowCoord();
	float r=0.0,g=0.0,b=0.0;
	if( normCoord.x < step.x ) r=1.0;
	else if( normCoord.x > (1.0-step.x) ) g=1.0;
	else if( normCoord.y < step.y ) b=1.0;
	else if( normCoord.y > (1.0-step.y) ) { g=b=1.0; }
	
	gl_FragColor = vec4(r,g,b,1.0);
}
