#pragma once

#include  "defines.h"
#include  "mathtypes.h"

#include  "core/kmemory.h"

#define K_PI 3.14159265358979323846f
#define K_PI_2 2.0f * K_PI
#define K_HALF_PI 0.5f * K_PI
#define K_QUARTER_PI 0.25f * K_PI
#define K_ONE_OVER_PI 1.0f / K_PI
#define K_ONE_OVER_TWO_PI 1.0f / K_PI_2
#define K_SQRT_TWO 1.41421356237309504880f
#define K_SQRT_THREE 1.73205080756887729352f
#define K_SQRT_ONE_OVER_TWO 0.70710678118654752440f
#define K_SQRT_ONE_OVER_THREE 0.57735026918962576450f
#define K_DEG2RAD_MULTIPLIER K_PI / 180.0f
#define K_RAD2DEG_MULTIPLIER 180.0f / K_PI

// The multiplier to convert seconds to milliseconds.
#define K_SEC_TO_MS_MULTIPLIER 1000.0f
// The multiplier to convert milliseconds to seconds.
#define K_MS_TO_SEC_MULTIPLIER 0.001f
// A huge number that should be larger than any valid number used.
#define K_INFINITY 1e30f
// Smallest positive number where 1.0 + FLOAT_EPSILON != 0
#define K_FLOAT_EPSILON 1.192092896e-07f
//'<,'> s/([^ ]\)/\U\1
// ------------------------------------------
// General math functions
// ------------------------------------------
f32 ksin(f32 x);
f32 kcos(f32 x);
f32 ktan(f32 x);
f32 kacos(f32 x);
f32 ksqrt(f32 x);
f32 kabs(f32 x);

static inline u8 isPowerOf2(u64 value) {
  return (value != 0) && ((value & (value - 1)) == 0);
}

i32 krandom();
i32 krandomInRange(i32 min, i32 max);

f32 fkrandom();
f32 fkrandomInRange(f32 min, f32 max);

// ------------------------------------------
// Vector 2
// ------------------------------------------

static inline Vec2 vec2Create(f32 x, f32 y) {
  Vec2 outVector;
  outVector.x = x;
  outVector.y = y;
  return outVector;
}

static inline Vec2 vec2Zero() {
  return (Vec2){0.0f, 0.0f};
}

static inline Vec2 vec2One() {
  return (Vec2){1.0f, 1.0f};
}

static inline Vec2 vec2Up() {
  return (Vec2){0.0f, 1.0f};
}

static inline Vec2 vec2Down() {
  return (Vec2){0.0f, -1.0f};
}

static inline Vec2 vec2Left() {
  return (Vec2){-1.0f, 0.0f};
}

static inline Vec2 vec2Right() {
  return (Vec2){1.0f, 0.0f};
}

static inline Vec2 vec2Add(Vec2 vector0, Vec2 vector1) {
  return (Vec2){
    vector0.x + vector1.x,
    vector0.y + vector1.y};
}

static inline Vec2 vec2Sub(Vec2 vector0, Vec2 vector1) {
  return (Vec2){
    vector0.x - vector1.x,
    vector0.y - vector1.y};
}

static inline Vec2 vec2Mul(Vec2 vector0, Vec2 vector1) {
  return (Vec2){
    vector0.x * vector1.x,
    vector0.y * vector1.y};
}

static inline Vec2 vec2Div(Vec2 vector0, Vec2 vector1) {
  return (Vec2){
    vector0.x / vector1.x,
    vector0.y / vector1.y};
}

static inline f32 vec2LengthSquared(Vec2 vector) {
  return vector.x * vector.x + vector.y * vector.y;
}

static inline f32 vec2Length(Vec2 vector) {
  return ksqrt(vec2LengthSquared(vector));
}

static inline void vec2Normalize(Vec2* vector) {
  const f32 length = vec2Length(*vector);
  vector->x /= length;
  vector->y /= length;
}

static inline Vec2 vec2Normalized(Vec2 vector) {
  vec2Normalize(&vector);
  return vector;
}

static inline u8 vec2Compare(Vec2 vector0, Vec2 vector1, f32 tolerance) {
  if (kabs(vector0.x - vector1.x) > tolerance) {
    return false;
  }

  if (kabs(vector0.y - vector1.y) > tolerance) {
    return false;
  }

  return true;
}

static inline f32 vec2Distance(Vec2 vector0, Vec2 vector1) {
  Vec2 d = (Vec2){
    vector0.x - vector1.x,
    vector0.y - vector1.y};
  return vec2Length(d);
}

// ------------------------------------------
// Vector 3
// ------------------------------------------

static inline Vec3 vec3Create(f32 x, f32 y, f32 z) {
  return (Vec3){x, y, z};
}

static inline Vec3 vec3FromVec4(Vec4 vector) {
  return (Vec3){vector.x, vector.y, vector.z};
}

static inline Vec4 vec3ToVec4(Vec3 vector, f32 w) {
  return (Vec4){vector.x, vector.y, vector.z, w};
}

static inline Vec3 vec3Zero() {
  return (Vec3){0.0f, 0.0f, 0.0f};
}

static inline Vec3 vec3One() {
  return (Vec3){1.0f, 1.0f, 1.0f};
}

static inline Vec3 vec3Up() {
  return (Vec3){0.0f, 1.0f, 0.0f};
}

static inline Vec3 vec3Down() {
  return (Vec3){0.0f, -1.0f, 0.0f};
}

static inline Vec3 vec3Left() {
  return (Vec3){-1.0f, 0.0f, 0.0f};
}

static inline Vec3 vec3Right() {
  return (Vec3){1.0f, 0.0f, 0.0f};
}

static inline Vec3 vec3Forward() {
  return (Vec3){0.0f, 0.0f, -1.0f};
}

static inline Vec3 vec3Back() {
  return (Vec3){0.0f, 0.0f, 1.0f};
}

static inline Vec3 vec3Add(Vec3 vector0, Vec3 vector1) {
  return (Vec3){
    vector0.x + vector1.x,
    vector0.y + vector1.y,
    vector0.z + vector1.z};
}

static inline Vec3 vec3Sub(Vec3 vector0, Vec3 vector1) {
  return (Vec3){
    vector0.x - vector1.x,
    vector0.y - vector1.y,
    vector0.z - vector1.z};
}

static inline Vec3 vec3Mul(Vec3 vector0, Vec3 vector1) {
  return (Vec3){
    vector0.x * vector1.x,
    vector0.y * vector1.y,
    vector0.z * vector1.z};
}

static inline Vec3 vec3MulScalar(Vec3 vector0, f32 scalar) {
  return (Vec3){
    vector0.x * scalar,
    vector0.y * scalar,
    vector0.z * scalar};
}

static inline Vec3 vec3Div(Vec3 vector0, Vec3 vector1) {
  return (Vec3){
    vector0.x / vector1.x,
    vector0.y / vector1.y,
    vector0.z / vector1.z};
}

static inline f32 vec3LengthSquared(Vec3 vector) {
  return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

static inline f32 vec3Length(Vec3 vector) {
  return ksqrt(vec3LengthSquared(vector));
}

static inline void vec3Normalize(Vec3* vector) {
  const f32 length = vec3Length(*vector);
  vector->x /= length;
  vector->y /= length;
  vector->z /= length;
}

static inline Vec3 vec3Normalized(Vec3 vector) {
  vec3Normalize(&vector);
  return vector;
}

static inline f32 vec3Dot(Vec3 vector0, Vec3 vector1) {
  f32 p = 0;
  p += vector0.x * vector1.x;
  p += vector0.y * vector1.y;
  p += vector0.z * vector1.z;
  return p;
}

static inline Vec3 vec3Cross(Vec3 vector0, Vec3 vector1) {
  return (Vec3){
    vector0.y * vector1.z - vector0.z * vector1.y,
    vector0.z * vector1.x - vector0.x * vector1.z,
    vector0.x * vector1.y - vector0.y * vector1.x};
}

static inline const u8 vec3Compare(Vec3 vector0, Vec3 vector1, f32 tolerance) {
  if (kabs(vector0.x - vector1.x) > tolerance) {
    return false;
  }

  if (kabs(vector0.y - vector1.y) > tolerance) {
    return false;
  }

  if (kabs(vector0.z - vector1.z) > tolerance) {
    return false;
  }

  return true;
}

static inline f32 vec3Distance(Vec3 vector0, Vec3 vector1) {
  Vec3 d = (Vec3){
    vector0.x - vector1.x,
    vector0.y - vector1.y,
    vector0.z - vector1.z};
  return vec3Length(d);
}


// ------------------------------------------
// Vector 4
// ------------------------------------------

static inline Vec4 vec4Create(f32 x, f32 y, f32 z, f32 w) {
  Vec4 outVector;
#if defined(KUSESIMD)
  outVector.data = MmSetrPs(x, y, z, w);
#else
  outVector.x = x;
  outVector.y = y;
  outVector.z = z;
  outVector.w = w;
#endif
  return outVector;
}

static inline Vec3 vec4ToVec3(Vec4 vector) {
  return (Vec3){vector.x, vector.y, vector.z};
}

static inline Vec4 vec4FromVec3(Vec3 vector, f32 w) {
#if defined(KUSESIMD)
  Vec4 outVector;
  outVector.data = MmSetrPs(x, y, z, w);
  return outVector;
#else
  return (Vec4){vector.x, vector.y, vector.z, w};
#endif
}

static inline Vec4 vec4Zero() {
  return (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
}

static inline Vec4 vec4One() {
  return (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
}

static inline Vec4 vec4Add(Vec4 vector0, Vec4 vector1) {
  Vec4 result;
  for (u64 i = 0; i < 4; ++i) {
    result.elements[i] = vector0.elements[i] + vector1.elements[i];
  }
  return result;
}

static inline Vec4 vec4Sub(Vec4 vector0, Vec4 vector1) {
  Vec4 result;
  for (u64 i = 0; i < 4; ++i) {
    result.elements[i] = vector0.elements[i] - vector1.elements[i];
  }
  return result;
}

static inline Vec4 vec4Mul(Vec4 vector0, Vec4 vector1) {
  Vec4 result;
  for (u64 i = 0; i < 4; ++i) {
    result.elements[i] = vector0.elements[i] * vector1.elements[i];
  }
  return result;
}

static inline Vec4 vec4Div(Vec4 vector0, Vec4 vector1) {
  Vec4 result;
  for (u64 i = 0; i < 4; ++i) {
    result.elements[i] = vector0.elements[i] / vector1.elements[i];
  }
  return result;
}

static inline f32 vec4LengthSquared(Vec4 vector) {
  return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

static inline f32 vec4Length(Vec4 vector) {
  return ksqrt(vec4LengthSquared(vector));
}

static inline void vec4Normalize(Vec4* vector) {
  const f32 length = vec4Length(*vector);
  vector->x /= length;
  vector->y /= length;
  vector->z /= length;
  vector->w /= length;
}

static inline Vec4 vec4Normalized(Vec4 vector) {
  vec4Normalize(&vector);
  return vector;
}

static inline f32 vec4DotF32(
  f32 a0, f32 a1, f32 a2, f32 a3,
  f32 b0, f32 b1, f32 b2, f32 b3) {
  f32 p;
  p =
    a0 * b0 +
    a1 * b1 +
    a2 * b2 +
    a3 * b3;
  return p;
}

//------------------------------------------IDENTITY MATRIX------------------------
static inline Mat4 mat4Identity(void){
  Mat4 outMatrix;
  kzeroMemory(outMatrix.data, sizeof(f32) * 16);
  outMatrix.data[0] = 1;
  outMatrix.data[5] = 1;
  outMatrix.data[10] = 1;
  outMatrix.data[15] = 1;
  return outMatrix;
}
//------------------------------------------4 X 4 MAT MUL--------------------------
static inline Mat4 mat4Mul(Mat4 first, Mat4 second){
  Mat4 outmatrix    = mat4Identity();
  const f32*  ptr1  = first.data;
  const f32*  ptr2  = second.data;
  f32*  dstptr= outmatrix.data;
  for(i32 i = 0; i < 4; ++i){
    for(i32 j = 0; j < 4; ++j){
      *dstptr = ptr1[0] * ptr2[0  + j] +
        ptr1[1] * ptr2[4  + j] +
        ptr1[2] * ptr2[8  + j] +
        ptr1[3] * ptr2[12 + j];
      dstptr += 1;
    }
    ptr1 += 4;
  }
  return outmatrix;
}
//------------------------------------------MAT 4 ORTHOGRPAHIC------------------------
static inline Mat4 mat4Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearclip, f32 farclip){
  Mat4  outmatrix = mat4Identity();
  f32 lr  = 1.0f/(left- right);
  f32 bt  = 1.0f/(bottom - top);
  f32 nf  = 1.0f/(nearclip - farclip);

  outmatrix.data[0] = -2.0f * lr;
  outmatrix.data[5] = -2.0f * bt;
  outmatrix.data[10]= -2.0f * nf;

  outmatrix.data[12]= ( left    + right   ) * lr;
  outmatrix.data[13]= ( top     + bottom  ) * bt;
  outmatrix.data[14]= (farclip  + nearclip) * nf; 

  return outmatrix;
}
// ------------------------------------------MAT 4 PRESPECTIVE--------------------------
static inline Mat4 mat4Prespective(f32 fovRadians, f32 aspectRatio, f32 nearclip, f32 farclip){
  f32 halftanfov = ktan(fovRadians * 0.5);
  Mat4 outmatrix  = {0};
  outmatrix.data[0] = 1.0f / (aspectRatio * halftanfov); 
  outmatrix.data[5] = 1.0f / halftanfov;
  outmatrix.data[10]= - ((farclip + nearclip)/(farclip - nearclip));
  outmatrix.data[11]= - 1.0f;
  outmatrix.data[14]= - ((2.0f * farclip * nearclip) / (farclip - nearclip));
  return outmatrix;
}
// ------------------------------------------MAT 4 PRESPECTIVE--------------------------
static inline Mat4 mat4LookAt(Vec3 position, Vec3 target, Vec3 up){
  Mat4 outmatrix;
  Vec3 zaxis;
  zaxis.x = target.x - position.x;
  zaxis.y = target.y - position.y;
  zaxis.z = target.z - position.z;
  zaxis = vec3Normalized(zaxis);

  Vec3 xaxis;
  xaxis = vec3Normalized(vec3Cross(zaxis, up));

  Vec3 yaxis;
  yaxis = vec3Cross(xaxis, zaxis);

  outmatrix.data[0] = xaxis.x;
  outmatrix.data[1] = yaxis.x;
  outmatrix.data[2] =-zaxis.x;
  outmatrix.data[3] = 0;

  outmatrix.data[4] = xaxis.y;
  outmatrix.data[5] = yaxis.y;
  outmatrix.data[6] =-zaxis.y;
  outmatrix.data[7] = 0;

  outmatrix.data[8] = xaxis.z;
  outmatrix.data[9] = yaxis.z;
  outmatrix.data[10] =-zaxis.z;
  outmatrix.data[11] = 0;

  outmatrix.data[12] =-vec3Dot(xaxis,position);
  outmatrix.data[13] =-vec3Dot(yaxis,position);
  outmatrix.data[14] = vec3Dot(zaxis,position);
  outmatrix.data[15] = 1.0f;


  return outmatrix;
}

//------------------------------------------MAT 4 IINVERSE------------------------
static inline Mat4 mat4Inverse(Mat4 matrix) {
  const f32* m = matrix.data;

  f32 t0 = m[10] * m[15];
  f32 t1 = m[14] * m[11];
  f32 t2 = m[6] * m[15];
  f32 t3 = m[14] * m[7];
  f32 t4 = m[6] * m[11];
  f32 t5 = m[10] * m[7];
  f32 t6 = m[2] * m[15];
  f32 t7 = m[14] * m[3];
  f32 t8 = m[2] * m[11];
  f32 t9 = m[10] * m[3];
  f32 t10 = m[2] * m[7];
  f32 t11 = m[6] * m[3];
  f32 t12 = m[8] * m[13];
  f32 t13 = m[12] * m[9];
  f32 t14 = m[4] * m[13];
  f32 t15 = m[12] * m[5];
  f32 t16 = m[4] * m[9];
  f32 t17 = m[8] * m[5];
  f32 t18 = m[0] * m[13];
  f32 t19 = m[12] * m[1];
  f32 t20 = m[0] * m[9];
  f32 t21 = m[8] * m[1];
  f32 t22 = m[0] * m[5];
  f32 t23 = m[4] * m[1];

  Mat4 outmatrix;
  f32* o = outmatrix.data;

  o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
  o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
  o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
  o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

  f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

  o[0] = d * o[0];
  o[1] = d * o[1];
  o[2] = d * o[2];
  o[3] = d * o[3];
  o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
  o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
  o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
  o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
  o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
  o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
  o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
  o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
  o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
  o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
  o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
  o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

  return outmatrix;
}

//------------------------------------------MAT 4 TRANSLATION------------------------
static inline Mat4 mat4Translation(Vec3 position) {
  Mat4 outmatrix = mat4Identity();
  outmatrix.data[12] = position.x;
  outmatrix.data[13] = position.y;
  outmatrix.data[14] = position.z;
  return outmatrix;
}

static inline Mat4 mat4Scale(Vec3 scale) {
  Mat4 outmatrix = mat4Identity();
  outmatrix.data[0] = scale.x;
  outmatrix.data[5] = scale.y;
  outmatrix.data[10] = scale.z;
  return outmatrix;
}

static inline Mat4 mat4EulerX(f32 angleRadians) {
  Mat4 outmatrix = mat4Identity();
  f32 c = kcos(angleRadians);
  f32 s = ksin(angleRadians);

  outmatrix.data[5] = c;
  outmatrix.data[6] = s;
  outmatrix.data[9] = -s;
  outmatrix.data[10] = c;
  return outmatrix;
}
static inline Mat4 mat4EulerY(f32 angleRadians) {
  Mat4 outmatrix = mat4Identity();
  f32 c = kcos(angleRadians);
  f32 s = ksin(angleRadians);

  outmatrix.data[0] = c;
  outmatrix.data[2] = -s;
  outmatrix.data[8] = s;
  outmatrix.data[10] = c;
  return outmatrix;
}
static inline Mat4 mat4EulerZ(f32 angleRadians) {
  Mat4 outmatrix = mat4Identity();

  f32 c = kcos(angleRadians);
  f32 s = ksin(angleRadians);

  outmatrix.data[0] = c;
  outmatrix.data[1] = s;
  outmatrix.data[4] = -s;
  outmatrix.data[5] = c;
  return outmatrix;
}
static inline Mat4 mat4EulerXYZ(f32 xRadians, f32 yRadians, f32 zRadians) {
  Mat4 rx = mat4EulerX(xRadians);
  Mat4 ry = mat4EulerY(yRadians);
  Mat4 rz = mat4EulerZ(zRadians);
  Mat4 outmatrix = mat4Mul(rx, ry);
  outmatrix = mat4Mul(outmatrix, rz);
  return outmatrix;
}

static inline Vec3 mat4Forward(Mat4 matrix) {
  Vec3 forward;
  forward.x = matrix.data[2];
  forward.y = matrix.data[6];
  forward.z = matrix.data[10];
  vec3Normalize(&forward);
  return forward;
}

static inline Vec3 mat4Backward(Mat4 matrix) {
  Vec3 backward;
  backward.x = -matrix.data[2];
  backward.y = -matrix.data[6];
  backward.z = -matrix.data[10];
  vec3Normalize(&backward);
  return backward;
}

static inline Vec3 mat4Up(Mat4 matrix) {
  Vec3 up;
  up.x = matrix.data[1];
  up.y = matrix.data[5];
  up.z = matrix.data[9];
  vec3Normalize(&up);
  return up;
}

static inline Vec3 mat4Down(Mat4 matrix) {
  Vec3 down;
  down.x = -matrix.data[1];
  down.y = -matrix.data[5];
  down.z = -matrix.data[9];
  vec3Normalize(&down);
  return down;
}

static inline Vec3 mat4Left(Mat4 matrix) {
  Vec3 left;
  left.x = matrix.data[0];
  left.y = matrix.data[4];
  left.z = matrix.data[8];
  vec3Normalize(&left);
  return left;
}

static inline Vec3 mat4Right(Mat4 matrix) {
  Vec3 right;
  right.x = -matrix.data[0];
  right.y = -matrix.data[4];
  right.z = -matrix.data[8];
  vec3Normalize(&right);
  return right;
}

// ------------------------------------------
// Quaternion
// ------------------------------------------

static inline Quat quatIdentity() {
  return (Quat){0, 0, 0, 1.0f};
}

static inline f32 quatNormal(Quat q) {
  return ksqrt(
    q.x * q.x +
    q.y * q.y +
    q.z * q.z +
    q.w * q.w);
}

static inline Quat quatNormalize(Quat q) {
  f32 normal = quatNormal(q);
  return (Quat){
    q.x / normal,
    q.y / normal,
    q.z / normal,
    q.w / normal};
}

static inline Quat quatConjugate(Quat q) {
  return (Quat){
    -q.x,
    -q.y,
    -q.z,
    q.w};
}

static inline Quat quatInverse(Quat q) {
  return quatNormalize(quatConjugate(q));
}

static inline Quat quatMul(Quat q0, Quat q1) {
  Quat outquaternion;

  outquaternion.x = q0.x * q1.w +
    q0.y * q1.z -
    q0.z * q1.y +
    q0.w * q1.x;

  outquaternion.y = -q0.x * q1.z +
    q0.y * q1.w +
    q0.z * q1.x +
    q0.w * q1.y;

  outquaternion.z = q0.x * q1.y -
    q0.y * q1.x +
    q0.z * q1.w +
    q0.w * q1.z;

  outquaternion.w = -q0.x * q1.x -
    q0.y * q1.y -
    q0.z * q1.z +
    q0.w * q1.w;

  return outquaternion;
}

static inline f32 quatDot(Quat q0, Quat q1) {
  return q0.x * q1.x +
  q0.y * q1.y +
  q0.z * q1.z +
  q0.w * q1.w;
}

static inline Mat4 quatToMat4(Quat q) {
  Mat4 outmatrix = mat4Identity();

  // https://stackoverflow.com/questions/1556260/convert-Quaternion-rotation-to-rotation-matrix

  Quat n = quatNormalize(q);

  outmatrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
  outmatrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
  outmatrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

  outmatrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
  outmatrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
  outmatrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

  outmatrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
  outmatrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
  outmatrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

  return outmatrix;
}

// Calculates a rotation matrix based on the Quaternion and the passed in center point.
static inline Mat4 quatToRotationMatrix(Quat q, Vec3 center) {
  Mat4 outmatrix;

  f32* o = outmatrix.data;
  o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
  o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
  o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

  o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
  o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
  o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

  o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
  o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
  o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
  o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

  o[12] = 0.0f;
  o[13] = 0.0f;
  o[14] = 0.0f;
  o[15] = 1.0f;
  return outmatrix;
}

static inline Quat quatFromAxisAngle(Vec3 axis, f32 angle, u8 normalize) {
  const f32 halfangle = 0.5f * angle;
  f32 s = ksin(halfangle);
  f32 c = kcos(halfangle);

  Quat q = (Quat){s * axis.x, s * axis.y, s * axis.z, c};
  if (normalize) {
    return quatNormalize(q);
  }
  return q;
}

static inline Quat quatSlerp(Quat q0, Quat q1, f32 percentage) {
  Quat outQuaternion;
  // Source: https://en.wikipedia.org/wiki/Slerp
  // Only unit Quaternions are valid rotations.
  // Normalize to avoid undefined behavior.
  Quat v0 = quatNormalize(q0);
  Quat v1 = quatNormalize(q1);

  // Compute the cosine of the angle between the two vectors.
  f32 dot = quatDot(v0, v1);

  // If the dot product is negative, slerp won't take
  // the shorter path. Note that v1 and -v1 are equivalent when
  // the negation is applied to all four components. Fix by
  // reversing one Quaternion.
  if (dot < 0.0f) {
    v1.x = -v1.x;
    v1.y = -v1.y;
    v1.z = -v1.z;
    v1.w = -v1.w;
    dot = -dot;
  }

  const f32 DOT_THRESHOLD = 0.9995f;
  if (dot > DOT_THRESHOLD) {
    // If the inputs are too close for comfort, linearly interpolate
    // and normalize the result.
    outQuaternion = (Quat){
      v0.x + ((v1.x - v0.x) * percentage),
      v0.y + ((v1.y - v0.y) * percentage),
      v0.z + ((v1.z - v0.z) * percentage),
      v0.w + ((v1.w - v0.w) * percentage)};

    return quatNormalize(outQuaternion);
  }

  // Since dot is in range [0, DOTTHRESHOLD], acos is safe
  f32 theta0 = kacos(dot);          // theta0 = angle between input vectors
  f32 theta = theta0 * percentage;  // theta = angle between v0 and result
  f32 sinTheta = ksin(theta);       // compute this value only once
  f32 sinTheta0 = ksin(theta0);   // compute this value only once

  f32 s0 = kcos(theta) - dot * sinTheta / sinTheta0;  // == sin(theta - theta) / sin(theta0)
  f32 s1 = sinTheta / sinTheta0;

  return (Quat){
    (v0.x * s0) + (v1.x * s1),
    (v0.y * s0) + (v1.y * s1),
    (v0.z * s0) + (v1.z * s1),
    (v0.w * s0) + (v1.w * s1)};
}

static inline f32 degToRad(f32 degrees) {
  return degrees * K_DEG2RAD_MULTIPLIER;
}

static inline f32 radToDeg(f32 radians) {
  return radians * K_RAD2DEG_MULTIPLIER;
}
