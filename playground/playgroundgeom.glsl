#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

in mat4 MVPgeom[];

// Default main function
void main() {
   gl_Position = MVPgeom[0] * gl_in[0].gl_Position;
   EmitVertex();
   
   gl_Position = MVPgeom[1] * gl_in[1].gl_Position;
   EmitVertex();

   gl_Position = MVPgeom[2] * gl_in[2].gl_Position;
   EmitVertex();

   EndPrimitive();

   gl_Position = MVPgeom[0] * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[0] * (gl_in[0].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[0] * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
   EmitVertex();

   EndPrimitive();

   gl_Position = MVPgeom[1] * (gl_in[1].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[1] * (gl_in[1].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[1] * (gl_in[1].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
   EmitVertex();

   EndPrimitive();

   gl_Position = MVPgeom[2] * (gl_in[2].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[2] * (gl_in[2].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
   EmitVertex();

   gl_Position = MVPgeom[2] * (gl_in[2].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
   EmitVertex();

   EndPrimitive();

}
