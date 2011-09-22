#include "Gpu.h";

void Gpu::initCl()
{
	/*Initiation of variables*/
	err = 0;
	returned_size = 0;
	device = NULL;
	platform = NULL;
	/*
	n = 500;
	// Allocate some memory and a place for the results
	a = (float *)malloc(n*sizeof(float));
	b = (float *)malloc(n*sizeof(float));
	results = (float *)malloc(n*sizeof(float));
	
	// Fill in the values
	for(int i=0;i<n;i++)
	{
		a[i] = (float)i;
		b[i] = (float)n-i;
		results[i] = 0.f;
	}*/
	Gpu::setupKernels();

}
void Gpu::setupKernels()
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
	char *program_source = Fileloader::load_source(filename);
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
											NULL, &err);
	assert(err == CL_SUCCESS);
	
	err = clBuildProgram(program[0], 0, NULL, "", NULL, NULL);
	assert(err == CL_SUCCESS);

	// Now create the kernel "objects" that we want to use in the example file 
	kernel[0] = clCreateKernel(program[0], "add", &err);
	assert(err == CL_SUCCESS);
}
void Gpu::runCl(float* a, float* b, float* results, int n)
{
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
	string e = Gpu::print_cl_errstring(err);
	cout << e;
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
}
void Gpu::deallocateCL()
{
	/*clReleaseMemObject(a_mem);
	clReleaseMemObject(b_mem);
	clReleaseMemObject(ans_mem);

	clReleaseComandQueue(cmd_queue);
	clReleaseContext(context);*/
}
string print_cl_errstring(cl_int err) 
{
    switch (err) {
        case CL_SUCCESS:                          return "Success!";
        case CL_DEVICE_NOT_FOUND:                 return "Device not found.";
        case CL_DEVICE_NOT_AVAILABLE:             return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:           return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                 return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:               return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                 return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:            return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:            return "Program build failure";
        case CL_MAP_FAILURE:                      return "Map failure";
        case CL_INVALID_VALUE:                    return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:              return "Invalid device type";
        case CL_INVALID_PLATFORM:                 return "Invalid platform";
        case CL_INVALID_DEVICE:                   return "Invalid device";
        case CL_INVALID_CONTEXT:                  return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:         return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:            return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                 return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:               return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:               return "Invalid image size";
        case CL_INVALID_SAMPLER:                  return "Invalid sampler";
        case CL_INVALID_BINARY:                   return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:            return "Invalid build options";
        case CL_INVALID_PROGRAM:                  return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:       return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:              return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:        return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                   return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                 return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:              return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:           return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:          return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:           return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:            return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:          return "Invalid event wait list";
        case CL_INVALID_EVENT:                    return "Invalid event";
        case CL_INVALID_OPERATION:                return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:              return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                return "Invalid mip-map level";
        default:                                  return "Unknown";
    }
}