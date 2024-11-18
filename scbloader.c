
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scbloader.h"

SCB scb_load_bin(char *path)
{
    SCB scb = calloc(1, sizeof(struct SCB));

    scb->path = path;
    FILE *file = fopen(path, "rb");

    fread(scb->magic, 1, 8, file);
    fread(&(scb->majorVer), 1, sizeof(unsigned short), file);
    fread(&(scb->minorVer), 1, sizeof(unsigned short), file);
    fread(scb->object_name, 1, 128, file);

    fread(&(scb->nVerts), 1, sizeof(unsigned), file);
    fread(&(scb->nFaces), 1, sizeof(unsigned), file);
    fread(&(scb->unk),    1, sizeof(unsigned), file);

    fread(&(scb->center),  1, sizeof(float3), file);
    fread(&(scb->extents), 1, sizeof(float3), file);

    //printf("Reading %d vertices (of size %d)\n", scb->nVerts, sizeof(float3));
    scb->verts =   calloc(scb->nVerts, sizeof(float3));
    fread(scb->verts,     scb->nVerts, sizeof(float3), file);
    fseek(file, sizeof(float3), SEEK_CUR); // apparently we need to skip one in order to get meaningful data...

    long int pos = ftell(file);
    //printf("At offset %X\n", pos);

    //printf("Reading %d faces (of size %d)\n", scb->nFaces, sizeof(face));
    scb->faceslist = calloc(scb->nFaces, sizeof(face));
    fread(scb->faceslist,   scb->nFaces, sizeof(face), file);

    fclose(file);
    return scb;
}

SCB scb_load_txt(char *path)
{
    SCB scb = calloc(1, sizeof(struct SCB));
    scb->path = path;
    FILE *file = fopen(path, "r");
    
    const int max_line_len = 16;
    char line[max_line_len];
    fgets(line, max_line_len, file); // [ObjectBegin]
    //puts(line);
    fscanf(file, "Name= %128s\n", &(scb->object_name));
    //printf("Name= %s\n", scb->object_name);
    fscanf(file, "CentralPoint= %f %f %f\n", &(scb->center[0]), &(scb->center[1]), &(scb->center[2]));
    //printf("CentralPoint= %f %f %f\n", scb->center[0], scb->center[1], scb->center[2]);
    fscanf(file, "Verts= %d\n", &(scb->nVerts));
    //printf("Verts= %d\n", scb->nVerts);
    scb->verts = calloc(scb->nVerts, sizeof(float3));
    for(int i = 0; i < scb->nVerts; i++)
    {
        fscanf(file, "%f %f %f\n", &(scb->verts[i][0]), &(scb->verts[i][1]), &(scb->verts[i][2]));
        //printf("%f %f %f\n", scb->verts[i][0], scb->verts[i][1], scb->verts[i][2]);
    }
    fscanf(file, "Faces= %d\n", &(scb->nFaces));
    //printf("Faces= %d\n", scb->nFaces);
    scb->faceslist = calloc(scb->nFaces, sizeof(face));
    for(int i = 0; i < scb->nFaces; i++)
    {
        int three = 3;
        fscanf(
            file,
            "%d %d %d %d %63s %f %f %f %f %f %f\n",
            &(three),
            &(scb->faceslist[i].indices[0]),
            &(scb->faceslist[i].indices[1]),
            &(scb->faceslist[i].indices[2]),
            &(scb->faceslist[i].materialname),
            &(scb->faceslist[i].uv[0][0]),
            &(scb->faceslist[i].uv[0][1]),
            &(scb->faceslist[i].uv[1][0]),
            &(scb->faceslist[i].uv[1][1]),
            &(scb->faceslist[i].uv[2][0]),
            &(scb->faceslist[i].uv[2][1])
        );
        /*
        //printf(
            "%d %d %d %d %s %f %f %f %f %f %f\n",
            three,
            scb->faceslist[i].indices[0],
            scb->faceslist[i].indices[1],
            scb->faceslist[i].indices[2],
            scb->faceslist[i].materialname,
            scb->faceslist[i].uv[0][0],
            scb->faceslist[i].uv[0][1],
            scb->faceslist[i].uv[1][0],
            scb->faceslist[i].uv[1][1],
            scb->faceslist[i].uv[2][0],
            scb->faceslist[i].uv[2][1]
        );
        */
    }
    //fgets(line, max_line_len, file); // [ObjectEnd]
    //puts(line);

    fclose(file);
    return scb;
}

SCB scb_load(char *path)
{
    return (!strcmp(path + strlen(path) - (sizeof(".sco") - 1), ".sco")) ? scb_load_txt(path) : scb_load_bin(path);
}

void obj_save_from_scb(SCB model)
{
    char* path = calloc(1, strlen(model->path) + 4);
    sprintf(path, "%s.obj", model->path);

    //printf("Saving to %s\n", path);
    FILE *file = fopen(path, "wb");

    fprintf(file, "# Converted from .scb model '%s'\n", model->object_name);

    //fprintf(file, "\n# Vertices\n", model->object_name);

    for(int i = 0; i < model->nVerts; i++)
    {
        fprintf(file, "v %.4f %.4f %.4f\n", model->verts[i][0], model->verts[i][1], model->verts[i][2]);
    }

    //fprintf(file, "\n# UVs\n", model->object_name);

    for(int i = 0; i < model->nFaces; i++)
    {
        face f = model->faceslist[i];
        fprintf(file, "vt %.4f %.4f\n", f.uv[0][0], 1.0 - f.uv[0][1]);
        fprintf(file, "vt %.4f %.4f\n", f.uv[1][0], 1.0 - f.uv[1][1]);
        fprintf(file, "vt %.4f %.4f\n", f.uv[2][0], 1.0 - f.uv[2][1]);
    }

    //fprintf(file, "\n# Faces\n", model->object_name);

    for(int i = 0; i < model->nFaces; i++)
    {
        face f = model->faceslist[i];
        fprintf(file, "f %d/%d %d/%d %d/%d\n",
            f.indices[0] + 1, i * 3 + 0 + 1,
            f.indices[1] + 1, i * 3 + 1 + 1,
            f.indices[2] + 1, i * 3 + 2 + 1
        );
    }

    fclose(file);
}

