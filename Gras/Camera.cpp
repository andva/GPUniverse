#include "Camera.h"
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif
const float ROTSPEED = 0.1f;
const float MOVEMENTSPEED = 0.02f * 0.2f;
Camera::Camera()
{
    xpos = 0;
    ypos = 0;
    zpos = 0;
    xrot = 0;
    yrot = 0;
	angle = 0;
	keyStates = new bool[256];
	for(int i = 0; i < 256; i++) keyStates[i] = false;

		
}
void Camera::move()
{	
	glRotatef(xrot,1.0,0.0,0.0);  //rotate our camera on the x-axis (left and right)
    glRotatef(yrot,0.0,1.0,0.0);  //rotate our camera on the y-axis (up and down)
    glTranslated(-xpos,-ypos,-zpos); //translate the screen to the position of our camera
}
void Camera::handleMouseMovement(int x, int y)
{
	int diffx = x-lastx;
	int diffy = y-lasty;
	lastx = x; lasty = y;
	xrot += (float) diffy*0.15; //set the xrot to xrot with the addition of the difference in the y position
    yrot += (float) diffx*0.15;    //set the xrot to yrot with the addition of the difference in the x position
	//fixa så inte det går snurra helt varv "uppåt"
	float maxrot = 360*0.48;
	if (xrot <= -maxrot)
    {
        xrot = -maxrot;
    }
    if (xrot >= maxrot)
    {
        xrot = maxrot;
    }
	Camera::move();
}
void Camera::handleMouseClick(int button, int state, int x, int y)
{
	if(!state == GLUT_DOWN)
    {
		lastx=x;
		lasty=y;
    }

}
void Camera::handleKeyRelease(unsigned char key)
{
	keyStates[key] = false;
}
void Camera::handleKeyClick(unsigned char key)
{
	keyStates[key] = true;
}
void Camera::keyOperations()
{
	if (keyStates['q'])
    {
		xrot += 1;
		if (xrot >360) xrot -= 360;
    }

    if (keyStates['z'])
    {
		xrot -= 1;
		if (xrot < -360) xrot += 360;
    }

    if (keyStates['w']) 
    {
		float xrotrad, yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xrotrad = (xrot / 180 * 3.141592654f); 
		xpos += float(sin(yrotrad))*MOVEMENTSPEED;
		zpos -= float(cos(yrotrad))*MOVEMENTSPEED;
		ypos -= float(sin(xrotrad))*MOVEMENTSPEED;
    }

    if (keyStates['s'])
    {
		float xrotrad, yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xrotrad = (xrot / 180 * 3.141592654f); 
		xpos -= float(sin(yrotrad))*MOVEMENTSPEED;
		zpos += float(cos(yrotrad))*MOVEMENTSPEED;
		ypos += float(sin(xrotrad))*MOVEMENTSPEED;
    }

    if (keyStates['d'])
    {
		float yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xpos += float(cos(yrotrad))* MOVEMENTSPEED;
		zpos += float(sin(yrotrad)) * MOVEMENTSPEED;
    }

    if (keyStates['a'])
    {
		float yrotrad;
		yrotrad = (yrot / 180 * 3.141592654f);
		xpos -= float(cos(yrotrad))  * MOVEMENTSPEED;
		zpos -= float(sin(yrotrad))  * MOVEMENTSPEED;
    }
}