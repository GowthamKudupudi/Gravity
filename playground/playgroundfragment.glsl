#version 330 core

//Tut4 not tut5
in vec3 fColor;

//  //Tut5 not tut4
//  in vec2 UV;
//  uniform sampler2D myTextureSampler;

//Tut2
out vec4 color;

void main() {

//        //Tut2
//        color = vec3(1,0,0);
    //Tut 4 not tut5
   color = vec4(fColor, 1.0);
//   color = vec4(1.0, 1.0, 1.0, 1.0);
//      //Tut 5 not tut4
//      color = texture(myTextureSampler, UV).rgb;
}
