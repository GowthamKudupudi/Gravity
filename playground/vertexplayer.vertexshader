#version 330 core

//  Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
//Tut4 not tut5
layout(location = 1) in vec3 vertexColor;
out vec3 fragmentColor;

////Tut5 not tut4
//  layout(location=1) in vec2 vertexUV;
//  out vec2 UV;

uniform mat4 MVP;
void main(){
    //Tut2
    gl_Position = MVP * vec4(vertexPosition_modelspace,1);
    //Tut4
    fragmentColor=vertexColor;
//      //Tut5
//      UV =  vertexUV;
}

