/*Copyright 2013 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
#limitations under the License.*/

#pragma once

#ifdef __APPLE__
//typedef unsigned long long uint64;
//typedef long long int64;
#elif _WIN32
typedef unsigned __int64   uint64;
typedef signed __int64     int64;

#endif

typedef unsigned char      uint8;
typedef signed char        int8;
typedef unsigned short     uint16;
typedef signed short       int16;
typedef unsigned int       uint32;
typedef uint32             uint;
typedef signed int         int32;

#ifdef _WIN32
const uint8  UINT8_MIN  = 0;
const uint8  UINT8_MAX  = 0xFFU;
const uint16 UINT16_MIN = 0;
const uint16 UINT16_MAX = 0xFFFFU;
const uint32 UINT32_MIN = 0;
const uint32 UINT32_MAX = 0xFFFFFFFFU;
const uint64 UINT64_MIN = 0;
const uint64 UINT64_MAX = 0xFFFFFFFFFFFFFFFFui64;

const int8  INT8_MIN  = -128;
const int8  INT8_MAX  = 127;
const int16 INT16_MIN = -32768;
const int16 INT16_MAX = 32767;
const int32 INT32_MIN = (-2147483647 - 1);
const int32 INT32_MAX = 2147483647;
const int64 INT64_MIN = (-9223372036854775807i64 - 1);
const int64 INT64_MAX = 9223372036854775807i64;
#endif



// The plan is to support better compression modes in the future.
enum eCompressionMode
{
    cMode_A =0, //mode 0 = 16 bit indicies everywhere
};





// We allow the texture to be broken up into 4x4 blocks, this is simply a helper class to facilitate that.
struct Block
{
    RGBAColor pixels[16]; //assumes 4x4 pixels, raster order
};

void compressFlipbook(std::vector<std::string>& filenameList, std::string outPrefix,const eCompressionMode compressionMode);


