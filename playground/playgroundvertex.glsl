#version 330 core

//  Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
//Tut4 not tut5
//out vec3 fragmentColor;

////Tut5 not tut4
//  layout(location=1) in vec2 vertexUV;
//  out vec2 UV;

uniform mat4 M;
uniform mat4 VP;
uniform float zHalfMax;
uniform float magnitude;

out mat4 VPgeom;
out float magGeom;
out vec3 fragColor;

void main(){
   //Tut2
   //gl_Position = MVP * vec4(vertexPosition_modelspace,1);
   gl_Position = M*vec4(vertexPosition_modelspace, 1);
   VPgeom = VP;
   magGeom = magnitude;
   fragColor = vertexColor;
   //Tut4
   //float color = 3*(vertexPosition_modelspace.z+zHalfMax)/(2*zHalfMax)-1;
   /* fragmentColor.x=color>1?0:color; */
   /* fragmentColor.y=color>2?0:(color>1?color-1:0); */
   /* fragmentColor.z=color>2?color-2:0; */
//   fragmentColor = vec3(1.0,1.0,1.0);
   //      //Tut5
   //      UV =  vertexUV;
}
