#version 330 core

//Tut4 not tut5
in vec3 fColor;
in vec4 shdwPos;
out vec4 color;

uniform sampler2D shadowMap;

void main() {
   float visibility=1.0;
   if (texture(shadowMap, shdwPos.xy).z < shdwPos.z) {
      visibility=0.5;
   }
   color = vec4(fColor, 1.0)*visibility;
}
