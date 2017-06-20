#pragma once

#include <stdio.h>
#include <iostream>

#include <cuda_runtime.h>

using namespace std;

// https://github.com/al42and/cuda-smi/blob/c7e95525d7dd9c2385f3e07a3b78e332a558274b/cuda-smi.cpp

/**
Singleton that allows for querying the GPU's statistics.
THIS IS NOT THREAD SAFE IN C++03 -- USE C++11!
*/
class GPUMonitor {
public:
	//==== SINGLETON STUFF ==============================================//
	static GPUMonitor& GetInstance()
	{
		static GPUMonitor instance; // Guaranteed to be destroyed.
									// Instantiated on first use.
		return instance;
	}
	//==== END SINGLETON STUFF ==============================================//

	void GetMemoryStats(long long int &bytesUsed, long long int &bytesTotal);
	float GetMemoryUsagePercent();

private:
	//==== SINGLETON STUFF ==============================================//
	GPUMonitor();
	// C++11:
	// Stop the compiler from generating copy methods for the object
	GPUMonitor(GPUMonitor const&) = delete;
	void operator=(GPUMonitor const&) = delete;
	//==== END SINGLETON STUFF ==============================================//

	int numGpus = 0;
	struct cudaDeviceProp device;


};