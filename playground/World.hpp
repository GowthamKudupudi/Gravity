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

class World{
public:
    enum Shape {
        SPHERE,
        CUBOID,
        CIRCLE,
        RECTANGLE,
        TRIANGLE
    };
    struct Spin{
        Spin():axis(vec3(0.0f,0.1f,0.0f)),radiansPerSecond(0){};
        Spin(vec3 axis, float radiansPerSecond):axis(axis),radiansPerSecond(radiansPerSecond){};
        vec3 axis;
        float radiansPerSecond;
    };
    class Object{
        friend class World;
    public:
        
    private:
        Object(
            float   fWidth,
            float   dHeight,
            float   fDepth          = 0.0,
            float   fXpos           = 0.0,
            float   fYpos           = 0.0,
            float   fZpos           = 0.0,
            GLuint   vertexBuffer    = 0,
            GLuint   colorBuffer     = 0,
            GLsizei  totalVertices   = 0,
            Shape    shape           = TRIANGLE,
            float    fDensity        = 1000.0,
            float    fCollisionCoeff = 0.5,
            vec3     v3Velocity      = vec3(),
            Spin     spin            = Spin()
        );
        float          m_fWidth;
        float          m_fHeight;
        float          m_fDepth;
        float          m_fXpos;
        float          m_fYpos;
        float          m_fZpos;
        GLuint          vertexBuffer;
        GLuint          colorBuffer;
        GLsizei         totalVertices;
        Shape          m_Shape;
        float          m_fDensity;
        float           m_fCollisionCoeff;
        vec3            m_v3Velocity;
        Spin            m_spin;
        unsigned int    m_uiWorldIndex=0;
    };
    static World* CreateTheWorld(GLFWwindow* pWindow);
    static Object* NewObject(
                      float   fWidth,
                      float   dHeight,
                      float   fDepth          = 0.0,
                      float   fXpos           = 0.0,
                      float   fYpos           = 0.0,
                      float   fZpos           = 0.0,
                      GLuint   vertexBuffer    = 0,
                      GLuint   colorBuffer     = 0,
                      GLsizei  totalVertices   = 0,
                      Shape    shape           = TRIANGLE,
                      float    fDensity        = 1000.0,
                      float    fCollisionCoeff = 0.5,
                      vec3     v3Velocity      = vec3(),
                      Spin     spin            = Spin()
                      );
    static void Draw(GLuint programID, GLuint matrixID,mat4 projectionMatrix, mat4 viewMatrix);
    static GLboolean CheckCollision(World::Object &one, World::Object &two); // AABB - AABB collision
private:
    World(GLFWwindow* pWindow);
    ~World();
    static float            m_fWidth;
    static float            m_fHeight;
    static float            m_fDepth;
    static float            m_fLastDrawTime;
    static vector<Object*>   m_vpObjects;
    static World*            m_pOneWorld;
    static GLFWwindow* m_pWindow;
};
#endif /* World_hpp */
