/* Copyright (c) <2003-2021> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __CU_COUNTIN_SORT_H__
#define __CU_COUNTIN_SORT_H__

#include <cuda.h>
#include <vector_types.h>
#include <cuda_runtime.h>

#include "ndCudaContext.h"
#include "ndCudaSceneInfo.h"
#include "ndCudaIntrinsics.h"

#define D_COUNTING_SORT_MAX_BLOCK_SIZE  1024

__global__ void ndCudaCountingCellsPrefixScanInternal(unsigned* histogram, unsigned blockCount, unsigned prefixKeySize)
{
	unsigned sum = 0;
	unsigned offset = 0;

	const unsigned threadId = threadIdx.x;
	for (int i = 0; i < blockCount; i++)
	{
		const unsigned count = histogram[offset + threadId];
		histogram[offset + threadId] = sum;
		sum += count;
		offset += prefixKeySize;
	}
	histogram[offset + threadId] = sum;
}

template <typename BufferItem, typename SortKeyPredicate>
__global__ void ndCudaCountingSortCountItemsInternal(const BufferItem* src, unsigned* histogram, unsigned size, SortKeyPredicate sortKey, unsigned prefixKeySize)
{
	__shared__  unsigned cacheBuffer[D_COUNTING_SORT_MAX_BLOCK_SIZE];

	const unsigned blockId = blockIdx.x;
	const unsigned threadId = threadIdx.x;
	
	cacheBuffer[threadId] = 0;
	__syncthreads();
	const unsigned index = threadId + blockDim.x * blockId;
	if (index < size)
	{
		const unsigned key = sortKey(src[index]);
		atomicAdd(&cacheBuffer[key], 1);
	}
	__syncthreads();
	
	const unsigned dstBase = prefixKeySize * blockId;
	histogram[dstBase + threadId] = cacheBuffer[threadId];
}

template <typename BufferItem, typename SortKeyPredicate>
__global__ void ndCudaCountingSortCountShuffleItemsInternal(const BufferItem* src, BufferItem* dst, unsigned* histogram, unsigned size, SortKeyPredicate sortKey, unsigned prefixKeySize)
{
	__shared__  BufferItem cachedCells[D_COUNTING_SORT_MAX_BLOCK_SIZE];
	__shared__  unsigned cacheSortedKey[D_COUNTING_SORT_MAX_BLOCK_SIZE];
	__shared__  unsigned cacheBaseOffset[D_COUNTING_SORT_MAX_BLOCK_SIZE];
	__shared__  unsigned cacheKeyPrefix[D_COUNTING_SORT_MAX_BLOCK_SIZE / 2 + D_COUNTING_SORT_MAX_BLOCK_SIZE + 1];
	__shared__  unsigned cacheItemCount[D_COUNTING_SORT_MAX_BLOCK_SIZE / 2 + D_COUNTING_SORT_MAX_BLOCK_SIZE + 1];

	const unsigned blockId = blockIdx.x;
	const unsigned threadId = threadIdx.x;
	const unsigned blocks = (size + blockDim.x - 1) / blockDim.x;
	
	const unsigned index = threadId + blockDim.x * blockId;
	const unsigned lastRoadOffset = blocks * D_COUNTING_SORT_MAX_BLOCK_SIZE;

	cacheSortedKey[threadId] = (D_COUNTING_SORT_MAX_BLOCK_SIZE << 16) | threadId;
	if (index < size)
	{
		cachedCells[threadId] = src[index];
		const unsigned key = sortKey(src[index]);
		cacheSortedKey[threadId] = (key << 16) | threadId;
	}
	
	cacheKeyPrefix[threadId] = 0;
	cacheItemCount[threadId] = 0;
	__syncthreads();

	const unsigned prefixBase = D_COUNTING_SORT_MAX_BLOCK_SIZE / 2;
	const unsigned srcOffset = blockId * D_COUNTING_SORT_MAX_BLOCK_SIZE;
	cacheBaseOffset[threadId] = histogram[srcOffset + threadId];
	cacheKeyPrefix[prefixBase + 1 + threadId] = histogram[lastRoadOffset + threadId];
	cacheItemCount[prefixBase + 1 + threadId] = histogram[srcOffset + D_COUNTING_SORT_MAX_BLOCK_SIZE + threadId] - cacheBaseOffset[threadId];
	
	const int threadId0 = threadId;
	for (int k = 2; k <= D_COUNTING_SORT_MAX_BLOCK_SIZE; k *= 2)
	{
		for (int j = k / 2; j > 0; j /= 2)
		{
			const int threadId1 = threadId0 ^ j;
			if (threadId1 > threadId0)
			{
				const int a = cacheSortedKey[threadId0];
				const int b = cacheSortedKey[threadId1];
				const int mask0 = (-(threadId0 & k)) >> 31;
				const int mask1 = -(a > b);
				const int mask = mask0 ^ mask1;
				cacheSortedKey[threadId0] = (b & mask) | (a & ~mask);
				cacheSortedKey[threadId1] = (a & mask) | (b & ~mask);
			}
			__syncthreads();
		}
	}
	
	for (int i = 1; i < D_COUNTING_SORT_MAX_BLOCK_SIZE; i = i << 1)
	{
		const unsigned prefixSum = cacheKeyPrefix[prefixBase + threadId] + cacheKeyPrefix[prefixBase - i + threadId];
		const unsigned countSum = cacheItemCount[prefixBase + threadId] + cacheItemCount[prefixBase - i + threadId];
		__syncthreads();
		cacheKeyPrefix[prefixBase + threadId] = prefixSum;
		cacheItemCount[prefixBase + threadId] = countSum;
		__syncthreads();
	}
	
	if (index < size)
	{
		const unsigned keyValue = cacheSortedKey[threadId];
		const unsigned key = keyValue >> 16;
		const unsigned threadIdBase = cacheItemCount[prefixBase + key];
	
		const unsigned dstOffset0 = threadId - threadIdBase;
		const unsigned dstOffset1 = cacheKeyPrefix[prefixBase + key] + cacheBaseOffset[key];
		dst[dstOffset0 + dstOffset1] = cachedCells[keyValue & 0xffff];
	}
}

template <typename BufferItem, typename SortKeyPredicate>
__global__ void ndCudaCountingSortCountSanityCheckInternal(ndCudaSceneInfo& info, const BufferItem* dst, unsigned size, SortKeyPredicate sortKey)
{
	const unsigned index = blockIdx.x * blockDim.x + blockIdx.x;
	if (index > 1)
	{
		const unsigned key0 = sortKey(dst[index - 0]);
		const unsigned key1 = sortKey(dst[index - 1]);
		if (key0 > key1)
		{
			info.m_frameIsValid = 0;
		}
	}
}


inline unsigned __device__ ndCudaCountingSortCalculateScanPrefixSize(unsigned items, unsigned keySize)
{
	unsigned blocks = (items + D_COUNTING_SORT_MAX_BLOCK_SIZE - 1) / D_COUNTING_SORT_MAX_BLOCK_SIZE;
	return keySize * (blocks + 2);
}

template <typename BufferItem>
struct CudaCountingSortInfo
{
	BufferItem* m_dst;
	const BufferItem* m_src;
	unsigned* m_histogram;
	unsigned m_itemsCount;
	unsigned m_keySize;

	static __global__ void ndCudaCountingSort(ndCudaSceneInfo& info, GetInfoPredicate<BufferItem> GetInfo)
	{

	}
};

//template <typename BufferItem, typename SortKeyPredicate>
//__global__ void ndCudaCountingSort(ndCudaSceneInfo& info, const BufferItem* src, BufferItem* dst, unsigned* histogram, unsigned size, SortKeyPredicate sortKey, const unsigned keySize)
template <typename GetInfoPredicate<typename BufferItem>>
__global__ void ndCudaCountingSort(ndCudaSceneInfo& info, GetInfoPredicate<BufferItem> GetInfo)
{
	if (!info.m_frameIsValid)
	{
		printf("skipping frame %d  function %s  line %d\n", info.m_frameCount, __FUNCTION__, __LINE__);
		return;
	}

	CudaCountingSortInfo<BufferItem> sortInfo;
	GetInfo(info, sortInfo);

	//const unsigned blocks = (size + D_COUNTING_SORT_MAX_BLOCK_SIZE -1 ) / D_COUNTING_SORT_MAX_BLOCK_SIZE;
	//ndCudaCountingSortCountItemsInternal << <blocks, D_COUNTING_SORT_MAX_BLOCK_SIZE, 0 >> > (src, histogram, size, sortKey, keySize);
	//ndCudaCountingCellsPrefixScanInternal << <1, keySize, 0 >> > (histogram, blocks);
	//ndCudaCountingSortCountShuffleItemsInternal << <blocks, D_COUNTING_SORT_MAX_BLOCK_SIZE, 0 >> > (src, dst, histogram, size, sortKey);
	
	//printf("%s\n", text);
	//printf("size: %d  blocks: %d\n", size, blocks);
	//printf("histogram: ");
	//for (int i = 0; i < 16; i++)
	//{
	//	printf("%d ", histogram[i]);
	//}
	//printf("\n");
	//
	//printf("histogram: ");
	//for (int i = 0; i < 16; i++)
	//{
	//	printf("%d ", histogram[i + keySize]);
	//}
	//printf("\n");

	//printf("function: %s\n", __FUNCTION__);
	//for (int i = 0; i < info.m_bodyAabbCell.m_size; i++)
	//{
	//	ndCudaBodyAabbCell cell;
	//	//cell.m_value = dst[i];
	//	cell.m_value = src[i];
	//	printf("x(%d) y(%d) z(%d) id(%d)\n", cell.m_x, cell.m_y, cell.m_z, cell.m_id);
	//}
	//printf("\n");

	
	#ifdef _DEBUG
	//ndCudaCountingSortCountSanityCheckInternal << <blocks, D_COUNTING_SORT_MAX_BLOCK_SIZE, 0 >> > (info, dst, size, sortKey);
	//if (info.m_frameIsValid == 0)
	//{
	//	printf("skipping frame %d  function %s  line %d\n", info.m_frameCount, __FUNCTION__, __LINE__);
	//}
	#endif
}

#endif