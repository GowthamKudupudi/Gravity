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
#include "World.hpp"
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

World::World(GLFWwindow* pWindow){}
World::~World(){}

World* World::CreateTheWorld(GLFWwindow* pWindow){
    if(!World::m_pOneWorld && pWindow){
        World::m_pOneWorld =  new World(pWindow);
    }
    return World::m_pOneWorld;
}

mat4 rotate(mat4 r,vec3 v){
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

void World::Draw(GLuint programID, GLuint matrixID, mat4 projectionMatrix, mat4 viewMatrix){
    float dTimePassed = m_fLastDrawTime;
    static const float initialTime = glfwGetTime();
    m_fLastDrawTime = glfwGetTime();
    //float dTotalTimePassed = m_fLastDrawTime-initialTime;
    dTimePassed = m_fLastDrawTime - dTimePassed;
    int i=0;int j=0;
    vector<Object> vTempObjs;
    vector<mat4> vTempTrans;
    // Copy of objects having stuck objects
    vector<Object> vMagnets;
    vector<Object*> vpStuckObj;
    int iStuckObjIndex=-1;
    for(i=0;i<m_vpObjects.size();i++){
        
        // Compute new position, velocity and spin of the object
        // based on present velocity and collision of all objects.
        
        Object* pObj=m_vpObjects[i];
        for(j=0;j<pObj->m_vpStuckObjs.size();j++){
            Object& rJObj=*pObj->m_vpStuckObjs[j];
            rJObj.m_v3Position-=pObj->m_v3Position;
        }
        //if(!pObj->m_VertexBuffer||!pObj->m_ColorBuffer)continue;
        mat4 trans(1.0f);
        trans = glm::translate(trans, pObj->m_v3Position);
        Object tempObj(*pObj);
        if(!pObj->m_fDensity)goto afterGravityImpact;
        for(j=0;j<m_vpObjects.size();j++){
            if(j==i)continue;
            Object* pJObj=m_vpObjects[j];
            if(!pJObj->m_fDensity)continue;
            float mass;
            vec3 displacement=pJObj->m_v3Position-pObj->m_v3Position;
            float d =length(displacement);
            switch(pJObj->m_Shape){
                case SPHERE:
                    mass=4/3*3.14*pow(pJObj->m_v3Size.x/2<d?pJObj->m_v3Size.x/2:d,3)*pJObj->m_fDensity;
                    break;
                default:
                    mass=pJObj->m_v3Size.x*pJObj->m_v3Size.y*pJObj->m_v3Size.z;
                    break;
            }
            float acceleration=G*mass/pow(d<1?1:d, 2);
            float speed=acceleration*dTimePassed;
            vec3 velocity=displacement*speed/(d<1?1:d);
            tempObj.m_v3Velocity+=velocity;
        }
afterGravityImpact:
        mat4 beforeCollisionTrans = glm::translate(trans, mat3(dTimePassed)*tempObj.m_v3Velocity);
        //printf("before translate %d:%f,%f,%f\n",i,tempObj.m_fXpos,tempObj.m_fYpos,tempObj.m_fZpos);
        tempObj.m_v3Position=vec3(beforeCollisionTrans[3].x,beforeCollisionTrans[3].y,beforeCollisionTrans[3].z);
        
        //printf("translate %d:%f,%f,%f\n",i,tempObj.m_fXpos,tempObj.m_fYpos,tempObj.m_fZpos);
        vTempObjs.push_back(tempObj);
        vTempTrans.push_back(trans);
        pObj->m_pCollider=NULL;
    }
    
    // Detect collisions and compute final velocities from the temperory velocites
    for(i=0;i<vTempObjs.size();i++){
        Object* pObj=m_vpObjects[i];
        Object& tempObj=vTempObjs[i];
        bool collision=false;
        mat4 trans=vTempTrans[i];
        for(j=0;j<vTempObjs.size();j++){
            if(j==i)continue;
            Object* pJObj = &vTempObjs[j];
            vec3 dPos = tempObj.m_v3Position-pJObj->m_v3Position;
            if(dPos.x<0)dPos.x=-dPos.x;
            if(dPos.y<0)dPos.y=-dPos.y;
            if(dPos.z<0)dPos.z=-dPos.z;
            vec3 tSize=tempObj.m_v3Size+pJObj->m_v3Size;
            tSize/=2.0f;
            // AABB collision detection. If axial distance is less than or equal that of when adjacent
            if(dPos.x<=tSize.x&&dPos.y<=tSize.y&&dPos.z<tSize.z){
                collision=true;
                dPos/=tSize;
                pObj->m_pCollider=pJObj;
                if(!tempObj.m_fDensity)break;
                //apply damping factor
                tempObj.m_v3Velocity*=tempObj.m_fCollisionCoeff*pJObj->m_fCollisionCoeff;
                if(dPos.x>dPos.y&&dPos.x>dPos.z){
                    tempObj.m_v3Velocity.x=-tempObj.m_v3Velocity.x;
                    printf("x collision. v=%f\n",tempObj.m_v3Velocity.x);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;tempObj.m_v3Velocity.x/=2;
                }
                else if(dPos.y>dPos.x&&dPos.y>dPos.z){
                    tempObj.m_v3Velocity.y=-tempObj.m_v3Velocity.y;
                    printf("y collision. v=%f\n",tempObj.m_v3Velocity.y);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;
                    tempObj.m_v3Velocity.y/=2;
                }
                else if(dPos.z>dPos.y&&dPos.z>dPos.x){
                    tempObj.m_v3Velocity.z=-tempObj.m_v3Velocity.z;
                    printf("z collision. v=%f\n",tempObj.m_v3Velocity.z);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;tempObj.m_v3Velocity.z/=2;
                }
                break;
            }
        }
        if(pObj->m_vpStuckObjs.size())vMagnets.push_back(*pObj);
        if(!collision)pObj->m_v3Velocity=tempObj.m_v3Velocity;
        vec3 v3Displacement=tempObj.m_v3Velocity*dTimePassed;
        trans=translate(trans, v3Displacement);
        pObj->m_v3Position=vec3(trans[3].x,trans[3].y,trans[3].z);
        printf("Drawing %d with w:%f,h:%f,d:%f\n",i,pObj->m_v3Size.x,pObj->m_v3Size.y,pObj->m_v3Size.z);
        printf("at %f,%f,%f\n",pObj->m_v3Position.x,pObj->m_v3Position.y,pObj->m_v3Position.z);
        printf("vx:%f,vy:%f,vz:%f\n",pObj->m_v3Velocity.x,pObj->m_v3Velocity.y,pObj->m_v3Velocity.z);
        mat4 scale(1.0f);
        mat4 rot(1.0f);
        vec3 v3AngularDisplacement=pObj->m_v3AVelocity*dTimePassed;
        pObj->m_v3Direction+=v3AngularDisplacement;
        // Draw later if the object is stuck to another object.
        ++iStuckObjIndex;
        if(vpStuckObj.size()&& pObj==vpStuckObj[iStuckObjIndex])continue;
        --iStuckObjIndex;
        rot=rotate(rot,pObj->m_v3Direction);
        scale = glm::scale(scale,pObj->m_v3Size/2.0f);
        mat4 model(1.0f);
        model = trans*scale*rot;
        if(pObj->m_VertexBuffer){
            mat4 MVP = projectionMatrix*viewMatrix*model;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, pObj->m_VertexBuffer);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, pObj->m_ColorBuffer);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
            glDrawArrays(GL_TRIANGLES, 0, pObj->m_TotalVertices);
        }
        for(j=0;j<pObj->m_vpStuckObjs.size();j++){
            vpStuckObj.push_back(pObj->m_vpStuckObjs[j]);
        }
        
    }
    
    //Draw stuck objects(our red triangle)
    int k=-1;
    for(i=0;i<m_vpObjects.size();i++){
        Object* pObj=m_vpObjects[i];
        if(!pObj->m_vpStuckObjs.size())continue;
        vec3 dPos=pObj->m_v3Position-vMagnets[++k].m_v3Position;
        vec3 dDir=pObj->m_v3Direction-vMagnets[k].m_v3Direction;
        // Update positions of stuck objects
        for(j=0;j<pObj->m_vpStuckObjs.size();j++){
            
            Object& rJObj=*pObj->m_vpStuckObjs[j];
            vec3 rpos=rJObj.m_v3Position-vMagnets[k].m_v3Position;
            rJObj.m_v3Direction+=dDir;
            mat4 trans(1.0);
            trans=translate(trans, rpos);
            trans=rotate(trans,dDir);
            vec4 pos=trans*vec4(rJObj.m_v3Position,0);
            rJObj.m_v3Position.x=pos.x;
            rJObj.m_v3Position.y=pos.y;
            rJObj.m_v3Position.z=pos.z;
            rJObj.m_v3Position+=dPos;
            mat4 rot(1.0f);
            rot=rotate(rot,rJObj.m_v3Direction);
            mat4 scale(1.0f);
            scale = glm::scale(scale,rJObj.m_v3Size/2.0f);
            trans=translate(mat4(1.0), rJObj.m_v3Position);
            mat4 model(1.0f);
            model = trans*scale*rot;
            if(rJObj.m_VertexBuffer){
                mat4 MVP = projectionMatrix*viewMatrix*model;
                glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, rJObj.m_VertexBuffer);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, rJObj.m_ColorBuffer);
                glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
                glDrawArrays(GL_TRIANGLES, 0, rJObj.m_TotalVertices);
            }
        }
    }
}
void World::Stick(Object* pObj1, Object* pToObj2,vec3 v3AtPos,vec3 v3InDirection){
    mat4 trans(1.0);
    trans=translate(trans, v3AtPos);
    trans=rotate(trans, pToObj2->m_v3Direction+v3InDirection);
    trans=translate(trans, pToObj2->m_v3Position);
    vec4 pos=trans*vec4(pObj1->m_v3Position,1);
    pObj1->m_v3Position.x=pos.x;
    pObj1->m_v3Position.y=pos.y;
    pObj1->m_v3Position.z=pos.z;
    pObj1->m_v3Direction+=v3InDirection;
    pToObj2->m_vpStuckObjs.push_back(pObj1);
}

World::Object* World::NewObject(
                                vec3     v3Size,
                                vec3     v3Position,
                                vec3     v3Direction,
                                bool     bMighty,
                                GLuint   vertexBuffer,
                                GLuint   colorBuffer,
                                GLsizei  totalVertices,
                                Shape    shape,
                                float    fDensity,
                                float    fCollisionCoeff,
                                vec3     v3Velocity,
                                vec3     v3AVelocity
                                ){
    Object* pObj = new Object(v3Size,v3Position, v3Direction, bMighty,vertexBuffer,colorBuffer,totalVertices,shape,fDensity,fCollisionCoeff,v3Velocity,v3AVelocity);
    pObj->m_uiWorldIndex=m_vpObjects.size();
    World::m_vpObjects.push_back(pObj);
    return pObj;
}

World::Object::Object(
                      vec3     v3Size,
                      vec3     v3Position,
                      vec3     v3Direction,
                      bool     bMighty,
                      GLuint   vertexBuffer,
                      GLuint   colorBuffer,
                      GLsizei  totalVertices,
                      Shape    shape,
                      float    fDensity,
                      float    fCollisionCoeff,
                      vec3     v3Velocity,
                      vec3     v3AVelocity
                      ):
m_v3Size(v3Size),
m_v3Position(v3Position),
m_v3Direction(v3Direction),
m_bMighty(bMighty),
m_VertexBuffer(vertexBuffer),
m_ColorBuffer(colorBuffer),
m_TotalVertices(totalVertices),
m_Shape(shape),
m_fDensity(fDensity),
m_fCollisionCoeff(fCollisionCoeff),
m_v3Velocity(v3Velocity),
m_v3AVelocity(v3AVelocity)
{
    
}

