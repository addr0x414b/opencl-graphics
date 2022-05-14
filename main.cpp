#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

void checkError(cl_int err, std::string location) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + location + ": " +
				std::to_string(err));
	}
}

int main() {
	cl::Platform platform = cl::Platform::get();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	cl::Device device = devices.front();

	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
	auto name = device.getInfo<CL_DEVICE_NAME>();
	auto version = device.getInfo<CL_DEVICE_VERSION>();
	std::cout << vendor << ", " << name << ", " << version << std::endl;

	cl::Context context(device);

	std::ifstream pipelineFile("../src/opencl/graphicsPipeline.cl");
	std::string pipelineSrc(std::istreambuf_iterator<char>(pipelineFile),
			(std::istreambuf_iterator<char>()));

	cl_int err;
	cl::Program pipelineProgram(context, pipelineSrc.c_str(), CL_TRUE, &err);
	checkError(err, "PipelineProgram");

	cl::CommandQueue queue(context, device, 0, &err);
	checkError(err, "Queue");

	cl::EnqueueArgs args(queue, cl::NDRange(3));

	int nums[] = {2, 2, 5, 4, 9, 9};
	int output[3];

	cl::Buffer numsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			6*sizeof(int), nums, &err);
	checkError(err, "NumsBuffer");

	cl::Buffer outputBuffer(context, CL_MEM_WRITE_ONLY, 3*sizeof(int), NULL, &err);
	checkError(err, "OutputBuffer");

	cl::Kernel helloWorldKernel(pipelineProgram, "helloWorld", &err);
	checkError(err, "HelloWorldKernel");
	cl::KernelFunctor<> helloWorld(helloWorldKernel);

	err = helloWorldKernel.setArg(0, sizeof(cl_mem), &numsBuffer);
	checkError(err, "HelloWorldKernel Arg 0");

	err = helloWorldKernel.setArg(1, sizeof(cl_mem), &outputBuffer);
	checkError(err, "HelloWorldKernel Arg 1");

	helloWorld(args);

	queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, 3*sizeof(int), output);

	std::cout << output[0] << ", " << output[1] << ", " << output[2] << std::endl;

}
