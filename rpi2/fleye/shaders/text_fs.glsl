varying vec2 texcoord;

void main(void)
{
	vec3 color = texture2D(font, texcoord ).xyz;
	float alpha = dot(color,color);
	gl_FragColor = vec4( color, alpha );
}
