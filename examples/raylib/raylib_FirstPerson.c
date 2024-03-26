#define PF_COMMON_IMPL
#include "../common.h"

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

void PF_BeginMode3D(Camera3D camera);
void PF_EndMode3D(void);

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform);
void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void PF_DrawModel(Model model, Vector3 position, float scale, Color tint);

bool WallCollision(Camera3D* camera, const Image* imMap);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [models] example - first person maze");

    Image dest = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLANK);
    Texture texDest = LoadTextureFromImage(dest);

    PFctx *ctx = pfContextCreate(dest.data, dest.width, dest.height, dest.format);
    pfMakeCurrent(ctx);

    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    Camera3D camera = {
        (Vector3) { 1.2f, 0.5f, 1.2f },
        (Vector3) { 1.5f, 0.5f, 1.5f },
        (Vector3) { 0.0f, 1.0f, 0.0f },
        60.0f,
        CAMERA_PERSPECTIVE
    };

    Image imMap = LoadImage(RESOURCES_PATH "images/cubicmap.png");
    Mesh mesh = GenMeshCubicmap(imMap, (Vector3) { 1.0f, 1.0f, 1.0f });
    Model model = LoadModelFromMesh(mesh);

    Image imTexMap = LoadImage(RESOURCES_PATH "images/cubicmap_atlas.png");
    PFtexture texMap = pfTextureGenFromBuffer(imTexMap.data, imTexMap.width, imTexMap.height, imTexMap.format);

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update

        Vector3 dir = {
            IsKeyDown(KEY_W) - IsKeyDown(KEY_S),
            IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
            IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_SHIFT)
        };

        dir = Vector3Scale(Vector3Normalize(dir), 5.0f * GetFrameTime());

        UpdateCameraPro(&camera, dir, (Vector3) {
            .x = GetMouseDelta().x * 0.1f,
            .y = GetMouseDelta().y * 0.1f,
            .z = 0.0,
        }, 0.0f);

        if (camera.position.y < 0.5f)
        {
            camera.target.y += 0.5f - camera.position.y;
            camera.position.y = 0.5;
        }

        if (WallCollision(&camera, &imMap))
        {
            (void)WallCollision(&camera, &imMap);
        }

        // Draw

        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);
        PF_BeginMode3D(camera);
            pfEnableTexture(&texMap);
            PF_DrawModel(model, (Vector3) { 0 }, 1.0f, WHITE);
            pfDisableTexture();
        PF_EndMode3D();

        UpdateTexture(texDest, dest.data);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(texDest, 0, 0, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadImage(imTexMap);
    UnloadImage(imMap);
    UnloadModel(model);
    CloseWindow();

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

bool WallCollision(Camera3D* camera, const Image* imMap)
{
    Vector2 pos2D = { .x = camera->position.x, .y = camera->position.z };
    Vector2 rdPos2D = { .x = round(pos2D.x), .y = round(pos2D.y) };

    int xMax = MIN(rdPos2D.x + 1, imMap->width - 1);
    int yMax = MIN(rdPos2D.y + 1, imMap->height - 1);

    Rectangle cam_rect = { .x = pos2D.x - 0.2f, .y = pos2D.y - 0.2f, .width = 0.4f, .height = 0.4f };
    Vector2 result_disp = { .x = 0, .y = 0 };

    for (int y = fmaxf(rdPos2D.y - 1, 0); y <= yMax; y += 1)
    {
        for (int x = fmaxf(rdPos2D.x - 1, 0); x <= xMax; x += 1)
        {
            if ((x != rdPos2D.x || y != rdPos2D.y) && ((PFubyte*)imMap->data)[y * imMap->width + x] > 0)
            {
                Rectangle tileRec = { .x = x - 0.5f, .y = y - 0.5f, .width = 1.0f, .height = 1.0f };

                Vector2 dist = { .x = pos2D.x - x, .y = pos2D.y - y };
                Vector2 minDist = { .x = (cam_rect.width + tileRec.width) * 0.5f, .y = (cam_rect.height + tileRec.height) * 0.5f };

                Vector2 collision_vector = { .x = 0, .y = 0 };

                if (fabsf(dist.x) < minDist.x && fabsf(dist.y) < minDist.y)
                {
                    Vector2 overlap = {
                        .x = minDist.x - fabsf(dist.x),
                        .y = minDist.y - fabsf(dist.y),
                    };

                    if (overlap.x < overlap.y)
                    {
                        collision_vector.x = (dist.x > 0) ? overlap.x : -overlap.x;
                    }
                    else
                    {
                        collision_vector.y = (dist.y > 0) ? overlap.y : -overlap.y;
                    }
                }

                if (fabsf(collision_vector.x) > fabsf(result_disp.x)) result_disp.x = collision_vector.x;
                if (fabsf(collision_vector.y) > fabsf(result_disp.y)) result_disp.y = collision_vector.y;
            }
        }
    }

    float adx = fabsf(result_disp.x);
    float ady = fabsf(result_disp.y);

    if (adx > ady)
    {
        camera->position.x += result_disp.x;
        camera->target.x += result_disp.x;
    }
    else
    {
        camera->position.z += result_disp.y;
        camera->target.z += result_disp.y;
    }

    return (adx > 0 && ady > 0);
}
