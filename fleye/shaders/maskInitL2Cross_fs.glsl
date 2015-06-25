varying vec2 texcoord;

//#define SCORE_TEST 1

float greenMask(vec3 p)
{
	const float greenThreshold = 0.667;

	float rbMax = max( p.x, p.z );
	float rbMin = min( p.x, p.z );
	float greenDiff = (p.y-rbMin);
	float greenRatio = (p.y - rbMax) / (p.y-rbMin);

#ifdef SCORE_TEST
	if( greenDiff>0.05 ) return clamp(greenRatio,0.0,1.0);
	else return 0.0;
#else
	if( greenDiff>0.05 && greenRatio>greenThreshold ) return 1.0;
	else return 0.0;
#endif
}

void main(void)
{
	vec3 ftex =( texture2D(tex, vec2(texcoord.x-xstep*0.5, (1.0-texcoord.y)-ystep*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x-xstep*0.5, (1.0-texcoord.y)+ystep*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+xstep*0.5, (1.0-texcoord.y)-ystep*0.5) ).xyz
			   + texture2D(tex, vec2(texcoord.x+xstep*0.5, (1.0-texcoord.y)+ystep*0.5) ).xyz ) * 0.25;
			   
#ifdef SCORE_TEST
	gl_FragColor.x = greenMask(ftex);
#else
   if( greenMask(ftex) > 0.5 )
    {
		gl_FragColor = vec4(0.5,0.5,0.5,0.5);
	}
	else
	{
		gl_FragColor = vec4(0.0,0.0,0.0,0.0);
	}
#endif
}
