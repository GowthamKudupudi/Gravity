//
//  World.cpp
//  Tutorials
//
//  Created by SatyaGowthamKudupudi on 08/01/17.
//
//
// Include GLEW
#ifndef __glew_h__
#include <GL\glew.h>
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

World::Object* World::NewObject(
                      float    fWidth,
                      float    fHeight,
                      float    fDepth,
                      float    fXpos,
                      float    fYpos,
                      float    fZpos,
                      GLuint    vertexBuffer,
                      GLuint    colorBuffer,
                      GLsizei   totalVertices,
                      Shape     shape,
                      float     fDensity,
                      float     fCollisionCoeff,
                      vec3      v3Velocity,
                      World::Spin      spin
){
    Object* pObj = new Object(fWidth,fHeight,fDepth,fXpos,fYpos,fZpos,vertexBuffer,colorBuffer,totalVertices,shape,fDensity,fCollisionCoeff,v3Velocity,spin);
    pObj->m_uiWorldIndex=m_vpObjects.size();
    World::m_vpObjects.push_back(pObj);
    return pObj;
}

GLboolean World::CheckCollision(World::Object &one, World::Object &two) // AABB - AABB collision
{
    // Collision x-axis?
    bool collisionX = one.m_fXpos + one.m_fWidth >= two.m_fXpos &&
    two.m_fXpos + two.m_fWidth >= one.m_fXpos;
    // Collision y-axis?
    bool collisionY = one.m_fYpos + one.m_fHeight >= two.m_fYpos &&
    two.m_fYpos + two.m_fHeight >= one.m_fYpos;
    // Collision z-axis?
    bool collisionZ = one.m_fZpos + one.m_fDepth >= two.m_fZpos &&
    two.m_fZpos + two.m_fDepth >= one.m_fZpos;
    // Collision only if on both axes
    return collisionX && collisionY && collisionZ;
}

void World::Draw(GLuint programID, GLuint matrixID, mat4 projectionMatrix, mat4 viewMatrix){
    float dTimePassed = m_fLastDrawTime;
    static const float initialTime = glfwGetTime();
    m_fLastDrawTime = glfwGetTime();
    float dTotalTimePassed = m_fLastDrawTime-initialTime;
    dTimePassed = m_fLastDrawTime - dTimePassed;
    int i=0;int j=0;
    vector<Object> vTempObjs;
    vector<mat4> vTempTrans;
    for(i=0;i<m_vpObjects.size();i++){
        
        // Compute new position, velocity and spin of the object
        // based on present velocity and collision of all objects.
        
        Object* pObj=m_vpObjects[i];
        if(!pObj->vertexBuffer||!pObj->colorBuffer)continue;
        mat4 trans(1.0f);
        mat4 scale(1.0f);
        mat4 rotate(1.0f);
        scale = glm::scale(scale,vec3(pObj->m_fWidth/2,pObj->m_fHeight/2,pObj->m_fDepth/2));
        rotate = glm::rotate(rotate, (float)dTotalTimePassed * pObj->m_spin.radiansPerSecond,pObj->m_spin.axis);
        trans = glm::translate(trans, vec3(pObj->m_fXpos,pObj->m_fYpos,pObj->m_fZpos));
        trans = trans*scale*rotate;
        Object tempObj(*pObj);
        for(j=0;j<m_vpObjects.size();j++){
            if(j==i)continue;
            Object* pJObj=m_vpObjects[j];
            float mass;
            vec3 displacement(pObj->m_fXpos-pJObj->m_fXpos,pObj->m_fYpos-pJObj->m_fYpos,pObj->m_fZpos-pJObj->m_fZpos);
            displacement=-displacement;
            float d =length(displacement);
            switch(pJObj->m_Shape){
                case SPHERE:
                    mass=4/3*3.14*pow(pJObj->m_fWidth/2<d?pJObj->m_fWidth/2:d,3)*pJObj->m_fDensity;
                    break;
                default:
                    mass=pJObj->m_fWidth*pJObj->m_fHeight*pJObj->m_fDepth;
                    break;
            }
            float acceleration=G*mass/pow(d<1?1:d, 2);
            float speed=acceleration*dTimePassed;
            vec3 velocity=displacement*speed/(d<1?1:d);
            tempObj.m_v3Velocity+=velocity;
        }
        mat4 beforeCollisionTrans = glm::translate(trans, mat3(dTimePassed)*tempObj.m_v3Velocity);
        //printf("before translate %d:%f,%f,%f\n",i,tempObj.m_fXpos,tempObj.m_fYpos,tempObj.m_fZpos);
        tempObj.m_fXpos=beforeCollisionTrans[3].x; tempObj.m_fYpos=beforeCollisionTrans[3].y; tempObj.m_fZpos=beforeCollisionTrans[3].z;
        //printf("translate %d:%f,%f,%f\n",i,tempObj.m_fXpos,tempObj.m_fYpos,tempObj.m_fZpos);
        vTempObjs.push_back(tempObj);
        vTempTrans.push_back(trans);
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
            float dx=tempObj.m_fXpos-pJObj->m_fXpos;
            float dy=tempObj.m_fYpos-pJObj->m_fYpos;
            float dz=tempObj.m_fZpos-pJObj->m_fZpos;
            if(dx<0)dx=-dx;
            if(dy<0)dy=-dy;
            if(dz<0)dz=-dz;
            float dw=tempObj.m_fWidth+pJObj->m_fWidth;
            float dh=tempObj.m_fHeight+pJObj->m_fHeight;
            float dd=tempObj.m_fDepth+pJObj->m_fDepth;
            dw/=2;dh/=2;dd/=2;
            // AABB collision detection. If axial distance is less than or equal that of when adjacent
            if(dx<=dw&&dy<=dh&&dz<=dd){
                collision=true;
                dx/=dw;
                dy/=dh;
                dz/=dd;
                if(dx>dy&&dx>dz){
                    tempObj.m_v3Velocity.x=-tempObj.m_v3Velocity.x;
                    printf("x collision. v=%f\n",tempObj.m_v3Velocity.x);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;tempObj.m_v3Velocity.x/=2;
                }
                else if(dy>dx&&dy>dz){
                    tempObj.m_v3Velocity.y=-tempObj.m_v3Velocity.y;
                    printf("y collision. v=%f\n",tempObj.m_v3Velocity.y);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;
                    tempObj.m_v3Velocity.y/=2;
                }
                else if(dz>dy&&dz>dx){
                    tempObj.m_v3Velocity.z=-tempObj.m_v3Velocity.z;
                    printf("z collision. v=%f\n",tempObj.m_v3Velocity.z);
                    pObj->m_v3Velocity=tempObj.m_v3Velocity;tempObj.m_v3Velocity.z/=2;
                }
                break;
            }
        }
        if(!collision)pObj->m_v3Velocity=tempObj.m_v3Velocity;
        trans=translate(trans, mat3(dTimePassed)*tempObj.m_v3Velocity);
        pObj->m_fXpos=trans[3].x; pObj->m_fYpos=trans[3].y; pObj->m_fZpos=trans[3].z;
        printf("Drawing %d with w:%f,h:%f,d:%f\n",i,pObj->m_fWidth,pObj->m_fHeight,pObj->m_fDepth);
        printf("at %f,%f,%f\n",pObj->m_fXpos,pObj->m_fYpos,pObj->m_fZpos);
        printf("vx:%f,vy:%f,vz:%f\n",pObj->m_v3Velocity.x,pObj->m_v3Velocity.y,pObj->m_v3Velocity.z);
        if(pObj->vertexBuffer){
            mat4 MVP = projectionMatrix*viewMatrix*trans;
            glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, pObj->vertexBuffer);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, pObj->colorBuffer);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
            glDrawArrays(GL_TRIANGLES, 0, pObj->totalVertices);
        }
    }
}

World::Object::Object(
                      float    fWidth,
                      float    fHeight,
                      float    fDepth,
                      float    fXpos,
                      float    fYpos,
                      float    fZpos,
                      GLuint    vertexBuffer,
                      GLuint    colorBuffer,
                      GLsizei   totalVertices,
                      Shape     shape,
                      float     fDensity,
                      float     fCollisionCoeff,
                      vec3      v3Velocity,
                      World::Spin  spin
                      ):
m_fWidth(fWidth),
m_fHeight(fHeight),
m_fDepth(fDepth),
m_fXpos(fXpos),
m_fYpos(fYpos),
m_fZpos(fZpos),
vertexBuffer(vertexBuffer),
colorBuffer(colorBuffer),
totalVertices(totalVertices),
m_Shape(shape),
m_fDensity(fDensity),
m_fCollisionCoeff(fCollisionCoeff),
m_v3Velocity(v3Velocity),
m_spin(spin)
{
    
}

