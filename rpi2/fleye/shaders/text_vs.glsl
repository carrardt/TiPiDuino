varying vec2 texcoord;

void main(void)
{   
   gl_Position = vec4(vertex.x,vertex.y, 0.0, 1.0);
   texcoord = vec2(vertex.z,vertex.w);
}
