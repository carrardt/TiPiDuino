attribute vec2 vertex;
varying vec2 texcoord;
void main(void)
{
   texcoord = ( vertex + vec2(1,1) ) * 0.5;
   gl_Position = vec4(vertex, 0.0, 1.0);
}
