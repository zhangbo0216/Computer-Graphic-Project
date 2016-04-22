


in  vec4 vPosition;
in  vec4 vColor;
in  vec4 vVelocity;

out vec4 color;
out float disappear;

uniform mat4  model_view, projection;
uniform float t;

void main() {
    // Acceleration (a) = (0.5)*(9.8)*(10^-7)*(t^2) = (4.9)*(10^-7)*(t^2)
    float a = (-4.9) * pow(10.0,-7.0);
    
    // According to Newton's Law:
    float x = vPosition.x + vVelocity.x * 0.001 * t;
    float y = vPosition.y + vVelocity.y * 0.001 * t + 0.5 * a * t * t;
    float z = vPosition.z + vVelocity.z * 0.001 * t;
    
    // Should the particle be displayed?
    // If the particle has dropped to Y < 0.1 (in the world frame),
    //	we stop rendering the particle
    if (y < 0.1) {
        disappear = 1.0;
    }
    else {
        disappear = -1.0;
    }
    
    // Update the position of the particle
    vec4 Position = vec4(x, y, z, 1.0);
    gl_Position = projection * model_view  * Position;
    
    // Keep the particle's color unchanged
    color = vColor;
}