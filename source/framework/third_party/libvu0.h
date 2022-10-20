
/***************************************************************
/  la lib 0.1 (snapshot 2)                                     /
/  Copyright (C) 2001 - Sony Computer Entertainment Inc.       /
/                                                              /
/  header:   libVu0 (libvu0.h)                                 /
/  author:   Sony Computer Entertainment Inc.                  /
/  last mod:                                                   /
/  license:  GNU Library General Public License Version 2      /
/            (see LICENSE on the main directory)               /
***************************************************************/

//
// Modified by Lampert on 2015-01-17.
//

#ifndef LIBVU0_H
#define LIBVU0_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef VU_ALIGNED
#define VU_ALIGNED __attribute__((aligned(16)))
#endif // VU_ALIGNED

// ========================================================
// Vector Unit (VU) Types:
// ========================================================

// 4D floating-point vector:
typedef float vuFVector[4] VU_ALIGNED;

// 4D 32bit integer vector:
typedef int vuIVector[4] VU_ALIGNED;

// 4x4 homogeneous matrix:
typedef float vuMat4x4[4][4] VU_ALIGNED;

// ========================================================
// Helper constants:
// ========================================================

typedef enum {
	vuSpotLight     = 1,
	vuParallelLight = 2
} vuLightSrcType;

typedef enum {
	vuClipUnderX = 1,
	vuClipUnderY = 2,
	vuClipUnderW = 4,
	vuClipOverX  = 8,
	vuClipOverY  = 16,
	vuClipOverW  = 32
} vuClip;

// ========================================================
// Vector ops:
// ========================================================

void  vuCopyVector(vuFVector v0, const vuFVector v1);
void  vuCopyVectorXYZ(vuFVector v0, const vuFVector v1);
void  vuFToI0Vector(vuIVector v0, const vuFVector v1);
void  vuFToI4Vector(vuIVector v0, const vuFVector v1);
void  vuIToF0Vector(vuFVector v0, const vuIVector v1);
void  vuIToF4Vector(vuFVector v0, const vuIVector v1);
void  vuScaleVector(vuFVector v0, const vuFVector v1, float s);
void  vuScaleVectorXYZ(vuFVector v0, const vuFVector v1, float s);
void  vuAddVector(vuFVector v0, const vuFVector v1, const vuFVector v2);
void  vuSubVector(vuFVector v0, const vuFVector v1, const vuFVector v2);
void  vuMulVector(vuFVector v0, const vuFVector v1, const vuFVector v2);
void  vuDivVector(vuFVector v0, const vuFVector v1, float q);
void  vuDivVectorXYZ(vuFVector v0, const vuFVector v1, float q);
void  vuInterVector(vuFVector v0, const vuFVector v1, const vuFVector v2, float r);
void  vuInterVectorXYZ(vuFVector v0, const vuFVector v1, const vuFVector v2, float r);
float vuInnerProduct(const vuFVector v0, const vuFVector v1);
void  vuOuterProduct(vuFVector v0, const vuFVector v1, const vuFVector v2);
void  vuNormalizeVector(vuFVector v0, const vuFVector v1);
void  vuClampVector(vuFVector v0, const vuFVector v1, float min, float max);

// ========================================================
// Matrix ops:
// ========================================================

void vuInverseMatrix(vuMat4x4 m0, vuMat4x4 m1);
void vuApplyMatrix(vuFVector v0, vuMat4x4 m, const vuFVector v1);
void vuUnitMatrix(vuMat4x4 m);
void vuCopyMatrix(vuMat4x4 m0, vuMat4x4 m1);
void vuTransposeMatrix(vuMat4x4 m0, vuMat4x4 m1);
void vuMulMatrix(vuMat4x4 m0, vuMat4x4 m1, vuMat4x4 m2);

void vuRotMatrix(vuMat4x4 m0, vuMat4x4 m1, const vuFVector rot);
void vuRotMatrixX(vuMat4x4 m0, vuMat4x4 m1, float rx);
void vuRotMatrixY(vuMat4x4 m0, vuMat4x4 m1, float ry);
void vuRotMatrixZ(vuMat4x4 m0, vuMat4x4 m1, float rz);

void vuTransMatrix(vuMat4x4 m0, vuMat4x4 m1, const vuFVector tv);
void vuRotTransPers(vuIVector v0, vuMat4x4 m0, const vuFVector v1, int mode);
void vuRotTransPersN(vuIVector * v0, vuMat4x4 m0, const vuFVector * v1, int n, int mode);

void vuCameraMatrix(vuMat4x4 m, const vuFVector p, const vuFVector zd, const vuFVector yd);
void vuViewScreenMatrix(vuMat4x4 m, float scrz, float ax, float ay, float cx, float cy, float zmin, float zmax, float nearz, float farz);

void vuNormalLightMatrix(vuMat4x4 m, const vuFVector l0, const vuFVector l1, const vuFVector l2);
void vuLightColorMatrix(vuMat4x4 m, const vuFVector c0, const vuFVector c1, const vuFVector c2, const vuFVector aa);
void vuDropShadowMatrix(vuMat4x4 m, const vuFVector lp, float a, float b, float c, vuLightSrcType type);

int  vuClipAll(vuFVector minv, vuFVector maxv, vuMat4x4 ms, vuFVector * vm, int n);
int  vuClipScreen(vuFVector v0);
int  vuClipScreen3(vuFVector v0, vuFVector v1, vuFVector v2);

#ifdef __cplusplus
} // extern C
#endif // __cplusplus

#endif // LIBVU0_H
