
//
// License:
//  This source code is released under the MIT License.
//  Copyright (c) 2015 Guilherme R. Lampert.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

// ========================================================
//
// Vertex transformation routines. This is not meant to be
// used as a header file but as a raw textual include.
//
// ========================================================

// Scale constants use to map a vertex to GS rasterizer space:
static const float GS_RASTER_SCALE_X = 2048.0f;
static const float GS_RASTER_SCALE_Y = 2048.0f;
static const float GS_RASTER_SCALE_Z = float(0xFFFFFF) / 32.0f;

// Expand scale factors into a vector (W ignored):
static const float V_GS_SCALE[4] ATTRIBUTE_ALIGNED(16) =
{
	GS_RASTER_SCALE_X,
	GS_RASTER_SCALE_Y,
	GS_RASTER_SCALE_Z,
	1.0f
};

// ========================================================

static inline void ftoi4XYZ(const Vector * vIn, int * vOut)
{
	// To integer fixed point. Format used for vertex positions.
	asm volatile (
		"lqc2       vf4, 0x0(%1) \n\t"
		"vftoi4.xyz vf5, vf4     \n\t"
		"sqc2       vf5, 0x0(%0) \n\t"
		: : "r" (vOut), "r" (vIn)
	);
}

// ========================================================

static inline void ftoi0XYZW(const Vector * vIn, int * vOut)
{
	// Color val to integer fixed point. Used for colors only.
	// Multiply XYZ by 128 and convert to integer FP.
	const float q = 128.0f;
	asm volatile (
		"lqc2        vf4, 0x0(%1)  \n\t"
		"mfc1        $8,  %2       \n\t"
		"qmtc2       $8,  vf6      \n\t"
		"vmulx.xyz   vf4, vf4, vf6 \n\t"
		"vftoi0.xyzw vf5, vf4      \n\t"
		"sqc2        vf5, 0x0(%0)  \n\t"
		: : "r" (vOut), "r" (vIn), "f" (q)
		: "$8"
	);
}

// ========================================================

static inline void setTexcColor(DrawVertex & dv, const Vector & texCoord, const Vector & color)
{
	// Sets the `texCoord` and `color` fields of a DrawVertex with 4 VU instructions.
	asm volatile (
		"lqc2 vf5, 0x00(%1) \n\t" // vf5 = texCoord
		"lqc2 vf6, 0x00(%2) \n\t" // vf6 = color
		"sqc2 vf6, 0x20(%0) \n\t" // dv.color = vf6
		"sqc2 vf5, 0x10(%0) \n\t" // dv.texCoord = vf5
		: : "r" (&dv), "r" (&texCoord), "r" (&color)
	);
}

// ========================================================

static inline void scaleVert(Vector * v, float q)
{
	// Multiply `v` by `q` and scale XYZ using the GS scale factors.
	asm volatile (
		"lqc2      vf4, 0x0(%0)  \n\t"
		"lqc2      vf5, 0x0(%1)  \n\t"
		"mfc1      $8,  %2       \n\t"
		"qmtc2     $8,  vf6      \n\t"
		"vmulx.xyz vf4, vf4, vf6 \n\t"
		"vmul.xyz  vf4, vf4, vf5 \n\t"
		"vadd.xyz  vf4, vf4, vf5 \n\t"
		"sqc2 vf4, 0x0(%0)       \n\t"
		: : "r" (v), "r" (V_GS_SCALE), "f" (q)
		: "$8"
	);
}

// ========================================================

static inline void applyXForm(Vector * vOut, const Matrix * m, const Vector * vIn)
{
	// Multiply Vector with Matrix (apply transform).
	// This was salvaged from libVu0.c
	asm volatile (
		"lqc2         vf4, 0x0(%1)  \n\t"
		"lqc2         vf5, 0x10(%1) \n\t"
		"lqc2         vf6, 0x20(%1) \n\t"
		"lqc2         vf7, 0x30(%1) \n\t"
		"lqc2         vf8, 0x0(%2)  \n\t"
		"vmulax.xyzw  ACC, vf4, vf8 \n\t"
		"vmadday.xyzw ACC, vf5, vf8 \n\t"
		"vmaddaz.xyzw ACC, vf6, vf8 \n\t"
		"vmaddw.xyzw  vf9, vf7, vf8 \n\t"
		"sqc2         vf9, 0x0(%0)  \n\t"
		: : "r" (vOut), "r" (m), "r" (vIn)
	);
}

// ========================================================

static inline void inverseMatrix(Matrix * mOut, const Matrix * mIn)
{
	// Inverse of input matrix.
	// This was salvaged from libVu0.c
	asm volatile (
		"lq          $8,  0x00(%1) \n\t"
		"lq          $9,  0x10(%1) \n\t"
		"lq          $10, 0x20(%1) \n\t"
		"lqc2        vf4, 0x30(%1) \n\t"
		"vmove.xyzw  vf5, vf4      \n\t"
		"vsub.xyz    vf4, vf4, vf4 \n\t"
		"vmove.xyzw  vf9, vf4      \n\t"
		"qmfc2       $11, vf4      \n\t"
		"pextlw      $12, $9,  $8  \n\t"
		"pextuw      $13, $9,  $8  \n\t"
		"pextlw      $14, $11, $10 \n\t"
		"pextuw      $15, $11, $10 \n\t"
		"pcpyld      $8,  $14, $12 \n\t"
		"pcpyud      $9,  $12, $14 \n\t"
		"pcpyld      $10, $15, $13 \n\t"
		"qmtc2       $8,  vf6      \n\t"
		"qmtc2       $9,  vf7      \n\t"
		"qmtc2       $10, vf8      \n\t"
		"vmulax.xyz  ACC, vf6, vf5 \n\t"
		"vmadday.xyz ACC, vf7, vf5 \n\t"
		"vmaddz.xyz  vf4, vf8, vf5 \n\t"
		"vsub.xyz    vf4, vf9, vf4 \n\t"
		"sq          $8,  0x00(%0) \n\t"
		"sq          $9,  0x10(%0) \n\t"
		"sq          $10, 0x20(%0) \n\t"
		"sqc2        vf4, 0x30(%0) \n\t"
		: : "r" (mOut), "r" (mIn)
		: "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15"
	);
}

// ========================================================

static inline bool clipTriangle(const Vector * v0, const Vector * v1, const Vector * v2)
{
	// This was salvaged from libVu0.c
	register int ret;
	asm volatile (
		"vsub.xyzw vf04, vf00, vf00 \n\t"
		".set push                  \n\t"
		".set mips3                 \n\t"
		"li %0, 0x4580000045800000  \n\t"
		".set pop                   \n\t"
		"lqc2     vf06, 0x0(%1)     \n\t"
		"lqc2     vf08, 0x0(%2)     \n\t"
		"lqc2     vf09, 0x0(%3)     \n\t"
		"qmtc2    %0,   vf07        \n\t"
		"ctc2     $0,   $vi16       \n\t"
		"vsub.xyw vf05, vf06, vf04  \n\t"
		"vsub.xy  vf05, vf07, vf06  \n\t"
		"vsub.xyw vf05, vf08, vf04  \n\t"
		"vsub.xy  vf05, vf07, vf08  \n\t"
		"vsub.xyw vf05, vf09, vf04  \n\t"
		"vsub.xy  vf05, vf07, vf09  \n\t"
		"vnop                       \n\t"
		"vnop                       \n\t"
		"vnop                       \n\t"
		"vnop                       \n\t"
		"vnop                       \n\t"
		"cfc2 %0, $vi16             \n\t"
		"andi %0, %0, 0xC0          \n\t"
		: "=r" (ret)
		: "r"  (v0), "r" (v1), "r" (v2)
	);
	return ret != 0;
}

// ========================================================

static inline bool cullBackFacingTriangle(const Vector * eye, const Vector * v0,
                                          const Vector * v1,  const Vector * v2)
{
	// Plane equation of the triangle will give us its orientation
	// which can be compared with the viewer's world position.
	//
	// const Vector d = crossProduct(*v2 - *v0, *v1 - *v0);
	// const Vector c = *v0 - *eye;
	// return dotProduct3(c, d) <= 0.0f;
	//
	register float dot;
	asm volatile (
		"lqc2        vf4, 0x0(%1)  \n\t" // vf4 = eye
		"lqc2        vf5, 0x0(%2)  \n\t" // vf5 = v0
		"lqc2        vf6, 0x0(%3)  \n\t" // vf6 = v1
		"lqc2        vf7, 0x0(%4)  \n\t" // vf7 = v2
		"vsub.xyz    vf8, vf7, vf5 \n\t" // vf8 = vf7(v2) - vf5(v0)
		"vsub.xyz    vf9, vf6, vf5 \n\t" // vf9 = vf6(v1) - vf5(v0)
		"vopmula.xyz ACC, vf8, vf9 \n\t" // vf6 = cross(vf8, vf9)
		"vopmsub.xyz vf6, vf9, vf8 \n\t"
		"vsub.w      vf6, vf6, vf6 \n\t"
		"vsub.xyz    vf7, vf5, vf4 \n\t" // vf7 = vf5(v0) - vf4(eye)
		"vmul.xyz    vf5, vf7, vf6 \n\t" // vf5 = dot(vf7, vf6)
		"vaddy.x     vf5, vf5, vf5 \n\t"
		"vaddz.x     vf5, vf5, vf5 \n\t"
		"qmfc2       $2,  vf5      \n\t" // store result on `dot` variable
		"mtc1        $2,  %0       \n\t"
		: "=f" (dot)
		: "r" (eye), "r" (v0), "r" (v1), "r" (v2)
		: "$2"
	);
	return dot <= 0.0f;
}

// ========================================================

static inline void emitVert(uint64 * restrict & packetPtr, Vector & tPos, const float q, const DrawVertex & dv)
{
	// Convert vertex position to fixed-point:
	int tPosFixed[4] ATTRIBUTE_ALIGNED(16);
	ftoi4XYZ(&tPos, tPosFixed);

	// Transform color to GS format and convert to fixed-point:
	int tColorFixed[4] ATTRIBUTE_ALIGNED(16);
	ftoi0XYZW(&dv.color, tColorFixed);

	// Store fixed-point color:
	color_t gsColor;
	gsColor.r = scast<ubyte>(tColorFixed[0]);
	gsColor.g = scast<ubyte>(tColorFixed[1]);
	gsColor.b = scast<ubyte>(tColorFixed[2]);
	gsColor.a = scast<ubyte>(tColorFixed[3]);
	gsColor.q = q;

	// Store texture coords unfixed but with perspective scale:
	texel_t gsTexel;
	gsTexel.u = dv.texCoord.x * q;
	gsTexel.v = dv.texCoord.y * q;

	// Store fixed-point position:
	xyz_t gsPos;
	gsPos.x = scast<u16>(tPosFixed[0]);
	gsPos.y = scast<u16>(tPosFixed[1]);
	gsPos.z = scast<u32>(tPosFixed[2]);

	// Add to packet:
	*(packetPtr)++ = gsColor.rgbaq;
	*(packetPtr)++ = gsTexel.uv;
	*(packetPtr)++ = gsPos.xyz;
}

// ========================================================

static inline void emitLine(qword_t * restrict & packetPtr, const Matrix & mvpMatrix,
                            const Vector & from, const Vector & to, const Color4f & color)
{
	float   qFrom;
	float   qTo;
	Vector  tPosFrom;
	Vector  tPosTo;
	xyz_t   gsPosFrom;
	xyz_t   gsPosTo;
	Vector  tColor;
	color_t gsColor;
	int     tPosFixed[4]   ATTRIBUTE_ALIGNED(16);
	int     tColorFixed[4] ATTRIBUTE_ALIGNED(16);

	// Transform the two vertexes:
	applyXForm(&tPosFrom, &mvpMatrix, &from);
	applyXForm(&tPosTo,   &mvpMatrix, &to);

	// Perspective divide and scale:
	qFrom = 1.0f / tPosFrom.w;
	qTo   = 1.0f / tPosTo.w;
	scaleVert(&tPosFrom, qFrom);
	scaleVert(&tPosTo,   qTo);

	// HACK: I'm getting lazy here and reusing the triangle clipping function.
	// Works well enough for our purposes. We don't need efficiency for debug line rendering.
	if (clipTriangle(&tPosFrom, &tPosTo, &tPosTo))
	{
		return;
	}

	// Vertex 0 to fixed point:
	ftoi4XYZ(&tPosFrom, tPosFixed);
	gsPosFrom.x = scast<u16>(tPosFixed[0]);
	gsPosFrom.y = scast<u16>(tPosFixed[1]);
	gsPosFrom.z = scast<u32>(tPosFixed[2]);

	// Vertex 1 to fixed point:
	ftoi4XYZ(&tPosTo, tPosFixed);
	gsPosTo.x = scast<u16>(tPosFixed[0]);
	gsPosTo.y = scast<u16>(tPosFixed[1]);
	gsPosTo.z = scast<u32>(tPosFixed[2]);

	// Transform color to GS format and convert to fixed-point:
	tColor.x = color.r;
	tColor.y = color.g;
	tColor.z = color.b;
	tColor.w = color.a;
	ftoi0XYZW(&tColor, tColorFixed);

	// Store fixed-point color:
	gsColor.r = scast<ubyte>(tColorFixed[0]);
	gsColor.g = scast<ubyte>(tColorFixed[1]);
	gsColor.b = scast<ubyte>(tColorFixed[2]);
	gsColor.a = scast<ubyte>(tColorFixed[3]);
	gsColor.q = 1.0f; // Unimportant in this case. Set to 1.

	// "Draw" the line (add it to a render packet):
	packetPtr->dw[0] = gsColor.rgbaq;
	packetPtr->dw[1] = gsPosFrom.xyz;
	++packetPtr;
	packetPtr->dw[0] = gsColor.rgbaq;
	packetPtr->dw[1] = gsPosTo.xyz;
	++packetPtr;
}

// ========================================================

#define DRAW3D_PROLOGUE() \
	BEGIN_DMA_TAG(currentFrameQwPtr); \
	if (currentTex != nullptr) \
	{ \
		setTextureBufferSampling(); \
	} \
	uint64 * restrict packetPtr = rcast<uint64 *>(draw_prim_start(currentFrameQwPtr, 0, &primDesc, &primColor))

// ========================================================

#define DRAW3D_EPILOGUE() \
	/* Check if we're in middle of a qword and pad it if needed: */ \
	while (rcast<u32>(packetPtr) % 16) \
	{ \
		*packetPtr++ = 0u; \
	} \
	currentFrameQwPtr = draw_prim_end(rcast<qword_t *>(packetPtr), 3, DRAW_STQ_REGLIST); \
	END_DMA_TAG(currentFrameQwPtr); \
	drawCount3d++

// ========================================================

#ifndef NO_BACK_FACE_CULLING

#define TRIANGLE_BACK_FACE_CULL(v0, v1, v2) \
	if (cullBackFacingTriangle(&eyePosModelSpace, &(v0), &(v1), &(v2))) \
	{ \
		continue; \
	}

#else // NO_BACK_FACE_CULLING defined

#define TRIANGLE_BACK_FACE_CULL(v0, v1, v2) /* No-op */

#endif // NO_BACK_FACE_CULLING

// ========================================================

#ifndef NO_TRIANGLE_CLIPPING

#define TRIANGLE_XFORM_SCR_CLIP(v0, v1, v2)  \
	applyXForm(&tPos0, &mvpMatrix, &(v0)); \
	applyXForm(&tPos1, &mvpMatrix, &(v1)); \
	applyXForm(&tPos2, &mvpMatrix, &(v2)); \
	const float q0 = 1.0f / tPos0.w; \
	scaleVert(&tPos0, q0); \
	const float q1 = 1.0f / tPos1.w; \
	scaleVert(&tPos1, q1); \
	const float q2 = 1.0f / tPos2.w; \
	scaleVert(&tPos2, q2); \
	if (clipTriangle(&tPos0, &tPos1, &tPos2)) \
	{ \
		continue; \
	}

#else // NO_TRIANGLE_CLIPPING defined

#define TRIANGLE_XFORM_SCR_CLIP(v0, v1, v2)  \
	applyXForm(&tPos0, &mvpMatrix, &(v0)); \
	applyXForm(&tPos1, &mvpMatrix, &(v1)); \
	applyXForm(&tPos2, &mvpMatrix, &(v2)); \
	const float q0 = 1.0f / tPos0.w; \
	scaleVert(&tPos0, q0); \
	const float q1 = 1.0f / tPos1.w; \
	scaleVert(&tPos1, q1); \
	const float q2 = 1.0f / tPos2.w; \
	scaleVert(&tPos2, q2); \
	/* No clipping after the transform when disabled! */

#endif // NO_TRIANGLE_CLIPPING

// ========================================================

#define XFORM_TRIANGLE(v0, v1, v2) \
	Vector tPos0, tPos1, tPos2; \
	TRIANGLE_BACK_FACE_CULL((v0).position, (v1).position, (v2).position); \
	TRIANGLE_XFORM_SCR_CLIP((v0).position, (v1).position, (v2).position); \
	emitVert(packetPtr, tPos0, q0, (v0)); \
	emitVert(packetPtr, tPos1, q1, (v1)); \
	emitVert(packetPtr, tPos2, q2, (v2));

// ========================================================

#define XFORM_TRIANGLE_NO_BF_CULL(v0, v1, v2) \
	Vector tPos0, tPos1, tPos2; \
	TRIANGLE_XFORM_SCR_CLIP((v0).position, (v1).position, (v2).position); \
	emitVert(packetPtr, tPos0, q0, (v0)); \
	emitVert(packetPtr, tPos1, q1, (v1)); \
	emitVert(packetPtr, tPos2, q2, (v2));

// ========================================================
