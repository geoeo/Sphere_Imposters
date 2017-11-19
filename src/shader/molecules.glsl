// glsw (OpenGL GLSL Shader Wrangler) allows to define multiple GLSL shaders, that are linked together, in a single file.
// Follow comments in the cpp file (glwidget) for more information.
// The "--Vertex" etc. indicate where one shader begins/ends,
// for example the vertex shader begins at "--Vertex" and ends at "--Geometry"

//////////////////////////////////////////////////////
-- Vertex

#extension GL_ARB_explicit_attrib_location : enable

// variables
in vec3 atomPos;
in vec3 inputColor;
in float inputRadius;

out vec4 vertexColor;
out float vertexRadius;

uniform mat4 view;

void main(void)
{
	vertexColor = vec4(inputColor,1.0);
	vertexRadius = inputRadius;

	gl_Position = view*vec4(atomPos,1.0f);

}

//////////////////////////////////////////////////////
-- Geometry

// variables
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 vertexColor[];
in float vertexRadius[];
out vec4 fragColor;

uniform mat4 proj;

void main()
{	

	fragColor = vertexColor[0];
	float radius = vertexRadius[0];	

	gl_Position = proj*(gl_in[0].gl_Position + vec4(-radius, -radius, 0.0, 0.0));
    EmitVertex();

	gl_Position = proj*(gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0.0));
    EmitVertex();

    gl_Position = proj*(gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0.0));
    EmitVertex();

	gl_Position = proj*(gl_in[0].gl_Position + vec4(radius, radius, 0.0, 0.0));
    EmitVertex();


    EndPrimitive();

}  

//////////////////////////////////////////////////////
-- Fragment

// variables
in vec4 fragColor;
out vec4 gl_FragColor;


void main()
{
	gl_FragColor = fragColor;
}
