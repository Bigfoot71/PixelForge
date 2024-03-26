#include "pixelforge.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PF_COMMON_IMPL
#include "../common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600


void PF_BeginMode3D(Camera3D camera);
void PF_EndMode3D(void);

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform);
void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void PF_DrawModel(Model model, Vector3 position, float scale, Color tint);


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [models] example - first person maze");

    Image dest = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLANK);
    Texture texDest = LoadTextureFromImage(dest);

    PFctx *ctx = pfContextCreate(dest.data, dest.width, dest.height, dest.format);
    pfMakeCurrent(ctx);
    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    Camera3D camera = {
        (Vector3) { 0.2f, 0.4f, 0.2f },
        (Vector3) { 0.185f, 0.4f, 0.0f },
        (Vector3) { 0.0f, 1.0f, 0.0f },
        45.0f
    };

    Image imMap = LoadImage(RESOURCES_PATH "images/cubicmap.png");
    Mesh mesh = GenMeshCubicmap(imMap, (Vector3) { 1.0f, 1.0f, 1.0f });
    Model model = LoadModelFromMesh(mesh);

    Image imTexMap = LoadImage(RESOURCES_PATH "images/cubicmap_atlas.png");
    PFtexture texMap = pfTextureGenFromBuffer(imTexMap.data, imTexMap.width, imTexMap.height, imTexMap.format);

    Color *mapPixels = LoadImageColors(imMap);
    UnloadImage(imMap);

    Vector3 mapPosition = { -16.0f, 0.0f, -8.0f };

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update

        Vector3 oldCamPos = camera.position;

        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        Vector2 playerPos = { camera.position.x, camera.position.z };
        float playerRadius = 0.1f;

        int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
        int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);

        if (playerCellX < 0) playerCellX = 0;
        else if (playerCellX >= imMap.width) playerCellX = imMap.width - 1;

        if (playerCellY < 0) playerCellY = 0;
        else if (playerCellY >= imMap.height) playerCellY = imMap.height - 1;

        for (int y = 0; y < imMap.height; y++)
        {
            for (int x = 0; x < imMap.width; x++)
            {
                if ((mapPixels[y*imMap.width + x].r == 255) &&
                    (CheckCollisionCircleRec(playerPos, playerRadius,
                    (Rectangle){ mapPosition.x - 0.5f + x*1.0f, mapPosition.z - 0.5f + y*1.0f, 1.0f, 1.0f })))
                {
                    camera.position = oldCamPos;
                }
            }
        }

        // Draw

        pfClear();
        PF_BeginMode3D(camera);
            pfEnableTexture(&texMap);
            PF_DrawModel(model, mapPosition, 1.0f, WHITE);
            pfDisableTexture();
        PF_EndMode3D();

        UpdateTexture(texDest, dest.data);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(texDest, 0, 0, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadImageColors(mapPixels);   // Unload color array
    UnloadImage(imTexMap);          // Unload map texture
    UnloadModel(model);             // Unload map model
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}


void PF_BeginMode3D(Camera3D camera)
{
    pfMatrixMode(PF_PROJECTION);    // Switch to projection matrix
    pfPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
    pfLoadIdentity();               // Reset current matrix (projection)

    float aspect = (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT;

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = 0.01*tan(DEG2RAD(camera.fovy*0.5));
        double right = top*aspect;

        pfFrustum(-right, right, -top, top, 0.01, 1000.0);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        pfOrtho(-right, right, -top,top, 0.01, 1000.0);
    }

    pfMatrixMode(PF_MODELVIEW);     // Switch back to modelview matrix
    pfLoadIdentity();               // Reset current matrix (modelview)

    // Setup Camera view
    PFmat4f matView = pfMat4fLookAt((PFvec3f*)(&camera.position), (PFvec3f*)(&camera.target), (PFvec3f*)(&camera.up));
    pfMultMatrixMat4f(&matView);    // Multiply modelview matrix by view matrix (camera)

    pfEnableDepthTest();            // Enable DEPTH_TEST for 3D
}

void PF_EndMode3D(void)
{
    pfMatrixMode(PF_PROJECTION);    // Switch to projection matrix
    pfPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    pfMatrixMode(PF_MODELVIEW);     // Switch back to modelview matrix
    pfLoadIdentity();               // Reset current matrix (modelview)

    pfDisableDepthTest();           // Disable DEPTH_TEST for 2D
}

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    if (mesh.animVertices)
    {
        pfEnableStatePointer(PF_VERTEX_ARRAY, mesh.animVertices);
        pfEnableStatePointer(PF_NORMAL_ARRAY, mesh.animNormals);
    }
    else
    {
        pfEnableStatePointer(PF_VERTEX_ARRAY, mesh.vertices);
        pfEnableStatePointer(PF_NORMAL_ARRAY, mesh.normals);
    }

    pfEnableStatePointer(PF_TEXTURE_COORD_ARRAY, mesh.texcoords);
    pfEnableStatePointer(PF_COLOR_ARRAY, mesh.colors);

    pfPushMatrix();
        pfMultMatrixMat4f((PFmat4f*)(&transform));
        pfColor4ub(material.maps[MATERIAL_MAP_DIFFUSE].color.r,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.g,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.b,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.a);

        if (mesh.indices != NULL) pfDrawVertexArrayElements(0, mesh.triangleCount*3, mesh.indices);
        else pfDrawVertexArray(0, mesh.vertexCount);
    pfPopMatrix();

    pfDisableStatePointer(PF_VERTEX_ARRAY);
    pfDisableStatePointer(PF_TEXTURE_COORD_ARRAY);
    pfDisableStatePointer(PF_NORMAL_ARRAY);
    pfDisableStatePointer(PF_COLOR_ARRAY);

    pfDisableTexture();
}

void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    PFmat4f matScale = pfMat4fScale(scale.x, scale.y, scale.z);
    PFmat4f matRotation = pfMat4fRotate((PFvec3f*)(&rotationAxis), DEG2RAD(rotationAngle));
    PFmat4f matTranslation = pfMat4fTranslate(position.x, position.y, position.z);

    PFmat4f matTransform = pfMat4fMul(&matScale, &matRotation);
    matTransform = pfMat4fMul(&matTransform, &matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    PFmat4f modelTransform = pfMat4fMul((PFmat4f*)(&model.transform), &matTransform);
    memcpy(&model.transform, &modelTransform, sizeof(Matrix));

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((((float)color.r/255.0f)*((float)tint.r/255.0f))*255.0f);
        colorTint.g = (unsigned char)((((float)color.g/255.0f)*((float)tint.g/255.0f))*255.0f);
        colorTint.b = (unsigned char)((((float)color.b/255.0f)*((float)tint.b/255.0f))*255.0f);
        colorTint.a = (unsigned char)((((float)color.a/255.0f)*((float)tint.a/255.0f))*255.0f);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        PF_DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}

void PF_DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    PF_DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}
