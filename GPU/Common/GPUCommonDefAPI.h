// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUCommonDefAPI.h
/// \author David Rohr

#ifndef GPUCOMMONDEFAPI_H
#define GPUCOMMONDEFAPI_H
// clang-format off

#ifndef GPUCOMMONDEF_H
  #error Please include GPUCommonDef.h!
#endif

//Define macros for GPU keywords. i-version defines inline functions.
//All host-functions in GPU code are automatically inlined, to avoid duplicate symbols.
//For non-inline host only functions, use no keyword at all!
#if !defined(GPUCA_GPUCODE) //For host / ROOT dictionary
  #define GPUd()
  #define GPUdDefault()
  #define GPUdi() inline
  #define GPUh()
  #define GPUhi() inline
  #define GPUhd()
  #define GPUhdi() inline
  #define GPUhdni()
  #define GPUg() INVALID_TRIGGER_ERROR_NO_HOST_CODE
  #define GPUshared()
  #define GPUglobal()
  #define GPUconstant()
  #define GPUbarrier()
  #define GPUAtomic(type) type

  struct float4 { float x, y, z, w; };
  struct float3 { float x, y, z; };
  struct float2 { float x; float y; };
  struct uchar2 { unsigned char x, y; };
  struct short2 { short x, y; };
  struct ushort2 { unsigned short x, y; };
  struct int2 { int x, y; };
  struct int3 { int x, y, z; };
  struct int4 { int x, y, z, w; };
  struct uint1 { unsigned int x; };
  struct uint2 { unsigned int x, y; };
  struct uint3 { unsigned int x, y, z; };
  struct uint4 { unsigned int x, y, z, w; };
  struct dim3 { unsigned int x, y, z; };
#elif defined(__OPENCL__) //Defines for OpenCL
  #define GPUd()
  #define GPUdDefault()
  #define GPUdi() inline
  #define GPUh() INVALID_TRIGGER_ERROR_NO_HOST_CODE
  #define GPUhi() INVALID_TRIGGER_ERROR_NO_HOST_CODE
  #define GPUhd() inline
  #define GPUhdi() inline
  #define GPUhdni()
  #define GPUg() __kernel
  #define GPUshared() __local
  #define GPUglobal() __global
  //#define GPUconstant() __constant //TODO: possibly add const __restrict where possible later!
  #define GPUconstant() __global
  #if defined(__OPENCLCPP__) && !defined(__clang__)
    #define GPUbarrier() work_group_barrier(mem_fence::global | mem_fence::local);
    #define GPUAtomic(type) atomic<type>
    static_assert(sizeof(atomic<unsigned int>) == sizeof(unsigned int), "Invalid atomic type");
  #else
    #define GPUbarrier() barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE)
    #define GPUAtomic(type) volatile type
    #ifdef CONSTEXPR
      #undef CONSTEXPR
    #endif
    #if defined(__OPENCLCPP__)
      #define CONSTEXPR __global constexpr
    #else
      #define CONSTEXPR const
    #endif
  #endif
#elif defined(__CUDACC__) //Defines for CUDA
  #define GPUd() __device__
  #define GPUdDefault()
  #define GPUdi() __device__ inline
  #define GPUh() __host__ inline
  #define GPUhi() __host__ inline
  #define GPUhd() __host__ __device__ inline
  #define GPUhdi() __host__ __device__ inline
  #define GPUhdni() __host__ __device__
  #define GPUg() __global__
  #define GPUshared() __shared__
  #define GPUglobal()
  #define GPUconstant()
  #define GPUbarrier() __syncthreads()
  #define GPUAtomic(type) type
#elif defined(__HIPCC__) //Defines for HIP
  #define GPUd() __device__
  #define GPUdDefault() __device__
  #define GPUdi() __device__ inline
  #define GPUh() __host__ inline
  #define GPUhi() __host__ inline
  #define GPUhd() __host__ __device__ inline
  #define GPUhdi() __host__ __device__ inline
  #define GPUhdni() __host__ __device__
  #define GPUg() __global__
  #define GPUshared() __shared__
  #define GPUglobal()
  #define GPUconstant()
  #define GPUbarrier() __syncthreads()
  #define GPUAtomic(type) type
#endif

#if defined(__OPENCL__) && !defined(__OPENCLCPP__) //Other special defines for OpenCL
  #define GPUsharedref() GPUshared()
  #define GPUglobalref() GPUglobal()
  #define GPUconstantref() GPUconstant()
#else //Other defines for the rest
  #define GPUsharedref()
  #define GPUglobalref()
  #define GPUconstantref()
#endif

// clang-format on
#endif
