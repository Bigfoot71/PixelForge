#include "./internal/context/context.h"
#include "./internal/vector.h"
#include "./pixelforge.h"
#include <string.h>

PFrenderlist
pfGenList(void)
{
    PFIrenderlist *list = malloc(sizeof(PFIrenderlist));
    if (list != NULL) {
        *list = pfiGenVector(1, sizeof(PFIrendercall));
    }
    return list;
}

void
pfDeleteList(PFrenderlist* renderList)
{
    if (renderList == NULL) return;

    PFIrenderlist *list = *renderList;

    if (list != NULL) {
        for (PFint i = list->size - 1; i >= 0; --i) {
            PFIrendercall *call = pfiAtVector(list, i);
            pfiDeleteVector(&call->positions);
            pfiDeleteVector(&call->texcoords);
            pfiDeleteVector(&call->normals);
            pfiDeleteVector(&call->colors);
        }
        pfiDeleteVector(list);
        PF_FREE(list);
    }

    *renderList = NULL;
}

void
pfNewList(PFrenderlist renderList)
{
    if (renderList == NULL) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    PFIrenderlist *list = renderList;

    while (list->size > 0) {
        PFIrendercall call = { 0 };
        pfiPopBackVector(list, &call);
        pfiDeleteVector(&call.positions);
        pfiDeleteVector(&call.texcoords);
        pfiDeleteVector(&call.normals);
        pfiDeleteVector(&call.colors);
    }

    G_currentCtx->currentRenderList = renderList;
    pfiMakeContextBackup();
}

void
pfEndList(void)
{
    if (G_currentCtx->currentRenderList == NULL) {
        G_currentCtx->errCode = PF_INVALID_OPERATION;
    }
    G_currentCtx->currentRenderList = NULL;
    pfiRestoreContext();
}

void
pfCallList(const PFrenderlist renderList)
{
    pfiMakeContextBackup();

    PFIrenderlist *list = renderList;
    for (const PFIrendercall *call = list->data; (const void*)call < pfiEndVector(list); ++call) {
        float *positions = call->positions.data;
        float *texcoords = call->texcoords.data;
        float *normals = call->normals.data;
        PFcolor *colors = call->colors.data;

        memcpy(G_currentCtx->faceMaterial, call->faceMaterial, 2 * sizeof(PFImaterial));
        pfBindTexture(call->texture);

        pfBegin(call->drawMode);
            for (PFsizei i = 0; i < call->positions.size; ++i) {
                pfColor4ubv((PFubyte*)(colors + i));
                pfTexCoordfv(texcoords + 2 * i);
                pfNormal3fv(normals + 3 * i);
                pfVertex4fv(positions + 4 * i);
            }
        pfEnd();
    }

    pfiRestoreContext();
}
