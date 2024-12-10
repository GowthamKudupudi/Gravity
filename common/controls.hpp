#ifndef CONTROLS_HPP
#define CONTROLS_HPP

extern float width;
extern float height;
extern glm::vec3 camPosition;
extern glm::vec3 sunPosition;
extern float horizontalAngle;
extern float verticalAngle;
extern float initialFoV;
extern float speed;
extern float mouseSpeed;
extern float minDisplayRange;
extern float maxDisplayRange;
extern float initialCamZPos;
extern float sunZPos;
extern double lastTime;
extern double currentTime;
extern float deltaTime;
extern glm::vec3 direction;

extern glm::mat4 ViewMatrix;
extern glm::mat4 ProjectionMatrix;
extern glm::mat4 ShdwViewMatrix;
extern glm::mat4 ShdwPrjMatrix;
extern GLuint depthMVPID2;

extern GLuint magnitudeID;
extern GLuint shdwProgID;
extern GLuint depthMVPID;
extern GLuint shadowMapID;
extern GLuint FramebufferName;
extern float magnitude;
extern GLuint depthTexture;

void computeMatricesFromInputs();

#endif
