#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;
 
in  vec3 ex_Color;
out vec4 gl_FragColor;
 
void main(void) {
    // Pass through our original color with full opacity.
    gl_FragColor = vec4(ex_Color.x, ex_Color.y, exColor.z, 1.0);
    //gl_FragColor = vec4(ex_Color.x, ex_Color.y, 0.0, 1.0);
}

