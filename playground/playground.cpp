// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;
float width=800.0f;
float height=600.0f;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
// Initial position : on +Z
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f; // Human eye 114
float initialCamZPos = 20000.0f;
float speed = 2500.0f; // 1500 km / second
float mouseSpeed = 0.005f;
float minDisplayRange=0.08f;     // 100m
float maxDisplayRange=sqrt(pow(initialCamZPos,2)*3); // 20,000km
glm::vec3 camPosition = glm::vec3( 0, 0, initialCamZPos );
float G = 6.674*pow(10,-20); // km3.kg-1.s-2
float D = 5510*pow(10,9); // kg.km-3
float WD = 1000*pow(10,9); // kg.km-3
float R = 6000;//6371; //km
glm::vec3 direction;

#include "common/shader.hpp"
//Tut5
#include "common/texture.hpp"
//Tut6
#include "common/controls.hpp"
#include "common/text2D.hpp"
#include "World.hpp"

#define octahedron_HT 0.816496580927726
#define EQ_TRIANGLE_HT 0.866025403784439
void NormalSmooth(const GLfloat* gfSolid, unsigned int uiSolidSize, GLfloat* gfSphere, unsigned int uiSphereSize, unsigned short iRegression=2, unsigned short usDimensions=3){
    if(uiSphereSize < (pow(4,iRegression)*uiSolidSize)) return;
    int i=0,j=0;
    unsigned int uiTempSolidFillSize=uiSolidSize;
    unsigned int uiTempSolidSize=uiSphereSize;
    GLfloat* gfTempSolid = (GLfloat*)malloc(uiTempSolidSize*sizeof(*gfSolid));
    memcpy(gfTempSolid, gfSolid, uiTempSolidFillSize*sizeof(*gfSolid));
    GLfloat gfTempTriangle[3*3];
    for (int r=0;r<iRegression;r++){
        i=0;
        j=0;
        while(i<uiTempSolidFillSize){
            // generate mid veritces
            int k=0;
            int n=0;
            GLfloat gfLen;
            for(k=0;k<3;k++){
                // for each dimension
                gfLen=0;
                for(n=0; n < usDimensions; n++){
                    gfTempTriangle[3*k+n] = gfTempSolid[(3*k)+i+n] + gfTempSolid[3*((k+1)%3)+i+n];
                    gfLen+=pow(gfTempTriangle[3*k+n],2);
                }
                gfLen=sqrt(gfLen);
                // normalize length
                for(n=0; n < usDimensions; n++)gfTempTriangle[3*k+n]/=gfLen;
            }
            // group vertices with mid vertices
            // each vertex
            for(k=0;k<3;k++){
                // at each dimension
                for(n=0;n<3;n++){
                    gfSphere[j+n]=gfTempSolid[i+3*((k+1)%3)+n];
                    gfSphere[j+3+n]=gfTempTriangle[3*k+n];
                    gfSphere[j+6+n]=gfTempTriangle[3*((k+1)%3)+n];
                }
                j+=9;
            }
            // vertices of triangle with mid points
            for(k=0;k<9;k++,j++)
                gfSphere[j]=gfTempTriangle[k];
            i+=9;
        }
        uiTempSolidFillSize=j;
        memcpy(gfTempSolid, gfSphere, uiTempSolidFillSize*sizeof(*gfTempSolid));
    }
}

int main( void )
{
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow( (int)width, (int)height, "Playground", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    World* pWorld= World::CreateTheWorld(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    //tut6
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, width/2, height/2);
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    GLuint programID = LoadShaders( "vertexplayer.vertexshader", "fragmentplayer.fragmentshader" );
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -1.0f, -1.0f,
        0.5f, -1.0f, -1.0f,
        0.0f,  1.0f, -1.0f,
    };
    
    static const GLfloat g_cube_vertex_buffer_data[] = {
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
    static const GLfloat g_octahedron_vertex_buffer_data[] = {
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
    static const unsigned int uiNumSphericalVertices=3*8*4*4*4*4*4;
    static const unsigned int uiNumAstVertices=3*8*4*4*4;
    GLfloat gfSphereVertexBufferData[3*uiNumSphericalVertices];
    NormalSmooth(g_octahedron_vertex_buffer_data, 3*3*8, gfSphereVertexBufferData, 3*uiNumSphericalVertices, 5);
    GLfloat gfAstVertexBufferData[3*uiNumAstVertices];
    NormalSmooth(g_octahedron_vertex_buffer_data, 3*3*8, gfAstVertexBufferData, 3*uiNumAstVertices, 3);
    // One color for each vertex. They were generated randomly.
    float r,g,b;
    GLfloat earthColBufData[3*uiNumSphericalVertices];
    for (int v = 0; v < uiNumSphericalVertices; v++){
        //193
        if((v>767 && v<1536) || (v>3071 && v<3840) || (v>13055 && v<13824) || (v>15359 && v<16128)){
            if((v>1151&&v<1344) || (v>3455 && v<3648) || (v>13439 && v<13632) || (v>15743 && v<15936) ){
                earthColBufData[3*v+2] = 1.0f;
                continue;
            }
            earthColBufData[3*v+0] = 1.0f;
            earthColBufData[3*v+1] = 1.0f;
            continue;
        }
        else if(!v%10000){
            r=(float)(rand()%5)/5;
            g=(float)(rand()%5)/5;
            b=(float)(rand()%5)/5;
        }
        r+=0.001f;
        r=r>=1.0f?(r-1.0f):r;
        earthColBufData[3*v+0] = r;
        g+=0.001f;
        g=g>=1.0f?(g-1.0f):g;
        earthColBufData[3*v+1] = g;
        b+=0.001f;
        b=b>=1.0f?(b-1.0f):b;
        earthColBufData[3*v+2] = b;
    }
    GLfloat astColBufData[3*uiNumAstVertices];
//    r=(float)(rand()%5)/5;
//    g=(float)(rand()%5)/5;
//    b=(float)(rand()%5)/5;
    for (int v = 0; v < uiNumAstVertices; v++){
        r+=0.1f;
        r=r>=1.0f?(r-1.0f):r;
        astColBufData[3*v+0] = r;
        g+=0.1f;
        g=g>=1.0f?(g-1.0f):g;
        astColBufData[3*v+1] = r;
        b+=0.1f;
        b=b>=1.0f?(b-1.0f):b;
        astColBufData[3*v+2] = r;
    }
    static const GLfloat gfTriangleBufferData[]={
        0.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };
    static const GLfloat gfTriColorBufferData[]={
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f
    };
    static const GLfloat gfCubeColorBufferData[]={
        1.0f,1.0f,.0f,
        1.0f,1.0f,.0f,
        1.0f,1.0f,.0f,
        1.0f,1.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
        1.0f,.0f,.0f,
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    GLuint CubeVertexBuffer;
    glGenBuffers(1, &CubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_vertex_buffer_data), g_cube_vertex_buffer_data, GL_STATIC_DRAW);
    
    GLuint octahedronVertexBuffer;
    glGenBuffers(1, &octahedronVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, octahedronVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_octahedron_vertex_buffer_data), g_octahedron_vertex_buffer_data, GL_STATIC_DRAW);
    
    GLuint sphereVertexBuffer;
    glGenBuffers(1, &sphereVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gfSphereVertexBufferData), gfSphereVertexBufferData, GL_STATIC_DRAW);
    
    GLuint astVertexBuffer;
    glGenBuffers(1, &astVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, astVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gfAstVertexBufferData), gfAstVertexBufferData, GL_STATIC_DRAW);
    
    GLuint triangleVertexBuffer;
    glGenBuffers(1, &triangleVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gfTriangleBufferData), gfTriangleBufferData, GL_STATIC_DRAW);
    
    GLuint triColorVertexBuffer;
    glGenBuffers(1, &triColorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gfTriColorBufferData), gfTriColorBufferData, GL_STATIC_DRAW);
    
    GLuint cubeColorVertexBuffer;
    glGenBuffers(1, &cubeColorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeColorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gfCubeColorBufferData), gfCubeColorBufferData, GL_STATIC_DRAW);
    
    GLuint earthColorBuffer;
    glGenBuffers(1, &earthColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, earthColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(earthColBufData), earthColBufData, GL_STATIC_DRAW);

    GLuint astColorBuffer;
    glGenBuffers(1, &astColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, astColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(astColBufData), astColBufData, GL_STATIC_DRAW);

    GLuint RedColorBuffer;
    glGenBuffers(1, &RedColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, RedColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_red_color_buffer_data), g_red_color_buffer_data, GL_STATIC_DRAW);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER,uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    GLsizei verticesToDraw=0;
    
    // Initialize our little text library with the Holstein font
    initText2D( "Holstein.DDS" );
    
    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    printf("G:%.10e\n"
           "D:%f\n"
           "R:%f\n",G,D,R);
    World::Object* pEarth=
        pWorld->NewObject(vec3(2*R,2*R,2*R),vec3(0,0,0),vec3(0.0f,0.0f,0.0f),false,sphereVertexBuffer,earthColorBuffer,uiNumSphericalVertices,World::Shape::SPHERE, 4000*D,0.8,vec3(0.0),vec3(0.0f,10.0f,0.0f));
    World::Object* pProjectile=
        pWorld->NewObject(vec3(1000,1000,1000),vec3(R+4000+1,0,0),vec3(0.0f),false,astVertexBuffer, astColorBuffer,uiNumAstVertices,World::SPHERE,1000*WD,1,vec3(0,800,0));
    World::Object* pEye=
    pWorld->NewObject(vec3(1000,1000,1), vec3(), vec3(),true,triangleVertexBuffer,triColorVertexBuffer,3,World::Shape::TRIANGLE);
    World::Stick(pEye, pEarth, vec3(0,0,R+100), vec3());
    
    //Bounding walls the distance of 4 times the radius of earth
    World::Object* eastWall=
        pWorld->NewObject(vec3(200, 2*initialCamZPos, 2*initialCamZPos), vec3(R+4600, 0,0),vec3(0.0f),false,0,0,0,World::Shape::CUBOID);
    World::Object* westWall=
        pWorld->NewObject(vec3(200, 2*initialCamZPos, 2*initialCamZPos), vec3(-R-4600, 0,0),vec3(0.0f),false,0,0,0,World::Shape::CUBOID);
    World::Object* NorthWall=
        pWorld->NewObject(vec3(2*initialCamZPos,200, 2*initialCamZPos), vec3(0,R+4600,0),vec3(0.0f),false,0,0,0,World::Shape::CUBOID);
    World::Object* SouthWall=
        pWorld->NewObject(vec3(2*initialCamZPos,200, 2*initialCamZPos), vec3(0,-R-4600,0),vec3(0.0f),false,0,0,0,World::Shape::CUBOID);
    World::Object* frontWall=
        pWorld->NewObject(vec3(2*initialCamZPos, 2*initialCamZPos,200), vec3(0,0,R+4600),vec3(),false,0,0,0,World::Shape::CUBOID);
    World::Object* backWall=
        pWorld->NewObject(vec3(2*initialCamZPos, 2*initialCamZPos,200),vec3( 0,0,-R-4600),vec3(),false,0,0,0,World::Shape::CUBOID);
    bool shot=false;
    int hitCount=0;
    do{
        // Measure speed
        double currentTime = glfwGetTime();
        if (glfwGetKey( window, GLFW_KEY_J ) == GLFW_PRESS&&!shot){
            World::Object* pProjectile=
            pWorld->NewObject(vec3(1000,1000,1000),camPosition,vec3(0.0f),false,CubeVertexBuffer, cubeColorVertexBuffer,uiNumAstVertices,World::CUBOID,10*WD,1,direction*1000.0f);
            shot=true;
            lastTime+=1.0;
        }
        
        nbFrames++;
        char text[256];
        char countText[256];
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
            sprintf(text,"%d fps", nbFrames );
            nbFrames = 0;
            lastTime += 1.0;
            shot=false;
        }
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        
        computeMatricesFromInputs();
        mat4 ProjectionMatrix = getProjectionMatrix();
        mat4 ViewMatrix=getViewMatrix();
        
        pWorld->Draw(programID, MatrixID, ProjectionMatrix, ViewMatrix);
        if(pEye->GetCollider()!=NULL && pEye->GetCollider()!=pEarth){
            hitCount++;
            
        }
        sprintf(countText,"hitCount: %d", hitCount );
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        //printText2D(text, 10, 10, 30);
        printText2D(countText, 10, 10, 30);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );
    
    glDeleteBuffers(1, &sphereVertexBuffer);
    glDeleteBuffers(1, &earthColorBuffer);
    glDeleteBuffers(1, &triangleVertexBuffer);
    glDeleteBuffers(1, &triColorVertexBuffer);
    
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
 
    // Delete the text's VBO, the shader and the texture
    cleanupText2D();
    
    glfwTerminate();
    
    return 0;
}

