#ifndef SCBFORMAT_H_INCLUDED
#define SCBFORMAT_H_INCLUDED

// League of Legends SCB file format
// It's the model format used for static (not animated) meshes
// It's converted from SCO files (SCene Objects) into SCB files (SCene Binary objects)
// It's pretty simple to figure out when you read through a SCO file and its corresponding SCB file at the same time!
//
// This format description was written by DragonGeo2
// Please e-mail any corrections/suggestions/etc. to dragongeo2@gmail.com
// Oh, and lemme know if you use this format specification for anything, I'd be interested!

typedef unsigned int unsigned3[3];
typedef float float3[3];
typedef float float2[2];

struct face
{
    unsigned3 indices; // the indices of the verts that this triangle uses
    char materialname[64];
    float3 uvx; // texcoords
    float3 uvy;
};
typedef struct face face;

// The SCB file starts here:
struct SCB
{
    char *path;
    char magic[9]; // always "r3d2Mesh"

    unsigned short majorVer; // always 2
    unsigned short minorVer; // always 2

    char object_name[129];

    unsigned nVerts;
    unsigned nFaces;
    unsigned hasVCP; // Always equals 1

    float3 center;
    float3 extents;
    float3 *verts; // nVerts

    face *faceslist; // nFaces

    char zero_padding[25]; // Padding
};
typedef struct SCB* SCB;

#endif // SCBFORMAT_H_INCLUDED
