#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 fragColor[];
in vec4 shadowPos[];
out vec3 fColor;
out vec4 shdwPos;
uniform mat4 VP;
uniform float magnitude;

vec3 normalOf (vec4 tri[3]) {
   vec3 a = tri[2].xyz - tri[0].xyz;
   vec3 b = tri[1].xyz - tri[0].xyz;
   return normalize(cross(a, b));
}

vec4 explode (vec4 position, vec3 normal, float magnitude) {
   vec3 direction = normal * magnitude;
   return position - vec4(direction, 0);
}

// Default main function
void main() {
   vec4 tri[3];
   tri[0] = (gl_in[0].gl_Position);
   tri[1] = (gl_in[1].gl_Position);
   tri[2] = (gl_in[2].gl_Position);
   vec3 normal = normalOf(tri);
   vec3 liDir = vec3(-1.0, 0.0, 0.0);
   float dfusComp = (0.5+dot(liDir,normal))/2;
//   float dfusComp = dot(liDir,normal);
   fColor = fragColor[0]*dfusComp;
   shdwPos = shadowPos[0];
//   gl_Position = tri[0];
   gl_Position = VP*explode(tri[0], normal, magnitude);
   EmitVertex();

   fColor = fragColor[1]*dfusComp;
   shdwPos = shadowPos[1];
//   gl_Position = tri[1];
   gl_Position = VP*explode(tri[1], normal, magnitude);
   EmitVertex();

   fColor = fragColor[2]*dfusComp;
   shdwPos = shadowPos[2];
//   gl_Position = tri[2];
   gl_Position = VP*explode(tri[2], normal, magnitude);
   EmitVertex();
   
   EndPrimitive();

//   gl_Position = MVPgeom[0] *
//      (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[0] *
//      (gl_in[0].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[0] *
//      (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
//   EmitVertex();
//
//   EndPrimitive();
//
//   gl_Position = MVPgeom[1] *
//      (gl_in[1].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[1] *
//      (gl_in[1].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[1] *
//      (gl_in[1].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
//   EmitVertex();
//
//   EndPrimitive();
//
//   gl_Position = MVPgeom[2] *
//      (gl_in[2].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[2] *
//      (gl_in[2].gl_Position + vec4(-1.0, 0.0, 0.0, 0.0));
//   EmitVertex();
//
//   gl_Position = MVPgeom[2] *
//      (gl_in[2].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
//   EmitVertex();
//
//   EndPrimitive();

}
