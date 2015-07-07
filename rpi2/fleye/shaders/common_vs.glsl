void main(void)
{
   gl_Position = vec4(vertex, 0.0, 1.0);
   gl_PointSize = max( size.x , size.y );
}
