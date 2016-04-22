/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

attribute  vec4 vPosition;
attribute  vec4 vColor;
attribute  vec2 vTexCoord;

varying vec4 color;
varying vec4 position;
varying vec3 fPosition;
varying vec2 texCoord;
varying vec2 Sphere_tex2;
varying vec2 Lattice_tex;
varying float Sphere_tex1;

uniform mat4 model_view;
uniform mat4 projection;
uniform int  upLatticeFlag;
void main() 
{
   // vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;
    Sphere_tex2=vec2(0.0,0.0);
    Sphere_tex1=0.0;
    vec3 pos	= (model_view * vPosition).xyz;
    position	= vPosition;
    fPosition	= pos;
    color = vColor;
    texCoord	= vTexCoord;
    gl_Position = projection * model_view * vPosition;
    if (upLatticeFlag==1)
        Lattice_tex= vec2(0.5 * (position.x + 1.0),0.5 * (position.y + 1.0));
    else
        Lattice_tex= vec2(0.3 * (position.x + position.y + position.z),0.3 * (position.x - position.y + position.z));
} 
