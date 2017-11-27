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
in vec4 lightPosView[];
in float vertexRadius[];


out vec4 fragColor;
out vec4 sphere_center_view;
out vec4 sphere_center_proj;
out vec4 pos_on_sphere_proj;
out float radius_obj_space;

uniform mat4 proj;

void main()
{	

	fragColor = vertexColor[0];
	float radius = vertexRadius[0];	
	radius_obj_space = radius;

	sphere_center_view = gl_in[0].gl_Position;
	sphere_center_proj = proj*sphere_center_view;
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
in vec4 sphere_center_view;
in vec4 sphere_center_proj;
in vec4 pos_on_sphere_proj;
in float radius_obj_space;

out vec4 gl_FragColor;

uniform mat4 view;
uniform float near;
uniform float far;

uniform float ambient;
uniform float diffuse;
uniform float specular;

uniform float screenWidth;
uniform float screenHeight;

uniform vec3 lightPos;

uniform mat4 proj;


void main()
{

	vec2 sphereCenterScreen = vec2(sphere_center_proj.x*screenWidth/2 + screenWidth/2,sphere_center_proj.y*screenHeight/2 + screenHeight/2);
	vec2 posOnSphereScreen = vec2(pos_on_sphere_proj.x*screenWidth/2 + screenWidth/2,pos_on_sphere_proj.y*screenHeight/2 + screenHeight/2);

	float x_d = gl_FragCoord.x -sphereCenterScreen.x;
	float y_d = gl_FragCoord.y -sphereCenterScreen.y;
	float radius_screen = abs(posOnSphereScreen.x - sphereCenterScreen.x);

	// MAPS TO [-1,1]
	vec2 P_screen = vec2(x_d,y_d)/radius_screen;
	float P_screen_mag = length(P_screen);

	// CIRCLE MEMBERSHIP
	if(P_screen_mag > 1.0){
		discard;
	}

	vec3 color = fragColor.rgb;
	float alpha = fragColor.a;

	// NORMAL RECONSTRUCTION
	float z_screen_comp = 1.0-abs(P_screen_mag);
	vec3 normal_frag = vec3(x_d,y_d,z_screen_comp);
	// This doesnt work... why?
	//vec3 normal_view = mat3(transpose(view))*normal_frag;
	//vec3 normal_obj = mat3(transpose(proj))*normal_view;
	vec3 normal_obj = mat3(transpose(view*proj))*normal_frag;
	vec3 normal_view = mat3(transpose(view))*normal_obj;
	vec3 normal_obj_normalized = normalize(normal_obj);

	// DEPTH RECONSTRUCTION
	vec4 normal_obj_scaled = vec4(normal_obj_normalized,0.0)*radius_obj_space;
	vec4 S_viewspace = sphere_center_view + normal_obj_scaled;
	vec4 S_screenspace = proj*S_viewspace;
	float ndc_depth = S_screenspace.z / S_screenspace.w;
	float depth = ndc_depth * 2.0 - 1.0;
	gl_FragDepth = depth;
	//gl_FragDepth =  gl_FragCoord.z; // popping artefacts


	// BLINN_PHONG
	float shininess = 64.0;
	vec3 normal_view_normalized = normalize(normal_view);
	vec4 lightPos_view = view*vec4(lightPos,1.0);
	lightPos_view = normalize(-lightPos_view);
	vec3 viewDir = normalize(-S_viewspace.xyz);
	vec3 lightDir = normalize(lightPos_view.xyz+viewDir);

	float lambertian = max(dot(normal_view_normalized,lightDir),0.0);

	vec3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(halfDir,normal_view_normalized),0.0);
	float spec = pow(specAngle,shininess);

	gl_FragColor = vec4(ambient * color,alpha);
	gl_FragColor += vec4(diffuse * lambertian * color,0.0);
	gl_FragColor += vec4(specular*spec*color,0.0);


	//gl_FragColor = vec4(normal_obj_normalized,1.0);
	//gl_FragColor = vec4(normal_view_normalized,1.0);
	//gl_FragColor = vec4(vec3(-1*normal_view_normalized.z),1.0);
	//gl_FragColor = vec4(viewDir,1.0);
	//gl_FragColor = vec4(color);
	//gl_FragColor = fragColor;

}
