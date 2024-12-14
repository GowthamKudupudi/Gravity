#version 330 core

//Tut4 not tut5
in vec3 fColor;
in vec4 shdwPos;
in float dfusComp;
out vec4 color;

uniform sampler2D shadowMap;
//uniform sampler2DShadow shadowMap;

void main() {
   float visibility=1.0;
   float bias = 0.005;
   if (dfusComp>0.0) {
      if (texture(shadowMap, shdwPos.xy).x<shdwPos.z-bias) {
         visibility=0.5;
      }
      /* for (int i=0; i<4; i++) { */
      /*    // use either : */
      /*    //  - Always the same samples. */
      /*    //    Gives a fixed pattern in the shadow, but no noise */
      /*    int index = i; */
      /*    //  - A random sample, based on the pixel's screen location.  */
      /*    //    No banding, but the shadow moves with the camera, which looks weird. */
      /*    // int index = int(16.0*random(gl_FragCoord.xyy, i))%16; */
      /*    //  - A random sample, based on the pixel's position in world space. */
      /*    //    The position is rounded to the millimeter to avoid too much aliasing */
      /*    // int index = int(16.0*random(floor(Position_worldspace.xyz*1000.0), i))%16; */
		
      /*    // being fully in the shadow will eat up 4*0.2 = 0.8 */
      /*    // 0.2 potentially remain, which is quite dark. */
      /*    float shd = texture( */
      /*       shadowMap, */
      /*       vec3(shdwPos.xy,// + poissonDisk[index]/700.0, */
      /*            (shdwPos.z-bias)/shdwPos.w)); */
      /*    visibility -= 0.2*(1.0-shd); */
      /* } */
   }
   color = vec4(fColor*dfusComp*visibility+0.1, 1.0);
}
