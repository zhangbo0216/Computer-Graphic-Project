/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

// #version 150 

//#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
//      due to different settings of the default GLSL version

attribute  vec4 vPosition;
attribute  vec3 vNormal;
attribute  vec2 vTexCoord;

varying vec4 position;
varying vec3 fPosition;
varying vec4 color;
varying vec2 texCoord;
varying vec2 Sphere_tex2;
varying float Sphere_tex1;
varying vec2 Lattice_tex;

uniform bool Point_Source;

uniform mat4 model_view, projection;
uniform vec4 GlobalAmbient, SurfaceAmbient;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 PointAmbientProduct, PointDiffuseProduct, PointSpecularProduct;
uniform vec4 PointPosition, PointEndPosition;


uniform int verticalFlag, eyeSpaceFlag,sphereTexFlag,upLatticeFlag;

uniform mat3 Normal_Matrix;
uniform vec3 LightDirection;
uniform float Shininess;

uniform float CutOff, Exponent;
uniform float Constant_Attenuation, Linear_Attenuation, Quadratic_Attenuation;

void main() {
    
    
    Sphere_tex2=vec2(0.0,0.0);
    Sphere_tex1=0.0;
    
    // For Point Light Sources
    vec4 point_ambient			= vec4(0.0, 0.0, 0.0, 0.0);
    vec4 point_diffuse			= vec4(0.0, 0.0, 0.0, 0.0);
    vec4 point_specular			= vec4(0.0, 0.0, 0.0, 0.0);
    float point_attenuation		= 0.0;
    
    // For SpotLight Sources
    vec4 spotLight_ambient		= vec4(0.0, 0.0, 0.0, 0.0);
    vec4 spotLight_diffuse		= vec4(0.0, 0.0, 0.0, 0.0);
    vec4 spotLight_specular		= vec4(0.0, 0.0, 0.0, 0.0);
    float spotLight_attenuation = 0.0;
    
    float angle;
    
    position	 = vPosition;
    vec3 pos	 = (model_view * vPosition).xyz;
    vec3 L		 = -normalize(LightDirection.xyz);
    vec3 E		 = normalize(-pos);
    vec3 H		 = normalize(L+E);
    vec3 N		 = normalize(Normal_Matrix * vNormal);
    
    fPosition	 = pos;
    texCoord	 = vTexCoord;
    
    vec4 ambient = AmbientProduct;
    
    float d		 = max(dot(L,N), 0.0);
    vec4 diffuse = d * DiffuseProduct;
    
    float s		 = pow(max(dot(N,H), 0.0), Shininess);
    vec4 specular = s * SpecularProduct;
    
    if (dot(L, N) < 0.0) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    if (Point_Source) {
        // Point Source Lighting
        float dist	= length(PointPosition.xyz - pos);
        vec3 L2		= normalize(PointPosition.xyz - pos);
        vec3 E2		= normalize(-pos);
        vec3 H2		= normalize(L2 + E2);
        
        point_ambient = PointAmbientProduct;
        
        float d2 = max(dot(L2, N), 0.0);
        point_diffuse = d2 * PointDiffuseProduct;
        
        float s2 = pow(max(dot(N, H2), 0.0), Shininess);
        point_specular = s2 * PointSpecularProduct;
        
        // Point Attenuation = 1 / (a + bd + cd^2)
        point_attenuation = 1.0/(Constant_Attenuation + Linear_Attenuation * dist + Quadratic_Attenuation * dist * dist);
        
        if (dot(L2, N) < 0.0){
            point_specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    else {
        // Spot Light Lighting
        float dist	= length(PointPosition.xyz - pos);
        vec3 L_neg	= normalize(PointPosition.xyz - PointEndPosition.xyz);
        vec3 L2		= normalize(PointPosition.xyz - pos);
        vec3 E2		= normalize(-pos);
        vec3 H2		= normalize(L2 + E2);
        
        float cosvalue	= dot(L_neg, L2);
        float phi		= acos(cosvalue);
        angle			= phi * 180.0 / 3.1415926535898;
        
        if (angle < CutOff) {
            spotLight_ambient = PointAmbientProduct;
            
            float d2 = max(dot(L2, N), 0.0);
            spotLight_diffuse = d2 * PointDiffuseProduct;
            
            float s2 = pow(max(dot(N, H2), 0.0), Shininess);
            spotLight_specular = s2 * PointSpecularProduct;
            
            // spotLight_attenuation = 1 / (a + bd + cd^2)
            spotLight_attenuation = (1.0/(Constant_Attenuation + Linear_Attenuation * dist + Quadratic_Attenuation * dist * dist))*pow(max(cosvalue, 0.0), Exponent);
            
            // Change the value of specular if necessary (l dot n) < 0
            if (dot(L2, N) < 0.0) {
                spotLight_specular = vec4(0.0, 0.0, 0.0, 1.0);
            }
        }
        else {
            // If phi > cutoff -> spotLight_attenuation = 0
            spotLight_attenuation = 0.0;
        }
    }
    gl_Position = projection * model_view * vPosition;
    
    // Final Result: L_a * K_a + Attenuation + Specular
    color = GlobalAmbient * SurfaceAmbient + (ambient + specular + diffuse) + point_attenuation * (point_ambient + point_specular + point_diffuse) + spotLight_attenuation * (spotLight_ambient + spotLight_specular + spotLight_diffuse);


        if (verticalFlag==1) {
            if (eyeSpaceFlag==1) {
                Sphere_tex1=2.5*pos.x;
                Sphere_tex2=vec2(0.75 * (pos.x + 1.0), 0.75 * (pos.y + 1.0));
            }
            else {
                Sphere_tex1=2.5 * position.x;
                Sphere_tex2=vec2(0.75 * (vPosition.x + 1.0), 0.75 * (vPosition.y + 1.0));
            }
        }
        else{
            if (eyeSpaceFlag==1) {
            Sphere_tex1=1.5 * (pos.x + pos.y + pos.z);
            Sphere_tex2=vec2(0.45 * (pos.x + pos.y + pos.z), 0.45 * (pos.x - pos.y + pos.z));
        }
        else {
            Sphere_tex1=1.5 * (position.x + position.y + position.z);
            Sphere_tex2=vec2(0.45 * (position.x + position.y + position.z), 0.45 * (position.x - position.y + position.z));
        }
        }
    if (upLatticeFlag==1)
        Lattice_tex= vec2(0.5 * (position.x + 1.0),0.5 * (position.y + 1.0));
    else
        Lattice_tex= vec2(0.3 * (position.x + position.y + position.z),0.3 * (position.x - position.y + position.z));

}