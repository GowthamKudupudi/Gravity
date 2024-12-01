//
//  World.cpp
//  Tutorials
//
//  Created by SatyaGowthamKudupudi on 08/01/17.
//
//
// Include GLEW
#ifndef __glew_h__
#include <GL/glew.h>
#endif
#include "world.hpp"
#include "common/controls.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

using namespace std;
using namespace glm;

extern float G;
float World::m_fWidth = 20000.0f;
float World::m_fHeight = 20000.0f;
float World::m_fDepth = 20000.0f;
float World::m_fLastDrawTime = 0.0;
World*      World::m_pOneWorld  = NULL;
GLFWwindow* World::m_pWindow    = NULL;
vector<World::Object*> World::m_vpObjects;
//void glUniformMatrix4fv(GLint location,
//	GLsizei count,
//	GLboolean transpose,
//	const GLfloat *value);
//
//void glEnableVertexAttribArray(GLuint index);
//
//void glBindBuffer(GLenum target,
//	GLuint buffer);
//void glVertexAttribPointer(GLuint index,
//	GLint size,
//	GLenum type,
//	GLboolean normalized,
//	GLsizei stride,
//	const GLvoid * pointer);
//#define GL_ARRAY_BUFFER                                0x8892

World::World (GLFWwindow* pWindow) {}
World::~World () {}

World* World::create_world (GLFWwindow* pWindow) {
   if (!World::m_pOneWorld && pWindow) {
      World::m_pOneWorld =  new World (pWindow);
   }
   return World::m_pOneWorld;
}

mat4 rotate (mat4 r,vec3 v) {
   r=glm::rotate(r, radians(v.x), vec3(1.0f,0.0f,0.0f));
   r=glm::rotate(r, radians(v.y), vec3(0.0f,1.0f,0.0f));
   r=glm::rotate(r, radians(v.z), vec3(0.0f,0.0f,1.0f));
   return r;
}

//GLboolean World::CheckCollision(World::Object &one, World::Object &two) // AABB - AABB collision
//{
//    // Collision x-axis?
//    bool collisionX = one.m_fXpos + one.m_fWidth >= two.m_fXpos &&
//    two.m_fXpos + two.m_fWidth >= one.m_fXpos;
//    // Collision y-axis?
//    bool collisionY = one.m_fYpos + one.m_fHeight >= two.m_fYpos &&
//    two.m_fYpos + two.m_fHeight >= one.m_fYpos;
//    // Collision z-axis?
//    bool collisionZ = one.m_fZpos + one.m_fDepth >= two.m_fZpos &&
//    two.m_fZpos + two.m_fDepth >= one.m_fZpos;
//    // Collision only if on both axes
//    return collisionX && collisionY && collisionZ;
//}

void World::Draw (
   GLuint programID, GLuint mID, GLuint vpID
) {
   //printf ("TimePassed: %f\n", deltaTime);
   int i = 0; int j = 0;
   vector <Object> vTempObjs;
   // Copy of objects having stuck objects
   vector <Object> vMagnets;
   vector <Object*> vpStuckObj;
   int iStuckObjIndex = -1;
   
   // Compute new velocity of the object
   // due to gravity of all objects.
   for (i = 0; i < m_vpObjects .size(); ++i) {
      Object* pObj = m_vpObjects [i];
      /*for (j = 0; j < pObj->m_vpStuckObjs .size(); ++j) {
         Object& rJObj = *pObj->m_vpStuckObjs[j];
         rJObj.m_v3Position -= pObj->m_v3Position;
      }*/
      Object tempObj (*pObj);
      // bounding wall density is also zero
      if (!pObj->m_fDensity) goto afterGravityImpact;
      if (pObj->massive_pm != pObj) goto afterGravityImpact;
      for (j = 6; j < m_vpObjects .size (); ++j) {
         if (j==i) continue;
         Object* pJObj = m_vpObjects [j];
         if (!pJObj->m_fDensity) continue;
         vec3 displacement = pJObj->m_v3Position - pObj->m_v3Position;
         float d = length (displacement);
         float acceleration = G * pJObj->mass_fm / pow (d, 2);
         float speed = acceleration * deltaTime;
         vec3 velocity = displacement * speed / d;
         tempObj .m_v3Velocity += velocity;
      }
   afterGravityImpact:
      tempObj .m_v3Position += tempObj .massive_pm -> m_v3Velocity * deltaTime;
      vTempObjs .push_back (tempObj);
      pObj->m_pCollider = NULL;
      tempObj.m_pCollider = NULL;
   }
   
   // Detect collisions and compute final velocities from the temperory
   // velocities
   for (i = 0; i < vTempObjs .size (); ++i) {
      Object* pObj = m_vpObjects [i];
      Object& tmpIObj = vTempObjs [i];
      int collision = 0;
      // Do not collide static objects
      if (i < 6) goto afterCollision;
      
      for (j = 0; j < vTempObjs .size (); ++j) {
      startOfEachCollision:
         if (j == i) continue;
         Object& tmpJObj = vTempObjs [j];
         vec3 dPos;
         vec3 tSize;
         // Do not collide with the objects stuck to them.
         for (int k = 0; k < pObj->m_vpStuckObjs .size (); ++k) {
            if (
               pObj->m_vpStuckObjs [k]->m_uiWorldIndex ==
                  tmpJObj.m_uiWorldIndex
            ) {
               goto endOfCollision;
            }
         }
         
         dPos = tmpIObj.m_v3Position - tmpJObj.m_v3Position;
         if (dPos.x < 0) dPos.x = -dPos.x;
         if (dPos.y < 0) dPos.y = -dPos.y;
         if (dPos.z < 0) dPos.z = -dPos.z;
         tSize = tmpIObj.m_v3Size + tmpJObj.m_v3Size;
         tSize /= 2.0f;
         
         // AABB collision detection. If axial distance is less than or equal
         // to that of when adjacent
         if (dPos.x <= tSize.x && dPos.y <= tSize.y && dPos.z <= tSize.z) {
            if (j > 5 && i > j) goto afterCollision;
            dPos /= tSize;
            pObj->m_pCollider = m_vpObjects [j];
            World::Object* pJObj = m_vpObjects [j];
            //if (!pJObj->m_fDensity) pJObj->m_pCollider = pObj;
            // apply damping factor
            //tempObj .m_v3Velocity *= tempObj .m_fCollisionCoeff * pJObj
            //->m_fCollisionCoeff;
            
            // axis along which the collision occured will be the axis along
            // which the distance between the objects will be greatest. As it is
            // AABB collision detection, we are taking only referance axes into
            // the consideration.
            // v2 = (2m1u1 + u2 (m2 - m1))/(m1 + m2)
            // v1 = ((2m1u1 + u2 (m2 - m1))/(m1 + m2)) + u2 - u1
            float m1 = tmpIObj.massive_pm->mass_fm;
            float m2 = tmpJObj.massive_pm->mass_fm;
            float u1 = 0.0;
            float u2 = 0.0;
            vec3 v3Displacement;
            if (dPos .x > dPos .y && dPos .x > dPos .z) {
               u1 = pObj->m_v3Velocity.x;
               u2 = pJObj->m_v3Velocity.x;
               pObj->m_v3Velocity=tmpIObj.m_v3Velocity;
               pJObj->m_v3Velocity=tmpJObj.m_v3Velocity;
               if (j < 6) {
                  pObj->m_v3Velocity.x *= -1;
               } else {
                  pObj->m_v3Velocity.x = (2*m1*u1 + u2*(m2-m1))/(m1+m2)+u2-u1;
                  pJObj->m_v3Velocity.x =(2*m1*u1 + u2*(m2-m1))/(m1+m2);
               }
               printf ("x collision. u12v12:(%f,%f,%f,%f)\n",
                  u1, u2, pObj->m_v3Velocity.x, pJObj->m_v3Velocity.x);
               // account for reflection loss
               v3Displacement = pObj->m_v3Velocity * deltaTime;
               //v3Displacement.x=0;
               pObj->m_v3Position += v3Displacement;
               v3Displacement = pJObj->m_v3Velocity * deltaTime;
               //v3Displacement.x = 0;
               pJObj->m_v3Position += v3Displacement;
            } else if (dPos.y > dPos.x && dPos.y > dPos.z) {
               u1 = pObj->m_v3Velocity.y;
               u2 = pJObj->m_v3Velocity.y;
               pObj->m_v3Velocity=tmpIObj.m_v3Velocity;
               pJObj->m_v3Velocity=tmpJObj.m_v3Velocity;
               if (j < 6) {
                  pObj->m_v3Velocity.y *= -1;
               } else {
                  pObj->m_v3Velocity.y = (2*m1*u1 + u2*(m2-m1))/(m1+m2)+u2-u1;
                  pJObj->m_v3Velocity.y =(2*m1*u1 + u2*(m2-m1))/(m1+m2);
               }
               printf ("y collision. u12v12:(%f,%f,%f,%f)\n",
                  u1, u2, pObj->m_v3Velocity.y, pJObj->m_v3Velocity.y);
               v3Displacement = pObj->m_v3Velocity * deltaTime;
               //v3Displacement.y=0;
               pObj->m_v3Position += v3Displacement;
               v3Displacement = pJObj->m_v3Velocity * deltaTime;
               //v3Displacement.y = 0;
               pJObj->m_v3Position += v3Displacement;
            } else if (dPos.z > dPos.y && dPos.z > dPos.x) {
               u1 = pObj->m_v3Velocity.z;
               u2 = pJObj->m_v3Velocity.z;
               pObj->m_v3Velocity=tmpIObj.m_v3Velocity;
               pJObj->m_v3Velocity=tmpJObj.m_v3Velocity;
               if (j < 6) {
                  pObj->m_v3Velocity.z *= -1;
               } else {
                  pObj->m_v3Velocity.z = (2*m1*u1 + u2*(m2-m1))/(m1+m2)+u2-u1;
                  pJObj->m_v3Velocity.z =(2*m1*u1 + u2*(m2-m1))/(m1+m2);
               }
               printf ("z collision. u12v12:(%f,%f,%f,%f)\n",
                  u1, u2, pObj->m_v3Velocity.z, pJObj->m_v3Velocity.z);
               v3Displacement = pObj->m_v3Velocity * deltaTime;
               //v3Displacement.z=0;
               pObj->m_v3Position += v3Displacement;
               v3Displacement = pJObj->m_v3Velocity * deltaTime;
               //v3Displacement.z = 0;
               pJObj->m_v3Position += v3Displacement;
            }
            pObj->massive_pm->m_v3Velocity = pObj->m_v3Velocity;
            pJObj->massive_pm->m_v3Velocity = pJObj->m_v3Velocity;
            goto afterCollision;
         }//if (dPos.x <= tSize.x && dPos.y <= tSize.y && dPos.z <= tSize.z)
      endOfCollision:
         ;
      }//for (j = 0; j < vTempObjs .size (); ++j)
      pObj->m_v3Velocity = tmpIObj.m_v3Velocity;
      pObj->m_v3Position += tmpIObj.m_v3Velocity * deltaTime;
   
   afterCollision:
      if (pObj ->m_vpStuckObjs .size () && pObj->massive_pm==pObj)
         vMagnets .push_back (*pObj);
      
      
      // Calculate new direction of the object based on its angular velociy
      vec3 v3AngularDisplacement = pObj ->m_v3AVelocity * deltaTime;
      pObj->m_v3Direction+=v3AngularDisplacement;
      if (i>5) {
         //printf ("%d: m:%f whd:(%f,%f,%f) ", i, pObj->mass_fm, pObj->m_v3Size.x,
         //pObj->m_v3Size.y, pObj->m_v3Size.z);
      //printf ("at xyz:(%a,%a,%a) ", pObj->m_v3Position.x, pObj->m_v3Position.y,
         //pObj->m_v3Position.z);
   //printf ("with vxyz:(%a,%a,%a)\n", pObj->m_v3Velocity.x,
         //pObj->m_v3Velocity.y, pObj->m_v3Velocity.z);
      }
      // Draw later if the object is stuck to another object.
      ++iStuckObjIndex;
      if (
         iStuckObjIndex < vpStuckObj .size () &&
         pObj == vpStuckObj [iStuckObjIndex]
      ) continue;
      --iStuckObjIndex;
      mat4 scale (1.0f);
      mat4 rot (1.0f);
      mat4 trans (1.0f);
      rot = rotate (rot, pObj->m_v3Direction);
      scale = glm::scale (scale, pObj->m_v3Size/2.0f);
      trans = translate (mat4(1.0f), pObj->m_v3Position);
      mat4 model (1.0f);
      model = trans * scale * rot;
      mat4 identity(1.0f);
      if (pObj->m_VertexBuffer) {
         mat4 VP = ProjectionMatrix * ViewMatrix * identity;
         glUniformMatrix4fv (mID, 1, GL_FALSE, &model[0][0]);
         glUniformMatrix4fv (vpID, 1, GL_FALSE, &VP[0][0]);
         glEnableVertexAttribArray (0);
         glBindBuffer (GL_ARRAY_BUFFER, pObj->m_VertexBuffer);
         glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
         glEnableVertexAttribArray (1);
         glBindBuffer (GL_ARRAY_BUFFER, pObj->m_ColorBuffer);
         glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
         glDrawArrays (GL_TRIANGLES, 0, pObj->m_TotalVertices);
      }
      for (
         j = 0; j < pObj->m_vpStuckObjs .size () && pObj->massive_pm == pObj; ++j
      ) {
         vpStuckObj .push_back (pObj->m_vpStuckObjs [j]);
      }
   }

   //Draw stuck objects(our red triangle)
   int k = -1;
   for (i = 6; i < m_vpObjects .size(); ++i) {
      Object* pObj = m_vpObjects [i];
      if (!pObj->m_vpStuckObjs .size() || pObj->massive_pm!=pObj) continue;
      vec3 dPos = pObj->m_v3Position - vMagnets [++k] .m_v3Position;
      vec3 dDir = pObj->m_v3Direction - vMagnets [k] .m_v3Direction;
      // Update positions of stuck objects
      for (
         j = 0; j < pObj->m_vpStuckObjs .size (); ++j
      ) {
         Object& rJObj = *pObj->m_vpStuckObjs [j];
         vec3 rpos = rJObj .m_v3Position - vMagnets[k] .m_v3Position;
         rJObj .m_v3Direction += dDir;
         mat4 trans (1.0);
         trans = translate (trans, rpos);
         trans = rotate (trans, dDir);
         vec4 pos = trans * vec4 (rJObj .m_v3Position, 0);
         rJObj .m_v3Position .x = pos .x;
         rJObj .m_v3Position .y = pos .y;
         rJObj .m_v3Position .z = pos .z;
         rJObj .m_v3Position += dPos;
         mat4 rot (1.0f);
         rot = rotate (rot, rJObj .m_v3Direction);
         mat4 scale (1.0f);
         scale = glm::scale (scale, rJObj .m_v3Size / 2.0f);
         trans = translate (mat4 (1.0), rJObj .m_v3Position);
         mat4 model (1.0f);
         model = trans * scale * rot;
         mat4 identity(1.0f);
         if (rJObj .m_VertexBuffer) {
            mat4 VP = ProjectionMatrix * ViewMatrix * identity;
            glUniformMatrix4fv (mID, 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv (vpID, 1, GL_FALSE, &VP[0][0]);
            glEnableVertexAttribArray (0);
            glBindBuffer (GL_ARRAY_BUFFER, rJObj .m_VertexBuffer);
            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray (1);
            glBindBuffer (GL_ARRAY_BUFFER, rJObj .m_ColorBuffer);
            glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glDrawArrays (GL_TRIANGLES, 0, rJObj .m_TotalVertices);
         }
      }
   }
}

void World::stick_objects (
   Object* pObj1, Object* pToObj2, vec3 v3AtPos, vec3 v3InDirection
) {
   mat4 trans (1.0);
   trans = translate (trans, v3AtPos);
   trans = rotate (trans, pToObj2->m_v3Direction + v3InDirection);
   trans = translate (trans, pToObj2->m_v3Position);
   vec4 pos = trans * vec4 (pObj1->m_v3Position, 1);
   pObj1->m_v3Position .x = pos .x;
   pObj1->m_v3Position .y = pos .y;
   pObj1->m_v3Position .z = pos .z;
   pObj1->m_v3Direction += v3InDirection;
   pToObj2->m_fDensity = (pToObj2->GetMass () + pObj1->GetMass ()) /
      (pToObj2->GetVolume () + pObj1->GetVolume ());
   pToObj2->m_vpStuckObjs .push_back (pObj1);
   pObj1->m_vpStuckObjs .push_back (pToObj2);
   if (pObj1->mass_fm > pToObj2->mass_fm) {
      pObj1->massive_pm = pObj1;
      pToObj2->massive_pm = pObj1;
   } else {
      pObj1->massive_pm = pToObj2;
      pToObj2->massive_pm = pToObj2;
   }
   pObj1->mass_fm += pToObj2->mass_fm;
   pToObj2->mass_fm = pObj1->mass_fm;
   
}

World::Object* World::NewObject (
   vec3 v3Size, vec3 v3Position, vec3 v3Direction, GLuint vertexBuffer,
   GLuint colorBuffer, GLsizei totalVertices, Shape shape, float fDensity,
   float fCollisionCoeff, vec3 v3Velocity, vec3 v3AVelocity
) {
   Object* pObj = new Object (v3Size, v3Position, v3Direction, vertexBuffer,
      colorBuffer, totalVertices, shape, fDensity, fCollisionCoeff, v3Velocity,
      v3AVelocity);
   pObj->m_uiWorldIndex = m_vpObjects .size ();
   World::m_vpObjects .push_back (pObj);
   return pObj;
}

World::Object::Object (
   vec3     v3Size,
   vec3     v3Position,
   vec3     v3Direction,
   GLuint   vertexBuffer,
   GLuint   colorBuffer,
   GLsizei  totalVertices,
   Shape    shape,
   float    fDensity,
   float    fCollisionCoeff,
   vec3     v3Velocity,
   vec3     v3AVelocity
) :
   m_v3Size (v3Size),
   m_v3Position (v3Position),
   m_v3Direction (v3Direction),
   m_VertexBuffer (vertexBuffer),
   m_ColorBuffer (colorBuffer),
   m_TotalVertices (totalVertices),
   m_Shape (shape),
   m_fDensity (fDensity),
   m_fCollisionCoeff (fCollisionCoeff),
   m_v3Velocity (v3Velocity),
   m_v3AVelocity (v3AVelocity),
   massive_pm (this)
{
   switch (m_Shape) {
      case SPHERE:
         mass_fm = 4 / 3 * 3.14 * pow (m_v3Size .x / 2, 3) * m_fDensity;
      default:
         mass_fm = m_v3Size .x * m_v3Size .y * m_v3Size .z * m_fDensity;
   }
}

World::Object::Object (const World::Object& obj_r) :
   m_v3Size (obj_r.m_v3Size),
   m_v3Position (obj_r.m_v3Position),
   m_v3Direction (obj_r.m_v3Direction),
   m_VertexBuffer (obj_r.m_VertexBuffer),
   m_ColorBuffer (obj_r.m_ColorBuffer),
   m_TotalVertices (obj_r.m_TotalVertices),
   m_Shape (obj_r.m_Shape),
   m_fDensity (obj_r.m_fDensity),
   m_fCollisionCoeff (obj_r.m_fCollisionCoeff),
   m_v3Velocity (obj_r.m_v3Velocity),
   m_v3AVelocity (obj_r.m_v3AVelocity),
   m_uiWorldIndex(obj_r.m_uiWorldIndex),
   mass_fm(obj_r.mass_fm),
   massive_pm(this)
{
   if (&obj_r != obj_r.massive_pm) {
   
   }
}

float World::Object::GetMass () {
   switch (m_Shape) {
      case SPHERE:
         return 4 / 3 * 3.14 * pow (m_v3Size .x / 2, 3) * m_fDensity;
      default:
         return m_v3Size .x * m_v3Size .y * m_v3Size .z * m_fDensity;
   }
}

float World::Object::GetVolume (float fR) {
   switch (m_Shape) {
      case SPHERE:
         if (!fR) fR = m_v3Size .x / 2;
         return 4 / 3 * 3.14 * pow (fR/2, 3);
      default:
         return m_v3Size .x * m_v3Size .y * m_v3Size .z;
   }
}
