//tut1 - create a window
//tut2 - draw a triangle in the window
//tut3 - view the triangle from a different angle
//tut3ext -  place a cube alongside triangle
//tut4 - draw a cube with different color faces
//tut5 - replace face colors with textures
//tut6 - mouse and keyboard interaction with opengl objects

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
//Tut5
#include <common/texture.hpp>
//Tut6
#include <common/controls.hpp>
int main( void )
{
    float width=800.0f;
    float height=600.0f;
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
    
    window = glfwCreateWindow( (int)width, (int)height, "Playground - Colored Cube", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
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
    
    //Tutorial 4
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    GLuint programID = LoadShaders( "vertexplayer.vertexshader", "fragmentplayer.fragmentshader" );
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    //Tut3 to view the triangle from different view
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f,100.0f);
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
    
    glm::mat4 View =  glm::lookAt(
                                  glm::vec3(4,3,3),
                                  glm::vec3(0,0,0),
                                  glm::vec3(0,1,0)
                                  );
    glm:: mat4 Model = glm::mat4(1.0f);
    glm::mat4 MVP = Projection*View*Model;
    mat4 TriangleMVP =Model;
    //Tut5
    GLuint Texture = loadDDS("uvtemplate.DDS");
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
    
    //tut2
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -1.0f, -1.0f,
        0.5f, -1.0f, -1.0f,
        0.0f,  1.0f, -1.0f,
    };
    
    //tut3 ext
    static const GLfloat g_cube_vertex_buffer_data[] = {
        // //tut4
        //static const GLfloat g_vertex_buffer_data[] = {
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
    static const GLfloat g_tetrahedron_vertex_buffer_data[] = {
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
    // One color for each vertex. They were generated randomly.
    static GLfloat g_color_buffer_data[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };
    //tut4ext
    static GLfloat g_red_color_buffer_data[]={
        1.0f,   0.0f,   0.0f,
        1.0f,   0.0f,   0.0f,
        1.0f,   0.0f,   0.0f
        //        0.673f,  0.211f,  0.457f,
        //        0.820f,  0.883f,  0.371f,
        //        0.982f,  0.099f,  0.879f
    };
    //tut5
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
    //Tut2
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    //Tut3ext
    GLuint CubeVertexBuffer;
    glGenBuffers(1, &CubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_vertex_buffer_data), g_cube_vertex_buffer_data, GL_STATIC_DRAW);
    //Tut4
    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
    //Tu4
    GLuint RedColorBuffer;
    glGenBuffers(1, &RedColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, RedColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_red_color_buffer_data), g_red_color_buffer_data, GL_STATIC_DRAW);
    //Tut5
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER,uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    
    do{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        
        //tut4 exercise
                for (int v = 0; v < 12*3 ; v++){
                    g_color_buffer_data[3*v+0] += 0.3f;
                    g_color_buffer_data[3*v+0]=g_color_buffer_data[3*v+0]>=1.0f?g_color_buffer_data[3*v+0]-1:g_color_buffer_data[3*v+0];
                    g_color_buffer_data[3*v+1] += 0.3f;
                    g_color_buffer_data[3*v+1]=g_color_buffer_data[3*v+1]>=1.0f?g_color_buffer_data[3*v+1]-1:g_color_buffer_data[3*v+1];
                    g_color_buffer_data[3*v+2] += 0.3f;
                    g_color_buffer_data[3*v+2]=g_color_buffer_data[3*v+2]>=1.0f?g_color_buffer_data[3*v+2]-1:g_color_buffer_data[3*v+2];
                }
         //tut6
                computeMatricesFromInputs();
                mat4 ProjectionMatrix = getProjectionMatrix();
                mat4 ViewMatrix=getViewMatrix();
                mat4 ModelMatrix=mat4(1.0);
                mat4 MVP = ProjectionMatrix*ViewMatrix*ModelMatrix;
        
//        //tut3
//        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &TriangleMVP[0][0]);
        
        //tut5
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
//        //tut5
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, Texture);
//        glUniform1i(TextureID, 0);
        
        //tut2
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
        
//        //tut4 not tut5
//        glEnableVertexAttribArray(1);
//        glBindBuffer(GL_ARRAY_BUFFER, RedColorBuffer);
//        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
        
                //tut5
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
                glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);
        
        //tut2
        glDrawArrays(GL_TRIANGLES, 0, 12*3);
        
        //tut2
        glDisableVertexAttribArray(0);
        //tut4
        glDisableVertexAttribArray(1);
        
        //tut4ext
        //        glUniformMatrix4fv(MatrixID,1,GL_FALSE,&MVP[0][0]);
        //        glEnableVertexAttribArray(0);
        //        glBindBuffer(GL_ARRAY_BUFFER, CubeVertexBuffer);
        //        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
        //        glEnableVertexAttribArray(1);
        //        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        //        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
        //        glDrawArrays(GL_TRIANGLES, 0, 12*3);
        //        glDisableVertexAttribArray(0);
        //        glDisableVertexAttribArray(1);
        
        //tut1
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );
    
    //tut2
    glDeleteBuffers(1, &vertexbuffer);
    //tut4 not tut5
    //    glDeleteBuffers(1, &colorbuffer);
    //tut5
    glDeleteBuffers(1, &uvbuffer);
    
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    
    glfwTerminate();
    
    return 0;
}

