#ifndef PF_COMMON_H
#define PF_COMMON_H

#include "pixelforge.h"

/* Base functions */

PFctx* PF_Init(void* pixels, PFuint width, PFuint height);
void PF_Reshape(PFuint width, PFuint height);

/* Draw 2D */

void PF_DrawRectangle(PFfloat x, PFfloat y, PFfloat largeur, PFfloat hauteur);
void PF_DrawTexture(PFtexture* texture, PFfloat x, PFfloat y, PFfloat width, PFfloat height);

/* Draw 3D */

void PF_Begin3D(PFuint width, PFuint height, PFdouble fovy);
void PF_End3D();

void PF_Update3D(PFfloat px, PFfloat py, PFfloat pz, PFfloat tx, PFfloat ty, PFfloat tz);

void PF_DrawCube(PFfloat size);
void PF_DrawCubeLines(PFfloat size);

void PF_DrawGrid(PFint slices, PFfloat spacing);

/* Definitions */

#ifdef PF_COMMON_IMPL

#include <math.h>

PFctx* PF_Init(void* pixels, PFuint width, PFuint height)
{
    PFctx *ctx = pfContextCreate(pixels, width, height, PF_PIXELFORMAT_R8G8B8A8);
    pfMakeCurrent(ctx);
    return ctx;
}

void PF_Reshape(PFuint width, PFuint height)
{
    pfViewport(0, 0, width, height);
    pfMatrixMode(PF_PROJECTION);
    pfLoadIdentity();
    pfOrtho(0.0, width, height, 0.0, 0.0, 1.0);    // Config with top-left corner as origin
    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();
}

/* Draw 2D */

void PF_DrawRectangle(PFfloat x, PFfloat y, PFfloat width, PFfloat height)
{
    pfBegin(PF_QUADS);

    pfVertex2f(x, y);
    pfVertex2f(x, y + height);
    pfVertex2f(x + width, y + height);
    pfVertex2f(x + width, y);

    pfEnd();
}

void PF_DrawTexture(PFtexture* texture, PFfloat x, PFfloat y, PFfloat width, PFfloat height)
{
    pfEnableTexture(texture);
    pfBegin(PF_QUADS);

    pfTexCoord2f(0.0f, 0.0f); pfVertex2f(x, y);
    pfTexCoord2f(0.0f, 1.0f); pfVertex2f(x, y + height);
    pfTexCoord2f(1.0f, 1.0f); pfVertex2f(x + width, y + height);
    pfTexCoord2f(1.0f, 0.0f); pfVertex2f(x + width, y);

    pfEnd();
    pfDisableTexture();
}

void PF_DrawTextureEx(PFtexture* texture, PFfloat x, PFfloat y, PFfloat width, PFfloat height, PFfloat ox, PFfloat oy, PFfloat angleDegrees)
{
    PFfloat angleRadians = DEG2RAD(angleDegrees);
    PFfloat c = cos(angleRadians);
    PFfloat s = sin(angleRadians);

    PFfloat hWidth = width * 0.5f;
    PFfloat hHeight = height * 0.5f;

    PFfloat x1 = -hWidth;
    PFfloat y1 = -hHeight;
    PFfloat x2 = -hWidth;
    PFfloat y2 = hHeight;
    PFfloat x3 = hWidth;
    PFfloat y3 = hHeight;
    PFfloat x4 = hWidth;
    PFfloat y4 = -hHeight;

    PFfloat rx1 = (x1 * c - y1 * s) + hWidth;
    PFfloat ry1 = (x1 * s + y1 * c) + hHeight;
    PFfloat rx2 = (x2 * c - y2 * s) + hWidth;
    PFfloat ry2 = (x2 * s + y2 * c) + hHeight;
    PFfloat rx3 = (x3 * c - y3 * s) + hWidth;
    PFfloat ry3 = (x3 * s + y3 * c) + hHeight;
    PFfloat rx4 = (x4 * c - y4 * s) + hWidth;
    PFfloat ry4 = (x4 * s + y4 * c) + hHeight;

    pfEnableTexture(texture);
    pfBegin(PF_QUADS);

    pfTexCoord2f(0.0f, 0.0f); pfVertex2f(x - ox + rx1, y - oy + ry1);
    pfTexCoord2f(0.0f, 1.0f); pfVertex2f(x - ox + rx2, y - oy + ry2);
    pfTexCoord2f(1.0f, 1.0f); pfVertex2f(x - ox + rx3, y - oy + ry3);
    pfTexCoord2f(1.0f, 0.0f); pfVertex2f(x - ox + rx4, y - oy + ry4);

    pfEnd();
    pfDisableTexture();
}


/* Draw 3D */

void PF_Begin3D(PFuint width, PFuint height, PFdouble fovy)
{
    pfMatrixMode(PF_PROJECTION);
    pfPushMatrix();
    pfLoadIdentity();

    PFdouble aspect = (PFdouble)width/(PFdouble)height;
    PFdouble top = 0.01*tan(DEG2RAD(fovy*0.5));
    PFdouble right = top*aspect;

    pfFrustum(-right, right, -top, top, 0.01, 1000.0);

    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();

    pfEnableDepthTest();
}

void PF_End3D()
{
    pfMatrixMode(PF_PROJECTION);    // Switch to projection matrix
    pfPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    pfMatrixMode(PF_MODELVIEW);     // Switch back to modelview matrix
    pfLoadIdentity();               // Reset current matrix (modelview)

    pfDisableDepthTest();           // Disable DEPTH_TEST for 2D
}

void PF_Update3D(PFfloat px, PFfloat py, PFfloat pz, PFfloat tx, PFfloat ty, PFfloat tz)
{
    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();

    PFvec3f position = { px, py, pz };
    PFvec3f target = { tx, ty, tz };
    PFvec3f up = { 0, 1, 0 };

    PFmat4f matView = pfMat4fLookAt(&position, &target, &up);
    pfMultMatrixMat4f(&matView);
}

void PF_DrawCube(PFfloat size)
{
    PFfloat halfSize = size*0.5f;

    pfBegin(PF_QUADS);

    // Front face
    pfColor3f(1.0f, 0.0f, 0.0f);
    pfVertex3f(-halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, halfSize, halfSize);
    pfVertex3f(-halfSize, halfSize, halfSize);

    // Back side
    pfColor3f(1.0f, 0.0f, 0.0f);
    pfVertex3f(halfSize, -halfSize, -halfSize);
    pfVertex3f(-halfSize, -halfSize, -halfSize);
    pfVertex3f(-halfSize, halfSize, -halfSize);
    pfVertex3f(halfSize, halfSize, -halfSize);

    // Left face
    pfColor3f(0.0f, 1.0f, 0.0f);
    pfVertex3f(-halfSize, -halfSize, -halfSize);
    pfVertex3f(-halfSize, -halfSize, halfSize);
    pfVertex3f(-halfSize, halfSize, halfSize);
    pfVertex3f(-halfSize, halfSize, -halfSize);

    // Right face
    pfColor3f(0.0f, 1.0f, 0.0f);
    pfVertex3f(halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, -halfSize, -halfSize);
    pfVertex3f(halfSize, halfSize, -halfSize);
    pfVertex3f(halfSize, halfSize, halfSize);

    // Upper side
    pfColor3f(0.0f, 0.0f, 1.0f);
    pfVertex3f(-halfSize, halfSize, halfSize);
    pfVertex3f(halfSize, halfSize, halfSize);
    pfVertex3f(halfSize, halfSize, -halfSize);
    pfVertex3f(-halfSize, halfSize, -halfSize);

    // Lower side
    pfColor3f(0.0f, 0.0f, 1.0f);
    pfVertex3f(halfSize, -halfSize, halfSize);
    pfVertex3f(-halfSize, -halfSize, halfSize);
    pfVertex3f(-halfSize, -halfSize, -halfSize);
    pfVertex3f(halfSize, -halfSize, -halfSize);

    pfEnd();
}

void PF_DrawCubeLines(PFfloat size)
{
    PFfloat halfSize = size*0.5f;

    pfBegin(PF_LINES);

    // Front panel edges
    pfVertex3f(-halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, -halfSize, halfSize);

    pfVertex3f(halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, halfSize, halfSize);

    pfVertex3f(halfSize, halfSize, halfSize);
    pfVertex3f(-halfSize, halfSize, halfSize);

    pfVertex3f(-halfSize, halfSize, halfSize);
    pfVertex3f(-halfSize, -halfSize, halfSize);

    // Back side edges
    pfVertex3f(-halfSize, -halfSize, -halfSize);
    pfVertex3f(halfSize, -halfSize, -halfSize);

    pfVertex3f(halfSize, -halfSize, -halfSize);
    pfVertex3f(halfSize, halfSize, -halfSize);

    pfVertex3f(halfSize, halfSize, -halfSize);
    pfVertex3f(-halfSize, halfSize, -halfSize);

    pfVertex3f(-halfSize, halfSize, -halfSize);
    pfVertex3f(-halfSize, -halfSize, -halfSize);

    // Edges between front and back faces
    pfVertex3f(-halfSize, -halfSize, halfSize);
    pfVertex3f(-halfSize, -halfSize, -halfSize);

    pfVertex3f(halfSize, -halfSize, halfSize);
    pfVertex3f(halfSize, -halfSize, -halfSize);

    pfVertex3f(halfSize, halfSize, halfSize);
    pfVertex3f(halfSize, halfSize, -halfSize);

    pfVertex3f(-halfSize, halfSize, halfSize);
    pfVertex3f(-halfSize, halfSize, -halfSize);

    pfEnd();
}

void PF_DrawGrid(PFint slices, PFfloat spacing)
{
    PFint halfSlices = slices/2;

    pfBegin(PF_LINES);
        for (PFint i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                pfColor3f(0.5f, 0.5f, 0.5f);
                pfColor3f(0.5f, 0.5f, 0.5f);
                pfColor3f(0.5f, 0.5f, 0.5f);
                pfColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                pfColor3f(0.75f, 0.75f, 0.75f);
                pfColor3f(0.75f, 0.75f, 0.75f);
                pfColor3f(0.75f, 0.75f, 0.75f);
                pfColor3f(0.75f, 0.75f, 0.75f);
            }

            pfVertex3f((PFfloat)i*spacing, 0.0f, (PFfloat)-halfSlices*spacing);
            pfVertex3f((PFfloat)i*spacing, 0.0f, (PFfloat)halfSlices*spacing);

            pfVertex3f((PFfloat)-halfSlices*spacing, 0.0f, (PFfloat)i*spacing);
            pfVertex3f((PFfloat)halfSlices*spacing, 0.0f, (PFfloat)i*spacing);
        }
    pfEnd();
}

#endif //PF_COMMON_IMPL


#endif //PF_COMMON_H