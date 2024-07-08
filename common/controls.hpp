#ifndef CONTROLS_HPP
#define CONTROLS_HPP

extern float width;
extern float height;
extern glm::vec3 camPosition;
extern float horizontalAngle;
extern float verticalAngle;
extern float initialFoV;
extern float speed;
extern float mouseSpeed;
extern float minDisplayRange;
extern float maxDisplayRange;
extern float initialCamZPos;
extern double lastTime;
extern double currentTime;
extern float deltaTime;
extern glm::vec3 direction;
extern float theta;

extern glm::mat4 ViewMatrix;
extern glm::mat4 ProjectionMatrix;

void computeMatricesFromInputs();

#endif
