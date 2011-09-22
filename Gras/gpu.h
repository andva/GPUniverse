#ifndef GPU_H
#define GPU_H

#include <CL\opencl.h>
#include <iostream>
#include <assert.h>
#include <string>
#include "Fileloader.h"
using namespace std;

class Gpu
{
public:
	/*
	*Program skapas av att kompilera en kernel-fil, det görs i programmet och
	*genererar ett gäng kernels (GPU-funktioner) som gör beräkningar med hjälp
	*av grafikkortet. Alla kernels sparas i kernel-arrayen kernel.
	*/
	cl_program program[1];
	cl_kernel kernel[2];
	cl_command_queue cmd_queue;
	cl_context   context;
	cl_device_id device;
	cl_platform_id platform;

	cl_int err;
	size_t returned_size;
	size_t buffer_size;
	
	cl_mem a_mem, b_mem, ans_mem;



	void initCl();
	void setupKernels();
	void runCl(float*, float*, float*, int);
	void deallocateCL();
	string print_cl_errstring(cl_int err);
};
#endif