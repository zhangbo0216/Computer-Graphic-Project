/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150


varying  vec2 texCoord;
varying  vec3 fPosition;
varying  vec4 color;
varying  vec4 position;
varying  vec2 Sphere_tex2,Lattice_tex;
varying float Sphere_tex1;

uniform int fogFlag;
uniform sampler2D texture_2D;
uniform sampler1D texture_1D;

uniform int textureFlag, sphereTexFlag,draw,element;
uniform int verticalFlag, eyeSpaceFlag;
uniform int latticeFlag, upLatticeFlag;
void main()
{
    
     float s, t;
     float toDiscard = 0.35;
     vec4 t_color;
     // Lattice Effect is applied to the sphere & shadow
     if (latticeFlag==1) {
         s=Lattice_tex.x;
         t=Lattice_tex.y;
     if (draw==2) {
     // Discard fragment if both s and t are less than toDiscard = 0.35
     
             s = fract(4.0 * s);
     
             t = fract(4.0 * t);
     
             if (s < toDiscard && t < toDiscard) {
     
                 discard;
     
             }
     
         }
     }
    
     
     /*--- Fog Effect ---*/
    
    float start   = 0.0;
    float end	  = 18.0;
    float density = 0.09;
    
    vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5);
    vec4 redColor = vec4(0.9, 0.1, 0.1, 1.0);
    
    float z       = length(fPosition);
    vec3 pos      = fPosition;
    
    
        if (draw==1&&textureFlag==1)
            t_color = color * texture2D(texture_2D, vec2(texCoord.x * 20.0 / 4.0, texCoord.y * 24.0 / 4.0));
        else if (draw==1)
            t_color=color;
        else if (sphereTexFlag == 1) 					// Contour Texture
            t_color = color * texture1D(texture_1D, Sphere_tex1);
        else if (sphereTexFlag == 2)
        {
            t_color = texture2D(texture_2D, Sphere_tex2);
            if (t_color.x < 1.0) {
                t_color =color * redColor;
            }
            else {
                t_color =color * t_color;
            }
        }
        else t_color = color;
    
        if (fogFlag > 0) {
        float fogFactor;	// fogFactor was just "f" in the notes
        
        // Solving for the fog factor based on settings
        if (fogFlag == 1) {
            // Linear Fog:				f = (end - z) / (end - start)
            fogFactor = (end - z)/(end - start);
        }
        else if (fogFlag == 2) {
            // Exponential Fog:			f = e^(-1.0 * density * z)
            fogFactor = exp(-1.0 * density * z);
        }
        else if (fogFlag == 3) {
            // Exponential Square Fog:	f = e^(-(density * z)^2)
            fogFactor = exp(-(density * z)*(density * z));
        }
        fogFactor = clamp(fogFactor, 0.0, 1.0);
                                                        // No Texture
            gl_FragColor = mix(fogColor, t_color, fogFactor);
        }else
            gl_FragColor=t_color;

        
    }
