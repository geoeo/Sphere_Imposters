// glsw (OpenGL GLSL Shader Wrangler) allows to define multiple GLSL shaders, that are linked together, in a single file.
// Follow comments in the cpp file (glwidget) for more information.
// The "--Vertex" etc. indicate where one shader begins/ends,
// for example the vertex shader begins at "--Vertex" and ends at "--Geometry"

//////////////////////////////////////////////////////
-- Vertex

#extension GL_ARB_explicit_attrib_location : enable

// variables
layout(location = 0) in vec3 atomPos;

uniform mat4 view;
uniform mat4 proj;

//out vec4 vertexColor;

void main(void)
{
	vec3 test = vec3(9.0,0.0,20.0);
	gl_Position = proj*view*vec4(atomPos,1.0f);
	//vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}

//////////////////////////////////////////////////////
-- Fragment

// variables
out vec4 gl_FragColor;
//in vec4 vertexColor;


void main()
{
	gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}
