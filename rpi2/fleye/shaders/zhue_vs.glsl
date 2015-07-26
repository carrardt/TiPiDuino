vec3 rgblut(float x)
{
	if( x < 0.3333 )
	{
		x = x * 3.0;
		return vec3(1.0-x,x,0.0);		
	}
	else if( x < 0.6666 )
	{
		x = (x-0.3333) * 3.0;
		return vec3(0.0,1.0-x,x);
	}
	else
	{
		x = (x-0.6666) * 3.0;
		return vec3(x,0.0,1.0-x);
	}

	//return vec4(0.25*x,x,0.25*x,1.0);
}

varying vec3 color;

void main(void)
{
   gl_Position = vec4(vertex.xy, 0.0, 1.0);
   color = rgblut(vertex.z);
   // gl_PointSize = max( size.x , size.y );
}
