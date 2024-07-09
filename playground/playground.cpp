/*                              Column index
12345678902134567890312456789041235678905123467890612345789071234568908123456790
 */
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <LASlib/lasreader.hpp>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Cartesian.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/boost/graph/graph_traits_Delaunay_triangulation_2.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <unordered_map>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;
float width=1800.0f;
float height=1600.0f;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/controls.hpp"

using namespace glm;
using namespace std;
using namespace CGAL;
typedef Exact_predicates_inexact_constructions_kernel            K;
//typedef Cartesian<float>            K;
typedef Projection_traits_xy_3<K>                                PrjTrts;
typedef Triangulation_vertex_base_with_info_2<unsigned, PrjTrts> Vb2;
typedef Triangulation_face_base_with_info_2<int, PrjTrts>        Fb2;
typedef K::Point_3                                               Point3;
//typedef K::Vector_3                                              Vec3;
typedef Surface_mesh<Point3>                                     Mesh;
typedef Triangulation_data_structure_2<Vb2>                      T2ds;
//typedef Triangulation_data_structure_2<Vb2, Fb2>                 T2ds;
typedef Delaunay_triangulation_2<PrjTrts, T2ds>                  TIN;
//typedef Delaunay_triangulation_2<PrjTrts>                        TIN;

// Initial position : on +Z
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
float oldVA = 0.0f;
float theta = (0.0f);
// Initial Field of View
float initialFoV = 114.0f; //45.0f; // Human eye 114
float initialCamZPos = 100.0f; // 20000.0 to look at earth
float speed = 25.0f; // 1500 km / second
float mouseSpeed = 0.005f;
float minDisplayRange = 0.008f;     // 100m
float maxDisplayRange = sqrt (pow (initialCamZPos, 2) * 3); // 20,000km
glm::vec3 camPosition = glm::vec3 ( 0, 0, initialCamZPos );
float G = 6.674 * pow (10,-20); // km3.kg-1.s-2
float D = 5510 * pow (10,9); // kg.km-3
float WD = 1000 * pow (10,9); // kg.km-3
float R = 6000;//6371; //km
glm::vec3 direction;
double lastTime = glfwGetTime();
double currentTime = lastTime;
float deltaTime = 0;
double lastPrintTime = lastTime;
double lastShootTime = lastTime;


#include "common/shader.hpp"
//Tut5
#include "common/texture.hpp"
//Tut6
#include "common/text2D.hpp"
#include "world.hpp"

#define octahedron_HT 0.816496580927726
#define EQ_TRIANGLE_HT 0.866025403784439
void NormalSmooth (
   const GLfloat* gfSolid, unsigned int uiSolidSize,
   GLfloat* gfSphere, unsigned int uiSphereSize,
   unsigned short iRegression=2, unsigned short usDimensions=3
) {
   if (uiSphereSize < (pow(4,iRegression)*uiSolidSize)) return;
   int i=0,j=0;
   unsigned int uiTempSolidFillSize=uiSolidSize;
   unsigned int uiTempSolidSize=uiSphereSize;
   GLfloat* gfTempSolid = (GLfloat*)malloc(uiTempSolidSize*sizeof(*gfSolid));
   memcpy(gfTempSolid, gfSolid, uiTempSolidFillSize*sizeof(*gfSolid));
   GLfloat gfTempTriangle[3*3];
   for (int r = 0; r < iRegression; ++r) {
      i=0;
      j=0;
      while (i<uiTempSolidFillSize) {
         // generate mid veritces
         int k=0;
         int n=0;
         GLfloat gfLen;
         for (k=0; k<3; ++k) {
            // for each dimension
            gfLen = 0;
            for (n=0; n < usDimensions; n++) {
               gfTempTriangle[3*k+n] = gfTempSolid[(3*k)+i+n] +
               gfTempSolid[3*((k+1)%3)+i+n];
               gfLen += pow (gfTempTriangle [3*k+n], 2);
            }
            gfLen = sqrt (gfLen);
            // normalize length
            for (n=0; n < usDimensions; ++n) gfTempTriangle[3*k+n]/=gfLen;
         }
         // group vertices with mid vertices
         // each vertex
         for (k=0; k<3; ++k) {
            // at each dimension
            for (n = 0; n < 3; ++n) {
               gfSphere[j+n]=gfTempSolid[i+3*((k+1)%3)+n];
               gfSphere[j+3+n]=gfTempTriangle[3*k+n];
               gfSphere[j+6+n]=gfTempTriangle[3*((k+1)%3)+n];
            }
            j+=9;
         }
         // vertices of triangle with mid points
         for (k=0; k<9; ++k, ++j)
            gfSphere [j] = gfTempTriangle [k];
         i += 9;
      }
      uiTempSolidFillSize=j;
      memcpy (gfTempSolid, gfSphere, uiTempSolidFillSize*sizeof(*gfTempSolid));
   }
}

mat4 rotate (mat4 r, vec3 v);

int main( void ) {
   if( !glfwInit() ) {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      getchar();
      return -1;
   }
   
   glfwWindowHint (GLFW_SAMPLES, 4);
   glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
   // To make MacOS happy; should not be needed
   glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
   window = glfwCreateWindow(
      (int)::width, (int)height, "Playground", NULL, NULL);
   if ( window == NULL ) {
      fprintf ( stderr, "Failed to open GLFW window. If you have an Intel GPU,"
      "they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
      getchar ();
      glfwTerminate ();
      return -1;
   }
   glfwMakeContextCurrent (window);
   World* pWorld = World::create_world (window);
   glewExperimental = true;
   if (glewInit() != GLEW_OK) {
      fprintf (stderr, "Failed to initialize GLEW\n");
      getchar ();
      glfwTerminate ();
      return -1;
   }
   
   glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
   //tut6
   // Hide the mouse and enable unlimited mouvement
   glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   // Set the mouse at the center of the screen
   glfwPollEvents ();
   glfwSetCursorPos (window, ::width/2, height/2);
   
   // Dark blue background
   glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
   
   // Enable depth test
   glEnable (GL_DEPTH_TEST);
   // Accept fragment if it closer to the camera than the former one
   glDepthFunc (GL_LESS);
   
   GLuint VertexArrayID;
   glGenVertexArrays (1, &VertexArrayID);
   glBindVertexArray (VertexArrayID);
   
   GLuint programID = LoadShaders ( "vertexshader.glsl",
                                    "fragmentshader.glsl" );
   GLuint MatrixID = glGetUniformLocation ( programID, "MVP");
   GLuint zHalfMaxID = glGetUniformLocation ( programID, "zHalfMax");
   
   static const GLfloat g_vertex_buffer_data [] = {
      -0.5f, -1.0f, -1.0f,
       0.5f, -1.0f, -1.0f,
       0.0f,  1.0f, -1.0f,
   };
   
   static const GLfloat g_cube_vertex_buffer_data [] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
      -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
       1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
       1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
       1.0f,-1.0f,-1.0f,
       1.0f, 1.0f,-1.0f,
       1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
       1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
       1.0f,-1.0f, 1.0f,
       1.0f, 1.0f, 1.0f,
       1.0f,-1.0f,-1.0f,
       1.0f, 1.0f,-1.0f,
       1.0f,-1.0f,-1.0f,
       1.0f, 1.0f, 1.0f,
       1.0f,-1.0f, 1.0f,
       1.0f, 1.0f, 1.0f,
       1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f,
       1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
       1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
       1.0f,-1.0f, 1.0f
   };
   static const GLfloat g_octahedron_vertex_buffer_data [] = {
       0.0f, 1.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       0.0f, 0.0f,-1.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f,-1.0f,
      -1.0f, 0.0f, 0.0f,
       0.0f,-1.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 1.0f,
       0.0f,-1.0f, 0.0f,
       0.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 0.0f,
       0.0f,-1.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       0.0f, 0.0f,-1.0f,
       0.0f,-1.0f, 0.0f,
       0.0f, 0.0f,-1.0f,
      -1.0f, 0.0f, 0.0f
   };
   static const GLfloat gfTriangleBufferData [] = {
       0.0f,  1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f
   };
   static const unsigned gfTriangleElementData [] = {
      0, 1, 2
   };
   static const GLfloat gfTriColorBufferData [] = {
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f
   };
   unordered_map<Point3, unsigned> mapP3;
   vector<vec3> vecV3;
   vector<unsigned> vecElms;
   LASreadOpener lasreadopener;
   lasreadopener.set_file_name("/home/Necktwi/workspace/displaz/SU785700.las");
   LASreader* lasreader = lasreadopener.open();
   if (!lasreader) {
      printf("file not found!\n");
      return 0;
   }
   LASpoint& pt = lasreader->point;
   size_t i=0;
   vec3 centroid;
   vec3 bboxMax;
   vec3 bboxMin;
   while (lasreader->read_point()) {
      if ((pt.classification | (pt.synthetic_flag << 5) |
          (pt.keypoint_flag << 6) | (pt.withheld_flag << 7))!=2) {
         Point3 p3(pt.get_x(), pt.get_y(), pt.get_z());
         vec3 v3(pt.get_x(), pt.get_y(), pt.get_z());
         if (!i)
            centroid=v3;
         else {
            centroid+=v3;
            centroid/=2;
         }
         vecV3.push_back(v3);
         mapP3[p3]=i++;
         if (i>=1000)
            break;
      }
   }
   printf("mapP3 size: %ld\n"
          "sizeof(Point3): %u\n"
          "sizeof(vec3): %u\n"
          "centroid: %f,%f,%f\n", mapP3.size(), sizeof(Point3), sizeof(vec3),
          centroid.x, centroid.y, centroid.z);
   TIN dsm(mapP3.begin(), mapP3.end());
   Mesh sm;
   CGAL::copy_face_graph(dsm, sm);
   for (Mesh::Face_index fi : sm.faces()) {
      for (Mesh::Vertex_index vi:
              vertices_around_face(sm.halfedge(fi), sm)) {
         vecElms.push_back(mapP3[sm.point(vi)]);
      }
   }
   for (unsigned i=0; i<vecV3.size(); ++i) {
      vecV3[i]-=centroid;
      bboxMax.x=bboxMax.x>vecV3[i].x?bboxMax.x:vecV3[i].x;
      bboxMin.x=bboxMin.x<vecV3[i].x?bboxMin.x:vecV3[i].x;
      bboxMax.y=bboxMax.x>vecV3[i].y?bboxMax.y:vecV3[i].y;
      bboxMin.y=bboxMin.x<vecV3[i].y?bboxMin.y:vecV3[i].y;
      bboxMax.z=bboxMax.z>vecV3[i].z?bboxMax.z:vecV3[i].z;
      bboxMin.z=bboxMin.z<vecV3[i].z?bboxMin.z:vecV3[i].z;
   }
   float zHalfMax = bboxMax.z>-bboxMin.z?bboxMax.z:-bboxMin.z;
   zHalfMax/=2;
   printf("vecElms.size: %u\n"
          "zHalfMax: %f\n"
          "bboxMax: %f,%f,%f\n"
          "bboxMin: %f,%f,%f\n", vecElms.size(), zHalfMax,
          bboxMax.x,bboxMax.y,bboxMax.z, bboxMin.x, bboxMin.y, bboxMin.z);
   GLuint pcVertexBuffer;
   glGenBuffers(1, &pcVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, pcVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vecV3.size(),
                 &vecV3[0], GL_STATIC_DRAW);
   // glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*9, &gfTriangleBufferData[0],
   //              GL_STATIC_DRAW);
   GLuint pcElmBuffer;
   glGenBuffers(1, &pcElmBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pcElmBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*vecElms.size(),
                &vecElms[0], GL_STATIC_DRAW);
   // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*3,
   //             &gfTriangleElementData[0], GL_STATIC_DRAW);
   glUniformMatrix4fv(zHalfMaxID, 1, GL_FALSE, &zHalfMax);
   
   glUseProgram (programID);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
   do {
      lastTime = currentTime;
      currentTime = glfwGetTime();
      deltaTime = currentTime - lastTime;
      computeMatricesFromInputs();
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      mat4 scale (1.0f);
      mat4 rot (1.0f);
      mat4 trans (1.0f);
      rot = rotate (rot, vec3(0.0f));
      scale = glm::scale (scale, vec3(1));
      trans = translate (mat4(1.0f), vec3(0.0f));
      
      mat4 model (1.0f);
      model = trans * scale * rot;
      mat4 MVP = ProjectionMatrix * ViewMatrix * model;
      glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP [0][0]);
      glEnableVertexAttribArray (0);
      glBindBuffer (GL_ARRAY_BUFFER, pcVertexBuffer);
      glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pcElmBuffer);
      glDrawElements(
         GL_TRIANGLES,      // mode
         vecElms.size(),    // count
//         3,
         GL_UNSIGNED_INT,   // type
         (void*)0           // element array buffer offset
      );
      glDisableVertexAttribArray(0);
      glfwSwapBuffers (window);
      glfwPollEvents ();
   } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0);
   glDeleteBuffers (1, &pcElmBuffer);
   glDeleteBuffers (1, &pcVertexBuffer);
   return 0;
   static const unsigned int uiNumSphericalVertices = 3*8*4*4*4*4*4;
   static const unsigned int uiNumAstVertices = 3*8*4*4*4;
   GLfloat gfSphereVertexBufferData [3*uiNumSphericalVertices];
   NormalSmooth (g_octahedron_vertex_buffer_data, 3*3*8,
                 gfSphereVertexBufferData, 3*uiNumSphericalVertices, 5);
   GLfloat gfAstVertexBufferData [3*uiNumAstVertices];
   NormalSmooth (g_octahedron_vertex_buffer_data, 3*3*8, gfAstVertexBufferData,
                  3*uiNumAstVertices, 3);
   // One color for each vertex. They were generated randomly.
   float r, g, b;
   GLfloat earthColBufData [3*uiNumSphericalVertices];
   for (int v = 0; v < uiNumSphericalVertices; ++v) {
      //193
      if ((v>767 && v<1536) || (v>3071 && v<3840) ||
         (v>13055 && v<13824) || (v>15359 && v<16128)
      ) {
         if ((v>1151&&v<1344) || (v>3455 && v<3648) || (v>13439 && v<13632) ||
            (v>15743 && v<15936)
         ) {
            earthColBufData[3*v+2] = 1.0f;
            continue;
         }
         earthColBufData[3*v+0] = 1.0f;
         earthColBufData[3*v+1] = 1.0f;
         continue;
      } else if (!v%10000) {
         r=(float)(rand()%5)/5;
         g=(float)(rand()%5)/5;
         b=(float)(rand()%5)/5;
      }
      r += 0.001f;
      r = r >= 1.0f ? (r-1.0f) : r;
      earthColBufData[3*v+0] = r;
      g += 0.001f;
      g = g >= 1.0f ? (g - 1.0f) : g;
      earthColBufData [3*v+1] = g;
      b += 0.001f;
      b = b >= 1.0f ? (b - 1.0f) : b;
      earthColBufData [3*v+2] = b;
   }
   GLfloat astColBufData [3*uiNumAstVertices];
   //    r=(float)(rand()%5)/5;
   //    g=(float)(rand()%5)/5;
   //    b=(float)(rand()%5)/5;
   for (int v = 0; v < uiNumAstVertices; ++v) {
      r += 0.1f;
      r = r >= 1.0f ? (r - 1.0f) : r;
      astColBufData [3*v+0] = r;
      g += 0.1f;
      g = g >= 1.0f ? (g - 1.0f) : g;
      astColBufData [3*v + 1] = r;
      b += 0.1f;
      b = b >= 1.0f ? (b - 1.0f) : b;
      astColBufData [3 * v + 2] = r;
   }
   static const GLfloat gfCubeColorBufferData [] = {
      1.0f, 1.0f, .0f,
      1.0f, 1.0f, .0f,
      1.0f, 1.0f, .0f,
      1.0f, 1.0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f, .0f, .0f,
      1.0f,.0f,.0f,
      1.0f,.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      1.0f,1.0f,.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f,
      .0f,1.0f,1.0f
      
   };
   
   static GLfloat g_red_color_buffer_data[]={
      1.0f,   0.0f,   0.0f,
      1.0f,   0.0f,   0.0f,
      1.0f,   0.0f,   0.0f
   };
   static const GLfloat g_uv_buffer_data[] = {
      0.000059f, 1.0f-0.000004f,
      0.000103f, 1.0f-0.336048f,
      0.335973f, 1.0f-0.335903f,
      1.000023f, 1.0f-0.000013f,
      0.667979f, 1.0f-0.335851f,
      0.999958f, 1.0f-0.336064f,
      0.667979f, 1.0f-0.335851f,
      0.336024f, 1.0f-0.671877f,
      0.667969f, 1.0f-0.671889f,
      1.000023f, 1.0f-0.000013f,
      0.668104f, 1.0f-0.000013f,
      0.667979f, 1.0f-0.335851f,
      0.000059f, 1.0f-0.000004f,
      0.335973f, 1.0f-0.335903f,
      0.336098f, 1.0f-0.000071f,
      0.667979f, 1.0f-0.335851f,
      0.335973f, 1.0f-0.335903f,
      0.336024f, 1.0f-0.671877f,
      1.000004f, 1.0f-0.671847f,
      0.999958f, 1.0f-0.336064f,
      0.667979f, 1.0f-0.335851f,
      0.668104f, 1.0f-0.000013f,
      0.335973f, 1.0f-0.335903f,
      0.667979f, 1.0f-0.335851f,
      0.335973f, 1.0f-0.335903f,
      0.668104f, 1.0f-0.000013f,
      0.336098f, 1.0f-0.000071f,
      0.000103f, 1.0f-0.336048f,
      0.000004f, 1.0f-0.671870f,
      0.336024f, 1.0f-0.671877f,
      0.000103f, 1.0f-0.336048f,
      0.336024f, 1.0f-0.671877f,
      0.335973f, 1.0f-0.335903f,
      0.667969f, 1.0f-0.671889f,
      1.000004f, 1.0f-0.671847f,
      0.667979f, 1.0f-0.335851f
   };
   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData (GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
      g_vertex_buffer_data, GL_STATIC_DRAW);
   GLuint CubeVertexBuffer;
   glGenBuffers(1, &CubeVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
   glBufferData (GL_ARRAY_BUFFER, sizeof(g_cube_vertex_buffer_data),
      g_cube_vertex_buffer_data, GL_STATIC_DRAW);
   
   GLuint octahedronVertexBuffer;
   glGenBuffers(1, &octahedronVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, octahedronVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_octahedron_vertex_buffer_data),
      g_octahedron_vertex_buffer_data, GL_STATIC_DRAW);
   
   GLuint sphereVertexBuffer;
   glGenBuffers(1, &sphereVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gfSphereVertexBufferData),
      gfSphereVertexBufferData, GL_STATIC_DRAW);
   
   GLuint astVertexBuffer;
   glGenBuffers(1, &astVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, astVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gfAstVertexBufferData),
      gfAstVertexBufferData, GL_STATIC_DRAW);
   
   GLuint triangleVertexBuffer;
   glGenBuffers(1, &triangleVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gfTriangleBufferData),
      gfTriangleBufferData, GL_STATIC_DRAW);
   
   GLuint triColorVertexBuffer;
   glGenBuffers(1, &triColorVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gfTriColorBufferData),
      gfTriColorBufferData, GL_STATIC_DRAW);
   
   GLuint cubeColorVertexBuffer;
   glGenBuffers(1, &cubeColorVertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, cubeColorVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(gfCubeColorBufferData),
      gfCubeColorBufferData, GL_STATIC_DRAW);
   
   GLuint earthColorBuffer;
   glGenBuffers(1, &earthColorBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, earthColorBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(earthColBufData), earthColBufData,
      GL_STATIC_DRAW);
   
   GLuint astColorBuffer;
   glGenBuffers (1, &astColorBuffer);
   glBindBuffer (GL_ARRAY_BUFFER, astColorBuffer);
   glBufferData (GL_ARRAY_BUFFER, sizeof(astColBufData), astColBufData,
      GL_STATIC_DRAW);
   
   GLuint RedColorBuffer;
   glGenBuffers(1, &RedColorBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, RedColorBuffer);
   glBufferData (GL_ARRAY_BUFFER, sizeof(g_red_color_buffer_data),
      g_red_color_buffer_data, GL_STATIC_DRAW);
   
   GLuint uvbuffer;
   glGenBuffers(1, &uvbuffer);
   glBindBuffer(GL_ARRAY_BUFFER,uvbuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data,
      GL_STATIC_DRAW);
   GLsizei verticesToDraw=0;
   
   // Initialize our little text library with the Holstein font
   initText2D( "Holstein.DDS" );
   
   // For speed computation
   int nbFrames = 0;
   printf("G:%.10e\n"
          "D:%f\n"
          "R:%f\n",G,D,R);
   //Bounding walls the distance of 4 times the radius of earth
   World::Object* eastWall = pWorld->NewObject (
      vec3 (200, 2*initialCamZPos, 2*initialCamZPos),
      vec3 (initialCamZPos, 0, 0), vec3 (0.0f), 0, 0, 0, World::CUBOID);
   World::Object* westWall = pWorld->NewObject (
      vec3 (200, 2 * initialCamZPos, 2 * initialCamZPos),
      vec3 (-initialCamZPos, 0, 0), vec3 (0.0f), 0, 0, 0, World::CUBOID);
   World::Object* NorthWall = pWorld->NewObject (
      vec3 (2*initialCamZPos, 200, 2 * initialCamZPos),
      vec3 (0, initialCamZPos, 0), vec3 (0.0f), 0, 0, 0, World::CUBOID);
   World::Object* SouthWall = pWorld->NewObject (
      vec3 (2*initialCamZPos, 200, 2*initialCamZPos),
      vec3 (0, -initialCamZPos, 0), vec3 (0.0f), 0, 0, 0, World::CUBOID);
   World::Object* frontWall = pWorld->NewObject (
      vec3 (2*initialCamZPos, 2*initialCamZPos, 200),
      vec3 (0, 0, initialCamZPos), vec3(), 0, 0, 0, World::CUBOID);
   World::Object* backWall = pWorld->NewObject (
      vec3 (2*initialCamZPos, 2*initialCamZPos, 200),
      vec3 (0, 0, -initialCamZPos), vec3(), 0, 0, 0, World::CUBOID);
   
   World::Object* pEarth = pWorld->NewObject (vec3 (2*R, 2*R, 2*R), vec3 (0,0,0),
      vec3 (0.0f, 0.0f, 0.0f), sphereVertexBuffer, earthColorBuffer,
      uiNumSphericalVertices, World::Shape::SPHERE, 40000 * D, 0.8, vec3 (0.0),
      vec3(0.0f, 1.0f, 0.0f));
   
   World::Object* pMoon=
      pWorld->NewObject(vec3(1000,1000,1000), vec3(R+4000+1,0,0), vec3(0.0f),
      astVertexBuffer, astColorBuffer, uiNumAstVertices, World::SPHERE,
      1000*WD, 1, vec3(0,2000,0));
   
   World::Object* pEye = pWorld->NewObject (vec3 (1000, 1000, 1000), vec3(),
      vec3(), CubeVertexBuffer, cubeColorVertexBuffer, uiNumAstVertices,
      World::CUBOID);
   World::stick_objects (pEye, pEarth, vec3(0,0,R+100), vec3());
   /*
   World::Object* pEye = pWorld->NewObject (vec3 (1000, 1000, 1000),
      vec3 (-2000, 0, 0), vec3 (), CubeVertexBuffer, cubeColorVertexBuffer,
      uiNumAstVertices, World::CUBOID, 40000000*D, 0.8, vec3 (0.0, 0.0, 0.0),
      vec3 (0.0f, 0.0f, 0.0f));
   
   World::Object* pEye2 = pWorld->NewObject (vec3 (1000, 1000, 1000),
      vec3 (2000, 0, 0), vec3 (), CubeVertexBuffer, cubeColorVertexBuffer,
      uiNumAstVertices, World::CUBOID, 40000000*D, 0.8, vec3(-0.0, 0.0, 0.0),
      vec3(0.0f, 0.0f, 0.0f));
   */
   //glEnable(GL_CULL_FACE);
   bool shot = false;
   int hitCount = 0;
   vec3 sizeOfBullet = vec3 (100, 100, 100);
   float speedOfBullet = 50.0f;
   do {
      lastTime = currentTime;
      currentTime = glfwGetTime();
      deltaTime = currentTime - lastTime;
      computeMatricesFromInputs();

      // Measure speed
      if (glfwGetKey (window, GLFW_KEY_SPACE) == GLFW_PRESS &&
         speedOfBullet * (currentTime - lastShootTime) > length (sizeOfBullet)
      ) {
         World::Object* pProjectile =
         pWorld->NewObject (sizeOfBullet, camPosition, vec3 (0.0f),
            CubeVertexBuffer, cubeColorVertexBuffer, uiNumAstVertices,
            World::CUBOID, 10 * D, 1, direction * speedOfBullet);
         lastShootTime = glfwGetTime ();
      }
      
      ++nbFrames;
      char text[256];
      char countText[256];
      // If last prinf() was more than 1sec ago
      if (currentTime - lastPrintTime >= 1.0) {
         sprintf (text, "%d fps", nbFrames);
         nbFrames = 0;
         lastPrintTime = currentTime;
      }
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram (programID);
      
      
      pWorld->Draw (programID, MatrixID);
      if (pEye->GetCollider() != NULL) {
         ++hitCount;
      }
      sprintf (countText, "hitCount: %d", hitCount);
      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
      
      printText2D(text, 10, 40, 20);
      printText2D (countText, 10, 10, 20);
      
      glfwSwapBuffers (window);
      glfwPollEvents ();
   } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );
   
   glDeleteBuffers (1, &sphereVertexBuffer);
   glDeleteBuffers (1, &earthColorBuffer);
   glDeleteBuffers (1, &triangleVertexBuffer);
   glDeleteBuffers (1, &triColorVertexBuffer);
   glDeleteBuffers (1, &vertexbuffer);
   glDeleteBuffers (1, &CubeVertexBuffer);
   glDeleteBuffers (1, &octahedronVertexBuffer);
   glDeleteBuffers (1, &sphereVertexBuffer);
   glDeleteBuffers (1, &astVertexBuffer);
   glDeleteBuffers (1, &triangleVertexBuffer);
   glDeleteBuffers (1, &triColorVertexBuffer);
   glDeleteBuffers (1, &cubeColorVertexBuffer);
   glDeleteBuffers (1, &astColorBuffer);
   glDeleteBuffers (1, &RedColorBuffer);
   glDeleteBuffers (1, &uvbuffer);
   
   glDeleteProgram (programID);
   glDeleteVertexArrays (1, &VertexArrayID);
   
   // Delete the text's VBO, the shader and the texture
   cleanupText2D ();
   
   glfwTerminate ();
   
   return 0;
}

