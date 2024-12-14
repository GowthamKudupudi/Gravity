#version 330 core

layout(location=0) out float fragmentdepth;

void main () {
   // Not really needed, OpenGL does it anyway
   // changing it has no effect
   fragmentdepth = gl_FragCoord.z;
}
