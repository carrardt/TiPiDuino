attribute vec2 vertex;
attribute vec2 tcoord;
varying vec2 texcoord;
void main(void)
{
   texcoord = tcoord;
   gl_Position = vec4(vertex, 0.0, 1.0);
}
