#version 420

// output fragment color
out vec3 color;

// coordinates of the fragment in range [0,1]
noperspective in vec2 loc;

uniform sampler2D tex;




void main() 
{
	color = texture(tex,loc).rgb;
} 
