// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

void computeMatricesFromInputs(){

	
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, width/2, height/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(width/2 - xpos );
	verticalAngle   += mouseSpeed * float( height/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction=vec3(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS){
		camPosition += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		camPosition -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_F ) == GLFW_PRESS){
		camPosition += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		camPosition -= right * deltaTime * speed;
	}
    if(camPosition.x>initialCamZPos)camPosition.x=initialCamZPos;
    if(camPosition.y>initialCamZPos)camPosition.y=initialCamZPos;
    if(camPosition.z>initialCamZPos)camPosition.z=initialCamZPos;
    if(camPosition.x<-initialCamZPos)camPosition.x=-initialCamZPos;
    if(camPosition.y<-initialCamZPos)camPosition.y=-initialCamZPos;
    if(camPosition.z<-initialCamZPos)camPosition.z=-initialCamZPos;

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), width / height, minDisplayRange, maxDisplayRange);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								camPosition,           // Camera is here
								camPosition+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
