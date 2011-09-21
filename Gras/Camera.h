#include <math.h> 
#include "vmath.h"
class Camera
{
public:
	float xpos;
	float ypos;
	float zpos;
	float xrot;
	float yrot;
	float lastx, lasty;
	float angle;
	

	void handleMouseMovement(int x, int y);
	void handleMouseClick(int button, int state, int x, int y);
	void handleKeyClick(unsigned char key);
	void handleKeyRelease(unsigned char key);
	void keyOperations();
	void move();
	bool* keyStates; 
	Camera();

};