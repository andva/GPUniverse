#include "gpu.h";

void gpu::initCl()
{
	/*Initiation of variables*/
	err = 0;
	returned_size = 0;
	device = NULL;
	platform = NULL;

	int n = 500;
	// Allocate some memory and a place for the results
	float * a = (float *)malloc(n*sizeof(float));
	float * b = (float *)malloc(n*sizeof(float));
	float * results = (float *)malloc(n*sizeof(float));
	
	// Fill in the values
	for(int i=0;i<n;i++)
	{
		a[i] = (float)i;
		b[i] = (float)n-i;
		results[i] = 0.f;
	}
	gpu::setupKernels();
	//runCL(a, b, results, n);
}
void gpu::setupKernels()
{
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

	// Now create a context to perform our calculation with the 
	// specified device 
	context = clCreateContext(0, 1, &device, NULL, NULL, &err);
	assert(err == CL_SUCCESS);

	// Load the program source from disk
	// The kernel/program is the project directory and in Xcode the executable
	// is set to launch from that directory hence we use a relative path
	const char * filename = "kernels.cl";
	char *program_source = fileloader::load_source(filename);
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
											NULL, &err);
	assert(err == CL_SUCCESS);
	
	err = clBuildProgram(program[0], 0, NULL, "", NULL, NULL);
	assert(err == CL_SUCCESS);

	// Now create the kernel "objects" that we want to use in the example file 
	kernel[0] = clCreateKernel(program[0], "add", &err);
	assert(err == CL_SUCCESS);

	// Allocate memory on the device to hold our data and store the results into
	//buffer_size = sizeof(float) * n;
}

void gpu::deallocateCL()
{

}