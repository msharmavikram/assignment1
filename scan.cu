// MP Scan
// Given a list (lst) of length n
// Output its prefix sum = {lst[0], lst[0] + lst[1], lst[0] + lst[1] + ...
// +
// lst[n-1]}

#include <wb.h>

#define BLOCK_SIZE 512 //@@ You can change this

#define wbCheck(stmt)                                                     \
  do {                                                                    \
    cudaError_t err = stmt;                                               \
    if (err != cudaSuccess) {                                             \
      wbLog(ERROR, "Failed to run stmt ", #stmt);                         \
      wbLog(ERROR, "Got CUDA error ...  ", cudaGetErrorString(err));      \
      return -1;                                                          \
    }                                                                     \
  } while (0)

/*	  
void GPUMalloc (float **d_A, int size){

   cudaError_t err = cudaMalloc((void **) d_A, size);

   if (err != cudaSuccess){
      printf("%s in %s at line %d\n",cudaGetErrorString(err),__FILE__,__LINE__);
      exit(EXIT_FAILURE);
   }
}*/	  
	  
__global__ void finalsum(float *output, float *input, int len) {
	//For first block dont compute. results are already final. 
	if(blockIdx.x == 0) return;
	
	unsigned int idx = 2* blockIdx.x * BLOCK_SIZE + threadIdx.x;
	
	if (idx < len) 
		output[idx] += input[blockIdx.x -1]; // final value 
	if ((idx + BLOCK_SIZE) < len)
		output[idx+BLOCK_SIZE] += input[blockIdx.x-1];
}
	
__global__ void scan(float *input, float *output, float *intermediate, int len) {
  //@@ Modify the body of this function to complete the functionality of
  //@@ the scan on the device
  //@@ You may need multiple kernel calls; write your kernels before this
  //@@ function and call them from the host

  //@@ This is Brent Kung scan kernel implementation. 
  __shared__ float sharedMem[BLOCK_SIZE*2]; 
  int idx = 2*blockIdx.x * BLOCK_SIZE + threadIdx.x; 
  //@@ get two set of values from the memory. i and i+block_size
  if( idx <len) 
	  sharedMem[threadIdx.x] = input[idx];
  else 
	  sharedMem[threadIdx.x] = 0.0;
  if( idx+ BLOCK_SIZE <len) 
	  sharedMem[threadIdx.x+ BLOCK_SIZE] = input[idx+ BLOCK_SIZE];
  else 
	  sharedMem[threadIdx.x+BLOCK_SIZE]  = 0.0;
	  
  __syncthreads(); //sync all inputs
  //forward path  
  for (unsigned int stride = 1; stride <= BLOCK_SIZE;stride *=2){
	int index = (threadIdx.x+1) * 2* stride -1; 
	if( index < 2*BLOCK_SIZE) 
		sharedMem[index] += sharedMem[index-stride]; 
        __syncthreads(); //sync all inputs
  }
  
  //backward path -- reduction
  for (unsigned int stride = BLOCK_SIZE/2; stride >0 ; stride /=2){
	int index = (threadIdx.x + 1)*stride*2 -1;
	if(index + stride < 2* BLOCK_SIZE)
	        sharedMem[index+stride] += sharedMem[index];
	__syncthreads();//wait until previous compute are over. 
  }
  
  if(idx <len) output[idx] = sharedMem[threadIdx.x];
  if(idx+ BLOCK_SIZE < len) output[idx+BLOCK_SIZE] = sharedMem[threadIdx.x + BLOCK_SIZE];
  
  //if its first kernel then do add data to intermediate stage 
  if(intermediate !=NULL && threadIdx.x ==0) 
	  intermediate[blockIdx.x] = sharedMem[2*BLOCK_SIZE-1];
  
}

int main(int argc, char **argv) {
  wbArg_t args;
  float *hostInput;  // The input 1D list
  float *hostOutput; // The output list
  float *deviceInput;
  float *deviceOutput;
  float *devicek2Input;
  float *devicek2Output;
  int numElements; // number of elements in the list

  args = wbArg_read(argc, argv);

  wbTime_start(Generic, "Importing data and creating memory on host");
  hostInput = (float *)wbImport(wbArg_getInputFile(args, 0), &numElements);
  hostOutput = (float *)malloc(numElements * sizeof(float));
  wbTime_stop(Generic, "Importing data and creating memory on host");

  wbLog(TRACE, "The number of input elements in the input is ", numElements);

  wbTime_start(GPU, "Allocating GPU memory.");
  wbCheck(cudaMalloc((void **)&deviceInput, numElements * sizeof(float)));
  wbCheck(cudaMalloc((void **)&deviceOutput, numElements * sizeof(float)));
  wbCheck(cudaMalloc((void **)&devicek2Input, BLOCK_SIZE*2 * sizeof(float)));
  wbCheck(cudaMalloc((void **)&devicek2Output, BLOCK_SIZE*2 * sizeof(float)));
  wbTime_stop(GPU, "Allocating GPU memory.");

  wbTime_start(GPU, "Clearing output memory.");
  wbCheck(cudaMemset(deviceOutput, 0, numElements * sizeof(float)));
  wbTime_stop(GPU, "Clearing output memory.");

  wbTime_start(GPU, "Copying input memory to the GPU.");
  wbCheck(cudaMemcpy(deviceInput, hostInput, numElements * sizeof(float),
                     cudaMemcpyHostToDevice));
  wbTime_stop(GPU, "Copying input memory to the GPU.");

  //@@ Initialize the grid and block dimensions here
  dim3 dimGrid((numElements-1)/(BLOCK_SIZE*2)+1, 1, 1);// not doing optimization as in reduction.
  dim3 dimBlock(BLOCK_SIZE, 1, 1);
 
  wbTime_start(Compute, "Performing CUDA computation");
  //@@ Modify this to complete the functionality of the scan
  //@@ on the deivce
  //First kernel does Bernt kung scan
  scan<<<dimGrid, dimBlock>>>(deviceInput, deviceOutput, devicek2Input, numElements);
  cudaDeviceSynchronize();
  //second kernel is same as scan but over aggregated sum of the first results. 
  // second kernel is one block only and it should write anything out.
  scan<<<dim3(1,1,1), dimBlock>>>(devicek2Input, devicek2Output, NULL, BLOCK_SIZE*2);
  cudaDeviceSynchronize();
  //third kernel adder of each results. 
  // Half of the elements in the deviceOutput are done compute. 
  //size of this kernel will be equal to size of input kernel. 
  finalsum<<<dimGrid, dimBlock>>>(deviceOutput, devicek2Output, numElements); 
  cudaDeviceSynchronize();
  wbTime_stop(Compute, "Performing CUDA computation");

  wbTime_start(Copy, "Copying output memory to the CPU");
  wbCheck(cudaMemcpy(hostOutput, deviceOutput, numElements * sizeof(float),cudaMemcpyDeviceToHost));
  wbTime_stop(Copy, "Copying output memory to the CPU");

  wbTime_start(GPU, "Freeing GPU Memory");
  cudaFree(deviceInput);
  cudaFree(deviceOutput);
  cudaFree(devicek2Input);
  cudaFree(devicek2Output);
  wbTime_stop(GPU, "Freeing GPU Memory");

  wbSolution(args, hostOutput, numElements);

  free(hostInput);
  free(hostOutput);

  return 0;
}

