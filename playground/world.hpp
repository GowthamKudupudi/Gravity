//
//  World.hpp
//  Tutorials
//
//  Created by SatyaGowthamKudupudi on 08/01/17.
//
//

#ifndef World_hpp
#define World_hpp

#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>
#include <glfw3.h>

using namespace std;
using namespace glm;

class World {
public:
   enum Shape {
      SPHERE,
      CUBOID,
      CIRCLE,
      RECTANGLE,
      TRIANGLE
   };
   struct Spin {
      Spin () : axis (vec3 (0.0f, 0.1f, 0.0f)), radiansPerSecond (0) {};
      Spin (vec3 axis, float radiansPerSecond) :
         axis (axis), radiansPerSecond (radiansPerSecond) {};
      vec3 axis;
      float radiansPerSecond;
   };
   class Object {
      friend class World;
   public:
      Object (const Object& Obj_r);
      Object* GetCollider () {return m_pCollider;}
      float GetMass ();
      float GetVolume (float fR = 0);
   private:
      Object (
         vec3     v3Size,
         vec3     v3Position,
         vec3     v3Direction,
         GLuint   vertexBuffer    = 0,
         GLuint   colorBuffer     = 0,
         GLsizei  totalVertices   = 0,
         Shape    shape           = TRIANGLE,
         float    fDensity        = 0.0f,
         float    fCollisionCoeff = 1.0f,
         vec3     v3Velocity      = vec3(),
         vec3     v3AVelocity     = vec3()
      );
      
      vec3              m_v3Size;
      vec3              m_v3Position;
      vec3              m_v3Direction;
      GLuint            m_VertexBuffer;
      GLuint            m_ColorBuffer;
      GLsizei           m_TotalVertices;
      Shape             m_Shape;
      float             m_fDensity;
      float             m_fCollisionCoeff;
      vec3              m_v3Velocity;
      vec3              m_v3AVelocity;
      unsigned int      m_uiWorldIndex = 0;
      Object*           m_pCollider = NULL;
      vector<Object*>   m_vpStuckObjs = vector<Object*>();
      float             mass_fm;
      Object*           massive_pm;
   };
   static World* create_world (GLFWwindow* pWindow);
   
   /*!
   @brief creates an object in the world
   @param v3Size size
   @param v3Position position
   @param v3Direction direction
   @param vertexBuffer vertex buffer
   @param colorBuffer color buffer
   @param totalVertices total vertices
   @param shape shape
   @param fDensity density
   @param fCollisionCoeff collision coefficient
   @param v3Velocity velocity
   @param v3AVelocity angular velocity
   */
   static Object* NewObject (
      vec3 v3Size, vec3 v3Position, vec3 v3Direction, GLuint vertexBuffer = 0,
      GLuint colorBuffer = 0, GLsizei totalVertices = 0, Shape shape = TRIANGLE,
      float fDensity = 0.0, float fCollisionCoeff = 1.0,
      vec3 v3Velocity = vec3(), vec3 v3AVelocity = vec3()
   );
   
   static void Draw (GLuint programID, GLuint mID, GLuint vpID);
   static void stick_objects (Object* pObj1, Object* pToObj2,
      vec3 v3AtPos,vec3 v3InDirection);

private:
   World (GLFWwindow* pWindow);
   ~World ();
   static float            m_fWidth;
   static float            m_fHeight;
   static float            m_fDepth;
   static float            m_fLastDrawTime;
   static vector <Object*> m_vpObjects;
   static World*           m_pOneWorld;
   static GLFWwindow*      m_pWindow;
};
#endif /* World_hpp */
