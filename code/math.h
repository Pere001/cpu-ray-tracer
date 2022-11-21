
// 
// Basic math functions
//

// Most functions are named with no suffix in the float version, and with suffix in
// the versions with other types. 


#ifndef MATH_H
#define MATH_H

#define PI 3.141592653589793238f
#define MAX_INTEGER_FLOAT 16777216 // (2^24) The number previous to the first integer that cannot be represented as float.
#define MAX_S32 0x7FFFFFFF  //  2,147,483,648
#define MIN_S32 0x80000000  // -2,147,483,647
#define MAX_U32 0xFFFFFFFFU //  4,294,967,295
#define MAX_F32 340282346638528859811704183484516925440.f // 3.4028234e+38 or 0x7F7FFFFF

#define SQUARE(x) ((x)*(x))


inline f32 Square(f32 x){
    f32 result = x*x;
    return result;
}

inline u32 SimpleHash(u32 a){
    a ^= 0xa3c59ac3; // 2747636419 // gota do this cuz literals are signed :V
    a *= 0x9E3779B9; // 2654435769
    a ^= a >> 16;
    a *= 0x9E3779B9; // 2654435769
    a ^= a >> 16;
    a *= 0x9E3779B9; // 2654435769
    return a;
}
// This is about 40-100% faster than SimpleHash. 
inline u32 FastHash(u32 a){
    a ^= 0xa3c59ac3;
    a *= 0x9E3779B9;
    a ^= a >> 16;
    a *= 0x9E3779B9; // 2654435769
    return a;
}

//
// <math.h> wrappers
//
#include <math.h>

f32 Pow(f32 base, f32 exponent){
    f32 result = pow(base, exponent);
    return result;
}
inline f32 Cos(f32 x){
    f32 result = cos(x);
    return result;
}
inline f32 Sin(f32 x){
    f32 result = sin(x);
    return result;
}
inline f32 Tan(f32 value){
    f32 result = tanf(value);
    return result;
}

// Arc Cosine. Input range: [-1, 1]. Return range: [0, pi]
inline f32 ACos(f32 x){
    AssertRange(-1.f, x, 1.f);
    f32 result = acos(x);
    return result;
}
// Arc Cosine. Input range: [-1, 1]. Return range: [-pi/2, pi/2]
inline f32 ASin(f32 x){
    AssertRange(-1.f, x, 1.f);
    f32 result = asin(x);
    return result;
}
// Arc Cosine. Return range: [-pi/2, pi/2]
inline f32 ATan(f32 x){
    f32 result = atan(x);
    return result;
}
// Arc Tangent of y/x. Return range: [-pi/2, pi/2]
inline f32 ATan2(f32 y, f32 x){
    f32 result = atan2(y, x);
    return result;
}
inline f32 SquareRoot(f32 x){
    f32 result = sqrt(x);
    return result;
}


//
// Basic operations
//

// Carmack's square root.
inline f32 SquareRootFast(f32 value){
    f32 x = value * 0.5f;
    f32 y = value;
    s32 i = *(s32 *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(f32 *) &i;
    y = y * (1.5f - (x * y*y));
    y = y * (1.5f - (x * y*y));
    return value * y; // This inverses the inversed sqrt because maths.
    //return y; // This would be the inverse sqrt.
}



//
// Abs, Frac, Floor, Ceil, Round, Max, Min, Lerp, Clamp...
//

inline f32 Abs(f32 value){
    if (value < 0)
        return -value;
    return value;
}
inline s32 AbsS32(s32 value){
    if (value < 0)
        return -value;
    return value;
}
inline f64 AbsF64(f64 value){
    if (value < 0)
        return -value;
    return value;
}

// Fractional part of value.
inline f32 Frac(f32 x){
    return x - (f32)(s64)x;
}

// Remainder of value/mod. 
f32 FMod(f32 value, f32 mod){
    f32 result = value - (mod * (f32)((s64)(value/mod)));
    return result;
}

inline f32 Floor(f32 x){
    f32 result = (f32)(s32)x;
    if (x < 0 && (result != x)){
        result -= 1.f;
    }
    return result;
}

inline f32 Ceil(f32 x){
    f32 result = (f32)(s32)x;
    if (x > 0 && (result != x)){
        result += 1.f;
    }
    return result;
}

inline f32 Round(f32 x){
    f32 result = Floor(x + .5f);
    return result;
}

inline f32 Max(f32 a, f32 b){
    if (a >= b)
        return a;
    return b;
}
inline s32 MaxS32(s32 a, s32 b){
    if (a >= b)
        return a;
    return b;
}
inline u32 MaxU32(u32 a, u32 b){
    if (a >= b)
        return a;
    return b;
}

inline f32 Min(f32 a, f32 b){
    if (a <= b)
        return a;
    return b;
}
inline s32 MinS32(s32 a, s32 b){
    if (a <= b)
        return a;
    return b;
}
inline u32 MinU32(u32 a, u32 b){
    if (a <= b)
        return a;
    return b;
}

inline f32 Lerp(f32 a, f32 b, f32 t){
    f32 result = (1.f - t)*a + b*t;
    return result;
}
inline f32 SLerp(f32 a, f32 b, f32 t){
    f32 result = Lerp(a, b, 6*t*t*t*t*t - 15*t*t*t*t + 10*t*t*t);
    return result;
}

inline f32 Clamp(f32 value, f32 min, f32 max){
    if (value > max) return max;
    if (value < min) return min;
    return value;
}
inline f32 Clamp01(f32 value){
    return Clamp(value, 0, 1.f);
}
inline s32 ClampS32(s32 value, s32 min, s32 max){
    if (value > max) return max;
    if (value < min) return min;
    return value;
}
inline u32 ClampU32(u32 value, u32 min, u32 max){
    if (value > max) return max;
    if (value < min) return min;
    return value;
}
inline f32 LerpClamp(f32 a, f32 b, f32 t){
    return Lerp(a, b, Clamp01(t));
}

inline f32 Sign(f32 x){
    if (x > 0)  return 1.f;
    else if (x) return -1.f;
    else	    return 0;
}
inline s32 Sign(s32 x){
    if (x > 0)  return 1;
    else if (x) return -1;
    else	    return 0;
}
inline f32 SignNonZero(f32 x){
    if (x < 0)
        return -1.f;
    return 1.f;
}

// - Cubic interpolation.
// - F(0)=0    F(.25)=.16     F(.5)=.5    F(.75)=.84     F(1)=1   
f32 Smoothstep(f32 x){
    Assert(x >= 0.0f && x <= 1.0f);
    f32 y = x*x*(3.0f - 2.0f*x);
    return y;
}
f32 SmoothstepClamp(f32 x){
    return Smoothstep(Clamp01(x));
}

// Returns (numerator / denominator) or n.
inline f32 SafeDivideN(f32 numerator, f32 denominator, f32 n){
    return (denominator == 0 ? n : numerator/denominator);
}
// Returns (numerator / denominator) or 1.
inline f32 SafeDivide0(f32 numerator, f32 denominator){
    return SafeDivideN(numerator, denominator, 0);
}
// Returns (numerator / denominator) or 1.
inline f32 SafeDivide1(f32 numerator, f32 denominator){
    return SafeDivideN(numerator, denominator, 1);
}



//
// Angles
//

inline f32 DegreesToRadians(f32 degrees){
    f32 radians = degrees*PI/180.f;
    return radians;
}
inline f32 RadiansToDegrees(f32 radians){
    f32 degrees = radians*180.f/PI;
    return degrees;
}

// NOTE: "normalzied angles" are [-pi, pi]. Most functions work with non-normalized
// angles, and the functions that expect normalized angles make that clear.

// Range: [-pi, pi]
inline f32 NormalizeAngle(f32 a){
    f32 result = FMod(a, 2*PI);
    if (result < 0)
        result += 2*PI;
    if (result > PI)
        result -= 2*PI;
    return result;
}
// Return range: [0, 2*pi]
inline f32 NormalizeAnglePositive(f32 a){
    f32 result = FMod(a, 2*PI);
    if (result < 0)
        result += 2*PI;
    return result;
}

// Returns the angle that must be added to the second angle to get to
// the first angle (normalized). Return range: [-pi, pi]
f32 AngleDifference(f32 to, f32 from){
    f32 result = NormalizeAngle(to - from);
    return result;
}

// Return range: [-pi, pi]
f32 FlipAngleX(f32 angle){
    angle = NormalizeAnglePositive(angle);
    f32 result = PI - angle;
    return result;
}

// Inputs don't need to be normalized. Can extrapolate. Result is not normalized.
inline f32 LerpAngle(f32 a, f32 b, f32 t){
    f32 diff = AngleDifference(b, a);
    f32 result = a + diff*t;
    return result;
}

// Limits 'a' to range 'limit0' to 'limit1' in CW direction.
// if 'limit0' == 'limit1' the only result will be that limit.
// if  'limit0' == 0 && 'limit1' == 2*PI the result can be any angle.
// Return range: [-pi, pi]
f32 ClampAngle(f32 a, f32 limit0, f32 limit1){
    f32 angle = NormalizeAnglePositive(a);
    if (limit0 != 0 && limit1 != 2*PI){ // Limited range
        limit0 = NormalizeAnglePositive(limit0);
        limit1 = NormalizeAnglePositive(limit1);

        if (limit0 <= limit1){
            if (angle < limit0 || angle > limit1){
                if (AngleDifference(angle, (limit1 + limit0)/2) < 0)
                    angle = limit0;
                else
                    angle = limit1;
            }
        }else{
            if (angle < limit0 && angle > limit1){
                if (AngleDifference(angle, (limit1 + limit0)/2) < 0)
                    angle = limit1;
                else
                    angle = limit0;
            }
        }
    }
    if (angle > PI)
        angle -= 2*PI;
    return angle;
}



//
// Map functions
//

// Min doesn't need to be lower than max.
inline f32 MapRangeTo01(f32 x, f32 min, f32 max){
    return (x - min)/(max - min);
}
// Min doesn't need to be lower than max.
inline f32 MapRangeTo01Clamp(f32 x, f32 min, f32 max){
    return Clamp01((x - min)/(max - min));
}

// - min is allowed to be greater than max.
inline f32 MapRangeToRangeClamp(f32 x, f32 xMin, f32 xMax, f32 yMin, f32 yMax){
    f32 t = Clamp01((x - xMin)/(xMax - xMin));
    f32 y = Lerp(yMin, yMax, t);
    return y;
}
// - min is allowed to be greater than max.
inline f32 MapRangeToRange(f32 x, f32 xMin, f32 xMax, f32 yMin, f32 yMax){
    f32 t = ((x - xMin)/(xMax - xMin));
    f32 y = Lerp(yMin, yMax, t);
    return y;
}
// F(0)=0,    F(.5f)=.75   F(1)=1
inline f32 Map01ToReverseSquare(f32 t){
    return 1.f - Square(1.f - t);
}

// - f(0)=0   f(.5)=1   f(1)=0
inline f32 Map01ToBellSin(f32 x){
    Assert(x >= 0.0f && x <= 1.0f);
    f32 y = .5f + .5f*Sin(2*PI*(x + .75f));
    return y;
}



//
// V2
//
struct v2{
    f32 x;
    f32 y;
};

inline v2 V2(f32 x, f32 y){
    v2 result = {x, y};
    return result;
}
inline v2 V2(f32 xy){
    v2 result = {xy, xy};
    return result;
}

inline v2 operator+(v2 a, v2 b){
    v2 result = {a.x + b.x, a.y + b.y};
    return result;
}

inline v2 operator-(v2 a, v2 b){
    v2 result = {a.x - b.x, a.y - b.y};
    return result;
}

inline v2 operator-(v2 a){
    v2 result = {-a.x, -a.y};
    return result;
}

inline v2 operator/(v2 a, f32 scalar){
    v2 result = {a.x/scalar, a.y/scalar};
    return result;
}
inline v2 operator*(v2 a, f32 scalar){
    v2 result = {a.x*scalar, a.y*scalar};
    return result;
}
inline v2 operator/(f32 scalar, v2 a){
    v2 result = {scalar/a.x, scalar/a.y};
    return result;
}
inline v2 operator*(f32 scalar, v2 a){
    v2 result = {a.x*scalar, a.y*scalar};
    return result;
}

inline v2 &operator+=(v2 &a, v2 b){
    a = a + b;
    return a;
}
inline v2 &operator*=(v2 &a, f32 scalar){
    a = a * scalar;
    return a;
}
inline v2 &operator-=(v2 &a, v2 b){
    a = a - b;
    return a;
}
inline v2 &operator/=(v2 &a, f32 scalar){
    a = a / scalar;
    return a;
}
inline b32 operator==(v2 a, v2 b){
    b32 result = (a.x == b.x) && (a.y == b.y);
    return result;
}
inline b32 operator!=(v2 a, v2 b){
    b32 result = (a.x != b.x) || (a.y != b.y);
    return result;
}

inline f32 Dot(v2 a, v2 b){
    f32 result = a.x*b.x + a.y*b.y;
    return result;
}

inline f32 Cross(v2 a, v2 b){
    f32 result = a.x*b.y - a.y*b.x;
    return result;
}

inline v2 Hadamard(v2 a, v2 b){
    v2 result = {a.x*b.x, a.y*b.y};
    return result;
}

inline f32 Length(v2 a){
    f32 result = SquareRoot(a.x*a.x + a.y*a.y);
    return result;
}
inline f32 LengthSqr(v2 a){
    f32 result = a.x*a.x + a.y*a.y;
    return result;
}


inline b32 PointInRectangle(v2 p, v2 r0, v2 r1){
    if (p.x < r0.x || p.x > r1.x || p.y < r0.y || p.y > r1.y)
        return false;
    return true;
}

inline v2 LerpV2(v2 a, v2 b, f32 t){
    v2 result = (1-t)*a + b*t;
    return result;
}

// Angle and direction stuff

// (Input can be 0,0).
inline f32 AngleOf(v2 a){
    if (a.x && a.y)
        return ATan2(a.y, a.x);
    return 0;
}

inline v2 V2LengthDir(f32 length, f32 direction){
    v2 result = {Cos(direction)*length, Sin(direction)*length};
    return result;
}

// Return range: [-pi, pi].
inline f32 AngleBetween(v2 from, v2 to){
    f32 dot = Dot(from, to);
    f32 det = Cross(from, to); //from.x*to.y - from.y*to.x; // Determinant
    f32 angle = ATan2(det, dot);
    return angle;
}

inline v2 RotateV2(v2 v, f32 angle){
    f32 sin = Sin(angle), cos = Cos(angle);
    v2 result = {v.x*cos - v.y*sin, v.x*sin + v.y*cos};
    return result;
}

inline v2 Normalize(v2 a){
    v2 result = V2LengthDir(1.f, AngleOf(a));
    return result;
}



//
// Integer V2
//
struct v2s{
    s32 x;
    s32 y;
};

inline v2s V2S(s32 x, s32 y){
    v2s result = {x, y};
    return result;
}
inline v2s V2S(s32 xy){
    v2s result = {xy, xy};
    return result;
}
inline v2s V2S(v2 a){
    v2s result = {(s32)a.x, (s32)a.y};
    return result;
}
inline v2 V2(v2s a){
    v2 result = {(f32)a.x, (f32)a.y};
    return result;
}

inline v2s operator*(s32 a, v2s b){
    v2s result = {a*b.x, a*b.y};
    return result;
}
inline v2s operator*(v2s a, s32 b){
    v2s result = {a.x*b, a.y*b};
    return result;
}
inline v2s operator/(v2s a, s32 b){
    v2s result = {a.x/b, a.y/b};
    return result;
}
inline v2s &operator*=(v2s &a, s32 b){
    a = a*b;
    return a;
}
inline v2s &operator/=(v2s &a, s32 b){
    a = a/b;
    return a;
}

inline v2s operator+(v2s a, v2s b){
    v2s result = {a.x + b.x, a.y + b.y};
    return result;
}
inline v2s &operator+=(v2s &a, v2s b){
    a = a + b;
    return a;
}

inline v2s operator-(v2s a, v2s b){
    v2s result = {a.x - b.x, a.y - b.y};
    return result;
}
inline v2s &operator-=(v2s &a, v2s b){
    a = a - b;
    return a;
}

inline v2s operator-(v2s a){
    v2s result = {-a.x, -a.y};
    return result;
}


inline b32 operator==(v2s a, v2s b){
    b32 result = (a.x == b.x) && (a.y == b.y);
    return result;
}
inline b32 operator!=(v2s a, v2s b){
    b32 result = (a.x != b.x) || (a.y != b.y);
    return result;
}

inline s32 LengthSqr(v2s a){
    s32 result = a.x*a.x + a.y*a.y;
    return result;
}

inline s32 DotV2S(v2s a, v2s b){
    s32 result = a.x*b.x + a.y*b.y;
    return result;
}
inline s32 CrossV2S(v2s a, v2s b){
    s32 result = a.x*b.y - a.y*b.x;
    return result;
}
inline v2s HadamardV2S(v2s a, v2s b){
    v2s result = {a.x*a.x, a.y*a.y};
    return result;
}

//
// V3
//
union v3{
    struct{ f32 x, y, z; };
    struct{ f32 r, g, b; };
    f32 asArray[3];
};
inline v3 V3(f32 x, f32 y, f32 z){
    v3 result = {x, y, z};
    return result;
}
inline v3 V3(f32 xyz){
    v3 result = {xyz, xyz, xyz};
    return result;
}
inline v3 operator*(f32 a, v3 b){
    v3 result = {a*b.x, a*b.y, a*b.z};
    return result;
}
inline v3 operator*(v3 a, f32 b){
    v3 result = {a.x*b, a.y*b, a.z*b};
    return result;
}
inline v3 operator/(v3 a, f32 b){
    v3 result = {a.x/b, a.y/b, a.z/b};
    return result;
}
inline v3 &operator*=(v3 &a, f32 b){
    a = a*b;
    return a;
}
inline v3 &operator/=(v3 &a, f32 b){
    a = a/b;
    return a;
}

inline v3 operator+(v3 a, v3 b){
    v3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}
inline v3 &operator+=(v3 &a, v3 b){
    a = a + b;
    return a;
}

inline v3 operator-(v3 a, v3 b){
    v3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}
inline v3 &operator-=(v3 &a, v3 b){
    a = a - b;
    return a;
}

inline v3 operator-(v3 a){
    v3 result = {-a.x, -a.y, -a.z};
    return result;
}


inline b32 operator==(v3 a, v3 b){
    b32 result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}
inline b32 operator!=(v3 a, v3 b){
    b32 result = (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
    return result;
}

inline f32 Length(v3 a){
    f32 result = SquareRoot(SQUARE(a.x) + SQUARE(a.y) + SQUARE(a.z));
    return result;
}
inline f32 LengthSqr(v3 a){
    f32 result = SQUARE(a.x) + SQUARE(a.y) + SQUARE(a.z);
    return result;
}


inline v3 Normalize(v3 a){
    v3 result;
    f32 length = Length(a);
    if (length > .00000000001f){
        result = a/length;
    }else{
        result = V3(0, 0, 1);
    }
    return result;
}
inline v3 NormalizeNonZero(v3 a){
    v3 result = a/Length(a);
    return result;
}


inline f32 Dot(v3 a, v3 b){
    f32 result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}
inline v3 Cross(v3 a, v3 b){
    v3 result = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
    return result;
}

// - Returns a unit vector perpendicular to 'a'.
inline v3 Perpendicular(v3 a){
    v3 result = Normalize(V3(a.y, -a.x, 0));
    return result;
}


inline f32 SrgbToLinear(f32 sRGB){
    f32 result = (sRGB < 0.04045f ? sRGB/12.92f : Pow((sRGB + 0.055f)/1.055f, 2.4f)); 
    return result;
}
inline f32 LinearToSrgb(f32 linear){
    f32 result = (linear < 0.0031308f ? linear*12.92f : 1.055f*Pow(linear, 1.f/2.4f) - 0.055f);
    return result;
}



//
// 3x3 matrix
//

// Row major
union mat3 {
    f32 p[9];
    struct{
        v3 r0, r1, r2;
    };
    struct{
        f32 p00, p10, p20,
            p01, p11, p21,
            p02, p12, p22;
    };
};
inline mat3 Identity3(){
    mat3 result = {
                    1.f, 0, 0,
                    0, 1.f, 0,
                    0, 0, 1.f };
    return result;
}

inline mat3 operator*(mat3 a, mat3 b){
    mat3 result = {};
    for(s32 r = 0; r < 3; r++){ // Row
        for(s32 c = 0; c < 3; c++){ // Column
            for(s32 i = 0; i < 3; i++){
                result.p[3*r + c] += a.p[3*r + i] * b.p[3*i + c]; 
            }
        }
    }
    return result;
}
inline mat3 &operator*=(mat3 &a, mat3 b){
    a = a*b;
    return a;
}

mat3 operator+(mat3 &a, mat3 b){
    mat3 result;
    for(s32 i = 0; i < 9; i++)
        result.p[i] = a.p[i] + b.p[i];
    return result;
}
mat3 operator-(mat3 &a, mat3 b){
    mat3 result;
    for(s32 i = 0; i < 9; i++)
        result.p[i] = a.p[i] - b.p[i];
    return result;
}
mat3 operator-(mat3 &a){
    for(s32 i = 0; i < 9; i++)
        a.p[i] = -a.p[i];
    return a;
}

mat3 Transpose(mat3 a){
    mat3 result = {
                    a.p00, a.p01, a.p02,
                    a.p10, a.p11, a.p12,
                    a.p20, a.p21, a.p22 };
    return result;
}

mat3 XRotation3(f32 angle){
    f32 c = Cos(angle);
    f32 s = Sin(angle);
    mat3 result = {
                    1.f,   0,   0,
                      0,   c,   s,
                      0,  -s,   c };
    return result;
}

mat3 YRotation3(f32 angle){
    f32 c = Cos(angle);
    f32 s = Sin(angle);
    mat3 result = {
                    c,   0,  -s,
                    0, 1.f,   0,
                    s,   0,   c };
    return result;
}

mat3 ZRotation3(f32 angle){
    f32 c = Cos(angle);
    f32 s = Sin(angle);
    mat3 result = {
                     c,   s,   0,
                    -s,   c,   0,
                     0,   0, 1.f };
    return result;
}

mat3 Scale3(v3 t){
    mat3 result = {
                    t.x, 0, 0,
                    0, t.y, 0,
                    0, 0, t.z };
    return result;
}

v3 MatrixMultiply(v3 v, mat3 m){
    v3 result = {(v.x*m.p00 + v.y*m.p10 + v.z*m.p20),
                 (v.x*m.p01 + v.y*m.p11 + v.z*m.p21),
                 (v.x*m.p02 + v.y*m.p12 + v.z*m.p22)};
    return result;
}


#endif
