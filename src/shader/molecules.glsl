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
out vec4 lightPosView;
out float vertexRadius;

uniform mat4 view;
uniform vec3 lightPos;

void main(void)
{
	vertexColor = vec4(inputColor,1.0);
	lightPosView = view * vec4(lightPos,1.0f);
	vertexRadius = inputRadius;

	gl_Position = view*vec4(atomPos,1.0f);

}

//////////////////////////////////////////////////////
-- Geometry

// variables
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 vertexColor[];
in vec4 lightPosView[];
in float vertexRadius[];


out vec4 fragColor;
out vec4 sphere_center_proj;
out vec4 pos_on_sphere_proj;
out vec4 lightPos;
out float radius_obj_space;

uniform mat4 proj;

void main()
{	

	fragColor = vertexColor[0];
	float radius = vertexRadius[0];	
	radius_obj_space = radius;

	lightPos = lightPosView[0];
	lightPos /= lightPos.w;

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
in vec4 lightPos; // viewSpace;
in float radius_obj_space;

out vec4 gl_FragColor;


uniform float near;
uniform float far;

uniform float ambient;
uniform float diffuse;
uniform float specular;

uniform float screenWidth;
uniform float screenHeight;

//uniform mat4 view_fragshader;
uniform mat4 proj_inv;

void main()
{

	vec2 sphereCenterScreen = vec2(sphere_center_proj.x*screenWidth/2 + screenWidth/2,sphere_center_proj.y*screenHeight/2 + screenHeight/2);
	vec2 posOnSphereScreen = vec2(pos_on_sphere_proj.x*screenWidth/2 + screenWidth/2,pos_on_sphere_proj.y*screenHeight/2 + screenHeight/2);

	float x_d = gl_FragCoord.x -sphereCenterScreen.x;
	float y_d = gl_FragCoord.y -sphereCenterScreen.y;
	float radius_screen = abs(posOnSphereScreen.x - sphereCenterScreen.x);

	float circle_test = x_d*x_d + y_d*y_d - radius_screen*radius_screen;

	if(circle_test > 0){
		discard;
	}

	float color = fragColor.x;

	vec4 normal_frag = vec4(x_d,y_d,circle_test,0.0);
	vec4 normal_view = proj_inv*normal_frag;
	vec3 normal_view_nomalized = normalize(normal_view.xyz);

	float depth_inner = (x_d*x_d + y_d*y_d);
	//float depth_linear = gl_FragCoord.z - sqrt(radius_screen + depth_inner);
	//float depth_linear = gl_FragCoord.z + circle_test*radius_obj_space;
	float depth_linear = gl_FragCoord.z ;
	float ndc_depth = depth_linear;
	//float ndc_depth = depth_linear * gl_FragCoord.w;
	//float depth = (((far-near) * ndc_depth) + near + far) / 2.0;
	float depth = ndc_depth;
	gl_FragDepth = depth;
	//gl_FragDepth = gl_FragCoord.z;


	vec4 viewDir = proj_inv*vec4(gl_FragCoord.x,gl_FragCoord.y, depth_linear,1.0);
	//vec4 viewDir = proj_inv*vec4(gl_FragCoord.x,gl_FragCoord.y, gl_FragCoord.z,1.0);
	viewDir /= viewDir.w;
	vec3 viewDirNorm = normalize(viewDir.xyz);

	vec3 lightDirNorm = -1*normalize(lightPos.xyz);

	vec3 halfDir = normalize(lightDirNorm + viewDirNorm);
	float specAngle = max(dot(halfDir,normal_view_nomalized),0.0);
	float spec = pow(specAngle,specular);

	float lambertian = max(dot(lightDirNorm,normal_view_nomalized),0.0);

	//gl_FragColor  = vec4(ambient * color + lambertian * color + specular*color);


	gl_FragColor = vec4(normal_view_nomalized,1.0);
	//gl_FragColor = vec4(viewDirNorm,1.0);
	//gl_FragColor = vec4(color);

}
