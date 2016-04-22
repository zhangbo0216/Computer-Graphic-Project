

in vec4   color;
in float disappear;

out vec4 fColor;
void main() {
    // Conditional discard: Check if the fragment should disappear, if so discard it
    //if (disappear>0.0) { discard; }
    
    // Display the fragment
    fColor = color;
}