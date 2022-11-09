
//
// Types, common utilities, macros, etc.
//

#ifndef BASE_H
#define BASE_H

// Edit this if your machine is Big Endian.
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 0


#define CompletePreviousReadsBeforeFutureReads _ReadBarrier() 
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()

#include <stdint.h>
typedef float    f32;
typedef double   f64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u64      b64;
typedef u32      b32;
typedef u16      b16;
typedef u8       b8;

// Can represent both a memory position, and memory size.
typedef uintptr_t umm; // (32-bit: 4 bytes; 64-bit: 8 bytes)
typedef  intptr_t smm; // (32-bit: 4 bytes; 64-bit: 8 bytes)



//
// Some macros
//
#define local_persist static
#define global static
#define internal static

#ifndef NO_ASSERTS
	#define Assert(x) { if (!(x)) { *(int *)0 = 0; } }
#else
	#define Assert(x)
#endif
#define AssertRange(x0, x1, x2) Assert(((x0) <= (x1)) && ((x1) <= (x2)))

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))
#define OffsetOf(type, member) (umm)&(((type *)0)->member)

// Uses the passed pointer's type to calculate the size.
#define ZeroStruct(pointer) ZeroSize((void *)(pointer), sizeof((pointer)[0]))

// Gets the type from the 'pointer' to calculate the size.
#define ZeroArrayPtr(pointer, count) ZeroSize((void *)(pointer), (count)*sizeof((pointer)[0]))

// 'actualArray' must be an ACTUAL ARRAY like int a[25], and it's type and count will be used to calculate the size.
#define ZeroArray(actualArray) ZeroSize((void *)(actualArray), sizeof(actualArray))
inline void ZeroSize(void *ptr, umm size){
	memset(ptr, 0, size);
}
#define SWAP(a, b) {auto temp = (a); (a) = (b); (b) = temp;}


//
// Type conversion
//
inline s32 SafeUmmToS32(umm a){
	Assert(a < 0x7FFFFFFF);
	return (s32)a;
}
inline u32 SafeUmmToU32(umm a){
	Assert(a <= 0xFFFFFFFF);
	return (u32)a;
}
inline u8 SafeUmmToU8(umm a){
	Assert(a <= 0xFF);
	return (u8)a;
}
inline u16 SafeUmmToU16(umm a){
	Assert(a <= 0xFFFF);
	return (u16)a;
}
inline u8 SafeS32ToU8(s32 a){
	Assert((a >= 0) && (a <= 0xFF));
	return (u8)a;
}
inline s8 SafeS32ToS8(s32 a){
	Assert((a >= -128) && (a < 128));
	return (s8)a;
}
inline u16 SafeS32ToU16(s32 a){
	Assert((a >= 0) && (a <= 0xFFFF));
	return (u16)a;
}
inline s16 SafeS32ToS16(s32 a){
	Assert((a >= -32768) && (a < 32768));
	return (u16)a;
}
inline s32 SafeS64ToS32(s64 a){
	Assert((a >= -(s64)0x80000000) && (a < (s64)0x80000000));
	return (s32)a;
}
inline u8 SafeU32ToU8(u32 a){
	Assert(a <= 0xFF);
	return (u8)a;
}
inline s8 SafeU32ToS8(u32 a){
	Assert(a <= 0x7F);
	return (s8)a;
}
inline u16 SafeU32ToU16(u32 a){
	Assert(a <= 0xFFFF);
	return (u16)a;
}
inline s32 SafeU32ToS32(u32 a){
	Assert(a <= 0x7FFFFFFF);
	return (s32)a;
}
inline u16 SafeS16ToU16(s16 a){
	Assert(a >= 0);
	return (u16)a;
}
inline s16 SafeU16ToS16(u16 a){
	Assert(a < 32768);
	return (s16)a;
}
inline u32 SafeU64ToU32(u64 a){
	Assert(a < (u64)0x800000000);
	return (u32)a;
}
inline u16 SafeF32ToU16(f32 a){
	Assert(a >= 0.0f && a < 65536.0f);
	if (a > 65536.0f/3){ // Ensure that conversion doesn't cause overflow.
		Assert((a > 65536.0f/2) == ((u32)a > 65536/2)); 
	}
	return (u16)a;
}
// Converts f32 to u32 asserting that the float is perfectly representable as integer.
inline u32 SafeF32ToU32(f32 a){
	Assert(0.f <= a && a <= 16777216.f); // 16777216 = last consecutive int representable as float
	Assert(a == (f32)((s32)a));
	return (u32)a;
}
// Converts f32 to s32 asserting that the float is perfectly representable as integer.
inline s32 SafeF32ToS32(f32 a){
	Assert(-16777216.f <= a && a <= 16777216.f);
	Assert(a == (f32)((s32)a));
	return (s32)a;
}


// 
// Buttons
//

struct button_state {
	b32 isDown;
	s32 transitionCount;
};

b32 ButtonWentDown(button_state *b){
	if (b->isDown && (b->transitionCount % 2)){
		return true;
	}
	return false;
}
b32 ButtonWentUp(button_state *b){
	if (!b->isDown && (b->transitionCount % 2)){
		return true;
	}
	return false;
}




#endif
