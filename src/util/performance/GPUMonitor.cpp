#include "GPUMonitor.hpp"


GPUMonitor::GPUMonitor() {
	
	cudaGetDeviceCount(&numGpus);
	std::cout << "GPUs attached: " << numGpus << std::endl;

	cudaGetDeviceProperties(&device, 0);

}


void GPUMonitor::GetMemoryStats(long long int &bytesUsed, long long int &bytesTotal)
{
	cudaSetDevice(0);
	size_t memFree;
	size_t memTotal;
	cudaMemGetInfo(&memFree, &memTotal);

	bytesUsed = memTotal - memFree;
	bytesTotal = memTotal;

	//std::cout << "Used: " << memTotal - memFree << " Free: " << memFree << " Total: " << memTotal << std::endl;
}

float GPUMonitor::GetMemoryUsagePercent() {
	long long int gpuBytesUsed;
	long long int gpuBytesTotal;
	GetMemoryStats(gpuBytesUsed, gpuBytesTotal);
	return (float)gpuBytesUsed / (float)gpuBytesTotal;
}