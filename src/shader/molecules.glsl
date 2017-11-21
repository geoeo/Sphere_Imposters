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
out vec4 sphere_center_proj;
out vec4 pos_on_sphere_proj;

uniform mat4 proj;

void main()
{	

	fragColor = vertexColor[0];
	float radius = vertexRadius[0];	

	sphere_center_proj = proj*gl_in[0].gl_Position;
	sphere_center_proj /= sphere_center_proj.w;

	pos_on_sphere_proj = proj*(gl_in[0].gl_Position + vec4(radius, 0.0, 0.0, 0.0));
	pos_on_sphere_proj /= pos_on_sphere_proj.w;

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
in vec4 sphere_center_proj;
in vec4 pos_on_sphere_proj;

out vec4 gl_FragColor;

uniform float ambient;
uniform float diffuse;
uniform float specular;

uniform float screenWidth;
uniform float screenHeight;

void main()
{


	vec4 color = fragColor;
	vec2 sphereCenterScreen = vec2(sphere_center_proj.x*screenWidth/2 + screenWidth/2,sphere_center_proj.y*screenHeight/2 + screenHeight/2);
	vec2 posOnSphereScreen = vec2(pos_on_sphere_proj.x*screenWidth/2 + screenWidth/2,pos_on_sphere_proj.y*screenHeight/2 + screenHeight/2);

	float x_d = gl_FragCoord.x -sphereCenterScreen.x;
	float y_d = gl_FragCoord.y -sphereCenterScreen.y;
	float radius_screen = abs(posOnSphereScreen.x - sphereCenterScreen.x);


	float circle_test = x_d*x_d + y_d*y_d - radius_screen*radius_screen;

	if(circle_test > 0){
		discard;
	}

	gl_FragColor = color;

}
