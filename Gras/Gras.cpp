#ifdef _WIN32
#include "windows.h"
#endif
#include "Gpu.h"
#include "Camera.h"
#include "Fileloader.h"
#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\freeglut.h>
#include "vmath.h"

#include <stdio.h>
#include <math.h> 
#include <CL\opencl.h>
#include <assert.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static void redraw(void);
static void update(void);
static void init(int, char);
static void idle(void);
void closeFunc();

void setupCamera();
void setupScene();
void loadShader();
int runCL(float*, float*, float*, int);
void drawColorFloor();
void mouseMovement(int, int);
void keyRelease(unsigned char, int, int);
void mouseClick(int, int, int, int);
void keyClick(unsigned char, int, int);
static void resize(int, int);
void showFPS();

using namespace std;

char title[200];
double t0;
int frame;
bool running;
double fps;
int width, height;
Camera cam;
Gpu kernels;


int main(int argc, char **argv)
{
	
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (800, 600);
	glutCreateWindow("");
	setupScene();
	glutDisplayFunc(update);	
	glutIdleFunc(idle);
	glutReshapeFunc(resize);
	glutCloseFunc(closeFunc);
		//Hanterar mus-rörelse
	glutPassiveMotionFunc(mouseMovement);
		//Hantera mus-klick
	//glutMouseFunc(mouseClick);
	
	glutKeyboardFunc(keyClick);
	glutKeyboardUpFunc(keyRelease);
	glutMainLoop();
	return 0; 
}
void closeFunc()
{
	//Exit function
}
void setupCamera()
{
    // Get window size. It may start out different from the requested
    // size, and will change if the user resizes the window.
	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);
    if(height<=0) height=1; // Safeguard against iconified/closed window

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport( 0, 0, width, height ); // The entire window

    // Select and setup the projection matrix.
    glMatrixMode(GL_PROJECTION); // "We want to edit the projection matrix"
    glLoadIdentity(); // Reset the matrix to identity
    // 65 degrees FOV, same aspect ratio as window, depth range 1 to 100
    gluPerspective( 65.0f, (GLfloat)width/(GLfloat)height, 0.3f, 100.0f );

    // Select and setup the modelview matrix.
    glMatrixMode( GL_MODELVIEW ); // "We want to edit the modelview matrix"
    glLoadIdentity(); // Reset the matrix to identity
    // Look from 0,-5,0 towards 0,0,0 with Z as "up" in the image
    gluLookAt( 0.0f, 0.0f, 0.0f,    // Eye position
               0.0f, 1.0f, 0.0f,   // View point
               0.0f, 0.0f, 1.0f );  // Up vector
}
void temp()
{
	int n = 500;
	// Allocate some memory and a place for the results
	
	float  *a = (float *)malloc(n*sizeof(float));
	float* b = (float *)malloc(n*sizeof(float));
	float* results = (float *)malloc(n*sizeof(float));
	
	// Fill in the values
	for(int i=0;i<n;i++)
	{
		a[i] = (float)i;
		b[i] = (float)n-i;
		results[i] = 0.f;
	}
	kernels.runCl(a, b, results, n);
}
void setupScene()
{
	t0 = 0.0;
	frame = 0;
	running = true;
	fps = 0;
	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);

	//Dölj pekaren när musen är i fönstret!
	glutSetCursor(GLUT_CURSOR_NONE);

	//Skapa OpenCL-variabeln som kommer att utföra
	//alla beräkningar som skall ske på gpun!
	kernels.initCl();
	//Läs in shaders som skall användas!
	temp();
	
	loadShader();
	
	//Initiera kameran!
	setupCamera();

	glEnable(GL_CULL_FACE); // Cull away all back facing polygons
    glEnable(GL_DEPTH_TEST); // Use the Z buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}
void loadShader()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  // Problem: glewInit failed, something is seriously wrong.
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	//Kan inte använda shaders
	if (!GLEW_ARB_vertex_shader && !GLEW_ARB_fragment_shader)
	{
		cout << "Not totally ready";
		exit(1);
	}
	//Source till shaders
	char* fragment_shader_source;
	char* vertex_shader_source;

	//shaders läggs i dessa
	GLenum sp, vs, fs;

	vs = glCreateShader(GL_VERTEX_SHADER);
	char* vsSource = Fileloader::load_source("vertex.vert");
	glShaderSource(vs, 1, (const GLchar**)&vsSource, NULL);
	glCompileShader(vs);
	//Test ifall vertex-shadern kompilerar
	GLint compiled;
	glGetObjectParameterivARB(vs, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
	   cout << "Vertex shader didn't compile!!";
	}   
	//Samma sak görs för fragment-shadern som för
	//Vertexshadern!
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	char* fsSource = Fileloader::load_source("vertex.vert");
	glShaderSource(fs, 1, (const GLchar**)&fsSource, NULL);
	glCompileShader(fs);
	glGetObjectParameterivARB(fs, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
	   cout << "Fragment shader didn't compile!!";
	}   
	//Frigör minne från source-koden
	free(vsSource);
	free(fsSource);
	//Bestämmer att man använder sp!
	sp = glCreateProgram();
	glLinkProgram(sp);
	glUseProgram(sp);

}
//int runCL(float * a, float * b, float * results, int n)
//{
	/*
	*Program skapas av att kompilera en kernel-fil, det görs i programmet och
	*genererar ett gäng kernels (GPU-funktioner) som gör beräkningar med hjälp
	*av grafikkortet. Alla kernels sparas i kernel-arrayen kernel.
	*//*
	cl_program program[1];
	cl_kernel kernel[2];
	
	cl_command_queue cmd_queue;
	cl_context   context;
	
	cl_device_id device = NULL;
	cl_platform_id platform = NULL;

	cl_int err = 0;
	size_t returned_size = 0;
	size_t buffer_size;
	
	cl_mem a_mem, b_mem, ans_mem;
	//Hitta alla tillgängliga plattformar
	err = clGetPlatformIDs(2, &platform, NULL);
	assert(err == CL_SUCCESS);

	// Hitta möjlig GPU att göra beräkningar på, finns ingen väljs cpu
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS) err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
	assert(device);
	
	// Hämta information om GPU:n
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), 
							vendor_name, &returned_size);
	err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), 
							device_name, &returned_size);
	assert(err == CL_SUCCESS);
	printf("Connecting to %s %s...\n", vendor_name, device_name);
	

		
	// And also a command queue for the context
	cmd_queue = clCreateCommandQueue(context, device, 0, NULL);

	// Load the program source from disk
	// The kernel/program is the project directory and in Xcode the executable
	// is set to launch from that directory hence we use a relative path
	const char * filename = "kernels.cl";
	char *program_source = Fileloader::load_source(filename);
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
											NULL, &err);
	assert(err == CL_SUCCESS);
		
	err = clBuildProgram(program[0], 0, NULL, "", NULL, NULL);
	assert(err == CL_SUCCESS);
		
	// Now create the kernel "objects" that we want to use in the example file 
	kernel[0] = clCreateKernel(program[0], "add", &err);
	assert(err == CL_SUCCESS);


	// Allocate memory on the device to hold our data and store the results into
	buffer_size = sizeof(float) * n;
		
	// Input array a
	a_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
	err = clEnqueueWriteBuffer(cmd_queue, a_mem, CL_TRUE, 0, buffer_size,
								(void*)a, 0, NULL, NULL);
		
	// Input array b
	b_mem = clCreateBuffer(context, CL_MEM_READ_ONLY, buffer_size, NULL, NULL);
	err |= clEnqueueWriteBuffer(cmd_queue, b_mem, CL_TRUE, 0, buffer_size,
								(void*)b, 0, NULL, NULL);
	assert(err == CL_SUCCESS);
		
	// Results array
	ans_mem	= clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_size, NULL, NULL);
		
	// Get all of the stuff written and allocated 
	clFinish(cmd_queue);

	// Now setup the arguments to our kernel
	err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &a_mem);
	err |= clSetKernelArg(kernel[0],  1, sizeof(cl_mem), &b_mem);
	err |= clSetKernelArg(kernel[0],  2, sizeof(cl_mem), &ans_mem);
	assert(err == CL_SUCCESS);

	// Run the calculation by enqueuing it and forcing the 
	// command queue to complete the task
	size_t global_work_size = n;
	err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 1, NULL, 
									&global_work_size, NULL, 0, NULL, NULL);
	assert(err == CL_SUCCESS);
	clFinish(cmd_queue);
		
	// Once finished read back the results from the answer 
	// array into the results array
	err = clEnqueueReadBuffer(cmd_queue, ans_mem, CL_TRUE, 0, buffer_size, 
								results, 0, NULL, NULL);
	assert(err == CL_SUCCESS);
	clFinish(cmd_queue);

	clReleaseMemObject(a_mem);
	clReleaseMemObject(b_mem);
	clReleaseMemObject(ans_mem);
		
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
*/
//	return CL_SUCCESS;
//}
void drawColorFloor()
{
	glPushMatrix();
	float w = 1.0f;
	float h = 0.3f;
	glBegin(GL_QUADS);

	glColor3f(1.0,1,0);
	glNormal3f(1.0,0,0);

	glVertex3f(w,0,0);
	glVertex3f(w,w,0);
	glVertex3f(w,w,h);
	glVertex3f(w,0,h);

	glColor3f(0,0,1);
	glNormal3f(-1,0,0);

	glVertex3f(0,0,0);
	glVertex3f(0,0,h);
	glVertex3f(0,w,h);
	glVertex3f(0,w,0);

	glColor3f(1,0,1);
	glNormal3f(0,1,0);

	glVertex3f(0,w,0);
	glVertex3f(0,w,h);
	glVertex3f(w,w,h);
	glVertex3f(w,w,0);

	glColor3f(0,1,0);
	glNormal3f(0,-1,0);

	glVertex3f(0,0,0);
	glVertex3f(w,0,0);
	glVertex3f(w,0,h);
	glVertex3f(0,0,h);

	glColor3f(0,1,1);
	glNormal3f(0,0,1);

	glVertex3f(0,0,h);
	glVertex3f(w,0,h);
	glVertex3f(w,w,h);
	glVertex3f(0,w,h);

	glColor3f(1,0,0);
	glNormal3f(0,0,-h);
	glVertex3f(0,0,0);
	glVertex3f(w,0,0);
	glVertex3f(w,w,0);
	glVertex3f(0,w,0);

	glEnd();
	glPopMatrix();
}
void drawColorSphere(float r, int segs) 
{
	/*int i, j;
	float x, y, z, z1, z2, R, R1, R2;

	// Top cap: a triangle fan
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,1);
	glColor3f(0.5,0.5,1);
	glVertex3f(0,0,r);
	z = cos(M_PI /segs);

	R = sin(M_PI /segs);

	for(i = 0; i <= 2*segs; i++) 
	{
		x = R*cos(i*2.0*M_PI/(2*segs));
		y = R*sin(i*2.0*M_PI/(2*segs));
		glNormal3f(x, y, z);
		glColor3f(0.5*(x+1), 0.5*(y+1), 0.5*(z+1));
		glVertex3f(r*x, r*y, r*z);
	}
	glEnd();  

	// Height segments: each a triangle strip
	for(j = 1; j < segs-1; j++) 
	{
		z1 = cos(j*M_PI/segs);
		R1 = sin(j*M_PI/segs);
		z2 = cos((j+1)*M_PI/segs);
		R2 = sin((j+1)*M_PI/segs);
		glBegin(GL_TRIANGLE_STRIP);
		for(i = 0; i <= 2*segs; i++) 
		{
			x = R1*cos(i*2.0*M_PI/(2*segs));
			y = R1*sin(i*2.0*M_PI/(2*segs));
			glNormal3f(x, y, z1);
			glColor3f(0.5*(x+1), 0.5*(y+1), 0.5*(z1+1));
			glVertex3f(r*x, r*y, r*z1);
			x = R2*cos(i*2.0*M_PI/(2*segs));
			y = R2*sin(i*2.0*M_PI/(2*segs));
			glNormal3f(x, y, z2);
			glColor3f(0.5*(x+1), 0.5*(y+1), 0.5*(z2+1));
			glVertex3f(r*x, r*y, r*z2);
		}
		glEnd();
	}

	// Bottom cap: a triangle fan
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,-1);
	glColor3f(0.5,0.5,0);
	glVertex3f(0,0,-r);
	z = -cos(M_PI/(float)segs);
	R = sin(M_PI/(float)segs);
	for(i = 2*segs; i >= 0; i--) 
	{
		x = R*cos(i*2.0f*M_PI/(2.0*segs));
		y = R*sin(i*2.0f*M_PI/(2.0*segs));
		glNormal3f(x, y, z);
		glColor3f(0.5f*(x+1.0f), 0.5f*(y+1.0f), 0.5f*(z+1.0f));
		glVertex3f(r*x, r*y, r*z);
	}
	glEnd();*/
}
void mouseMovement(int x, int y)
{
	cam.handleMouseMovement(x,y);
	//Om man har flyttat musen återställs positionen till det ursprungliga läget!
	if(cam.lastx != width /2 || cam.lasty != height / 2)
	{
		glutWarpPointer(width /2, height / 2);
		cam.lastx = (float)width/2;
		cam.lasty = (float)height/2;
	}
	
}
void keyRelease(unsigned char key, int x, int y)
{
	cam.handleKeyRelease(key);
}
void mouseClick(int button, int state, int x, int y)
{
	cam.handleMouseClick(button, state, x, y);
}
void keyClick(unsigned char key, int a, int b)
{
	cam.handleKeyClick(key);
	if(key == 27)
	{
		exit(0);
	}
}
static void resize(int w, int h)
{
	width = w;
	height = h;
	const float ar = (float) width / (float) height;
	if(height<=0) height=1;
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    gluPerspective( 65.0f, (GLfloat)width/(GLfloat)height, 1.0f, 100.0f );
	glMatrixMode( GL_MODELVIEW ); 

}
static void update(void)
{
	cam.keyOperations();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_DEPTH_TEST); //enable the depth testing
    glEnable (GL_LIGHTING); //enable the lighting
    glEnable (GL_LIGHT0); //enable LIGHT0, our Diffuse Light
    glShadeModel (GL_SMOOTH); //set the shader to smooth shader

	glLoadIdentity(); 
    //glTranslatef(0.0f, 0.0f, -30);
    glRotatef(cam.xrot,1.0,0.0,0.0);
    //glutSolidCube(2); //Our character to follow
	
	glRotatef(cam.yrot,0.0,1.0,0.0);  //rotate our camera on the y-axis (up and down)
    glTranslated(-cam.xpos,0.0f,-cam.zpos); //translate the screen to the position of our camera
    drawColorFloor();

	glutSwapBuffers();
}
static void idle(void)
{
	showFPS();
	glutPostRedisplay();
}
void showFPS()
{
	double t;
	t=glutGet(GLUT_ELAPSED_TIME);
	fps = frame*1000.0/(t-t0);
	if (t - t0 > 1000) 
	{
		sprintf_s(title,"Simulation Beta (%.1f FPS)", fps);
		glutSetWindowTitle(title);
		t0 = t;
		frame = 0;
	}
	frame++;
}


