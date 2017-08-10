#include "mesh.h"
#include "gl.h"

#include <string.h>

#define BUFFER_OFFSET(i) ((void*)(i))

/*
* I got a little lazy, so these verts are stored in global memory and
* just copied to the heap in the createCube function You probably
* shouldn't do silly stuff like this normally
*/
static Vertex cubeVerts[] = {
    { { 1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}, { 0, 0 }, },
    { { 1.0,  1.0,  1.0 }, { 0.0,  0.0,  1.0 }, { 0, 1 }, },
    { { -1.0,  1.0,  1.0 }, { 0.0,  0.0,  1.0 }, { 1, 1 }, },
    { { -1.0, -1.0,  1.0 }, { 0.0,  0.0,  1.0 }, { 1, 0 }, },
    { { 1.0, -1.0, -1.0 }, { 0.0,  0.0, -1.0 }, { 0, 0 }, },
    { { -1.0, -1.0, -1.0 }, { 0.0,  0.0, -1.0 }, { 0, 1 }, },
    { { -1.0,  1.0, -1.0 }, { 0.0,  0.0, -1.0 }, { 1, 1 }, },
    { { 1.0,  1.0, -1.0 }, { 0.0,  0.0, -1.0 }, { 1, 0 }, },
    { { -1.0, -1.0, -1.0 }, { -1.0,  0.0,  0.0 }, { 0, 0 }, },
    { { -1.0, -1.0,  1.0 }, { -1.0,  0.0,  0.0 }, { 0, 1 }, },
    { { -1.0,  1.0,  1.0 }, { -1.0,  0.0,  0.0 }, { 1, 1 }, },
    { { -1.0,  1.0, -1.0 }, { -1.0,  0.0,  0.0 }, { 1, 0 }, },
    { { 1.0, -1.0, -1.0 }, { 1.0,  0.0,  0.0 }, { 0, 0 }, },
    { { 1.0,  1.0, -1.0 }, { 1.0,  0.0,  0.0 }, { 0, 1 }, },
    { { 1.0,  1.0,  1.0 }, { 1.0,  0.0,  0.0 }, { 1, 1 }, },
    { { 1.0, -1.0,  1.0 }, { 1.0,  0.0,  0.0 }, { 1, 0 }, },
    { { -1.0,  1.0, -1.0 }, { 0.0,  1.0,  0.0 }, { 0, 0 }, },
    { { -1.0,  1.0,  1.0 }, { 0.0,  1.0,  0.0 }, { 0, 1 }, },
    { { 1.0,  1.0,  1.0 }, { 0.0,  1.0,  0.0 }, { 1, 1 }, },
    { { 1.0,  1.0, -1.0 }, { 0.0,  1.0,  0.0 }, { 1, 0 }, },
    { { -1.0, -1.0, -1.0 }, { 0.0, -1.0,  0.0 }, { 0, 0 }, },
    { { 1.0, -1.0, -1.0 }, { 0.0, -1.0,  0.0 }, { 0, 1 }, },
    { { 1.0, -1.0,  1.0 }, { 0.0, -1.0,  0.0 }, { 1, 1 }, },
    { { -1.0, -1.0,  1.0 }, { 0.0, -1.0,  0.0 }, { 1, 0 }, },
};

static unsigned int cubeIndices[] = {
    0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
};

/*
* Allocate the memory for a mesh with the specifed number of vertices
* and indices
*/
Mesh*
createMesh(size_t numVerts, size_t numIndices)
{
    Mesh* mesh = (Mesh*) malloc(sizeof(Mesh));
    mesh->numVerts = numVerts;
    mesh->numIndices = numIndices;
    mesh->verts = (Vertex*) calloc(numVerts, sizeof(Vertex));
    mesh->indices = (unsigned int*) calloc(numIndices, sizeof(int));

    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ibo);
    glGenBuffers(1, &mesh->tbo);
    bindVBOs(mesh->vbo, mesh->ibo, mesh->tbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVerts * sizeof(Vertex), mesh->verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);
    unbindVBOs();
    printf("Finished building VBO for mesh\n");

    return mesh;
}

/*
* Free up the memory used by a mesh
*/
void
destroyMesh(Mesh* mesh)
{
    if (mesh) {
        if (mesh->indices)
        free(mesh->indices);
        if (mesh->verts)
        free(mesh->verts);
        free(mesh);
    }
}

/*
* Draw a mesh.
* Will also draw the debug lines toggled in the provided flags
*/
void
renderMesh(Mesh* mesh, DrawingFlags* flags)
{
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

    if (flags->lighting)
    glEnable(GL_LIGHTING);

    if (flags->textures)
    glEnable(GL_TEXTURE_2D);

    // Use vertex arrays to send the vertices to the GPU all at
    // once using glVertexPointer, which is in fact deprecated, but it's
    // closer to how you do this in modern GL using VBOs.
    if (flags->rm == VA) {
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), mesh->verts);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), &mesh->verts[0].normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mesh->verts[0].tc);

        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, mesh->indices);

        glPopClientAttrib();
    }
    else if (flags->rm == VBO){
        bindVBOs(mesh->vbo, mesh->ibo);

        glPushAttrib(GL_CURRENT_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
        glNormalPointer(GL_FLOAT, sizeof(Vertex), &mesh->verts[0].normal);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mesh->verts[0].tc);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, mesh->indices);
        glPopAttrib();
        unbindVBOs();

    } else if (flags->rm == im) {
        // Specify everything one vertex at a time using glBegin and glEnd
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < mesh->numIndices; ++i) {
            unsigned int index = mesh->indices[i];
            submitNormal(mesh->verts[index].normal);
            submitTexCoord(mesh->verts[index].tc);
            submitVertex(mesh->verts[index].pos);
        }
        glEnd();
    }

    glPopAttrib();

    if (flags->axes) {
        drawAxes();
    }

    if (flags->normals) {
        for (size_t i = 0; i < mesh->numVerts; ++i) {
            Vertex v = mesh->verts[i];
            Vec3f n = addVec3f(mulVec3f(v.normal, 0.1), v.pos);
            drawLine(YELLOW, v.pos, n);
        }
    }
}

/*
* Create a simple cube with one quad per side.
* You could of course make this code like the plane and allow for an arbitrary number of quads per side,
* but I don't think that's really necessary here
*/
Mesh*
createCube()
{
    Mesh* mesh = createMesh(24, 36);
    memcpy(mesh->verts, cubeVerts, 24 * sizeof(Vertex));
    memcpy(mesh->indices, cubeIndices, 36 * sizeof(unsigned int));
    return mesh;
}

/*
* Create a plane with row x cols number of quads.
*/

// Strategy pattern
Mesh*
createPlaneColMajor(float width, float height, size_t rows, size_t cols)
{
    Mesh* mesh = createMesh((rows + 1) * (cols + 1), rows * cols * 6);

    // Vertices
    float x, x0 = 0.5 * width, dx = width / (float)cols;
    float z, z0 = 0.5 * height, dz = height / (float)rows;
    for (size_t i = 0; i <= cols; ++i) {
        x = i * dx - x0;
        for (size_t j = 0; j <= rows; ++j) {
            z = j * dz - z0;
            size_t index = i * (rows + 1) + j;
            mesh->verts[index].pos = (Vec3f) { x, 0, z };
            mesh->verts[index].normal.y = 1.0;
            mesh->verts[index].tc.x = (float) i / (float) cols;
            mesh->verts[index].tc.y = (float) j / (float) rows;
        }
    }

    // Indices i.e. elements
    size_t index = 0;
    for (size_t j = 0; j < cols; ++j) {
        for (size_t i = 0; i < rows ; ++i) {
            mesh->indices[index++] = j * (rows + 1) + i;
            mesh->indices[index++] = (j + 1) * (rows + 1) + i;
            mesh->indices[index++] = j * (rows + 1) + i + 1;
            mesh->indices[index++] = j * (rows + 1) + i + 1;
            mesh->indices[index++] = (j + 1) * (rows + 1) + i;
            mesh->indices[index++] = (j + 1) * (rows + 1) + i + 1;
        }
    }

    return mesh;
}

Mesh*
createPlaneRowMajor(float width, float height, size_t rows, size_t cols)
{
    Mesh* mesh = createMesh((rows + 1) * (cols + 1), rows * cols * 6);

    // Vertices
    float x, x0 = 0.5 * width, dx = width / (float)cols;
    float z, z0 = 0.5 * height, dz = height / (float)rows;
    for (size_t j = 0; j <= rows; ++j) {
        z = j * dz - z0;
        for (size_t i = 0; i <= cols; ++i) {
            x = i * dx - x0;
            size_t index = j * (cols + 1) + i;
            mesh->verts[index].pos = (Vec3f) { x, 0, z };
            mesh->verts[index].normal.y = 1.0;
            mesh->verts[index].tc.x = (float) i / (float) cols;
            mesh->verts[index].tc.y = (float) j / (float) rows;
        }
    }

    // Indices i.e. elements
    size_t index = 0;
    for (size_t i = 0; i < rows ; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            mesh->indices[index++] = i * (cols + 1) + j;
            mesh->indices[index++] = (i + 1) * (cols + 1) + j;
            mesh->indices[index++] = i * (cols + 1) + j + 1;
            mesh->indices[index++] = i * (cols + 1) + j + 1;
            mesh->indices[index++] = (i + 1) * (cols + 1) + j;
            mesh->indices[index++] = (i + 1) * (cols + 1) + j + 1;
        }
    }

    return mesh;
}

Mesh*
createPlane(float width, float height, size_t rows, size_t cols)
{
    return createPlaneRowMajor(width, height, rows, cols);
}


/*
* Create a sphere with the specified number of stacks and slices
*/
Mesh*
createSphere(size_t stacks, size_t slices)
{
    Mesh* mesh = createMesh((stacks + 1) * (slices + 1), stacks * slices * 6);

    // Vertices
    float u, du = 2.0 * M_PI / (float)slices;
    float v, dv = M_PI / (float)stacks;
    for (size_t j = 0; j <= stacks; ++j) {
        v = j * dv;
        for (size_t i = 0; i <= slices; ++i) {
            u = i * du;
            size_t index = j * (slices + 1) + i;
            mesh->verts[index].pos.x = sinf(v) * cosf(u);
            mesh->verts[index].pos.y = sinf(v) * sinf(u);
            mesh->verts[index].pos.z = cosf(v);
            mesh->verts[index].normal = normaliseVec3f(mesh->verts[index].pos);
            // These texture coordinates aren't great, but it's hard to get
            // something that looks nice at the poles of your sphere
            mesh->verts[index].tc.y = 1.0 - (float) i / (float)slices;
            mesh->verts[index].tc.x = 1.0 - (float) j / (float)stacks;
        }
    }

    // Indices i.e. elements
    size_t index = 0;
    for (size_t i = 0; i < stacks; ++i) {
        for (size_t j = 0; j < slices; ++j) {
            mesh->indices[index++] = i * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j + 1;
            mesh->indices[index++] = i * (slices + 1) + j + 1;
            mesh->indices[index++] = i * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j + 1;
        }
    }

    return mesh;
}

/*
* Create a cylinder with the specified stacks and slices
* and endcaps facing down the z axis
*/
Mesh*
createCylinder(size_t stacks, size_t slices, float radius)
{
    Mesh* mesh = createMesh((stacks + 1) * (slices + 1) + (slices + 1) * 2,
    stacks * slices * 6 + slices * 3 * 2);

    // Sides vertices
    float u, du = 2.0 * M_PI / slices;
    for (size_t j = 0; j <= stacks; ++j) {
        for (size_t i = 0; i <= slices; ++i) {
            float u = i * du;
            size_t index = j * (slices + 1) + i;
            mesh->verts[index].pos.x = radius * cosf(u);
            mesh->verts[index].pos.y = radius * sinf(u);
            mesh->verts[index].pos.z = j / (float) stacks - 0.5;
            mesh->verts[index].tc.x = 1.0 - (float) i / (float) slices;
            mesh->verts[index].tc.y = 1.0 - (float) j / (float) stacks;
            mesh->verts[index].normal = normaliseVec3f(mesh->verts[index].pos);
            mesh->verts[index].normal.z = 0;
        }
    }

    // Sides indices i.e. elements
    size_t index = 0;
    for (size_t i = 0; i < stacks; ++i) {
        for (size_t j = 0; j < slices; ++j) {
            mesh->indices[index++] = i * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j + 1;
            mesh->indices[index++] = i * (slices + 1) + j + 1;
            mesh->indices[index++] = i * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j;
            mesh->indices[index++] = (i + 1) * (slices + 1) + j + 1;
        }
    }

    // Endcap vertices
    // The texture coordinates for these might not work for radius
    // greater than 1, you might need to enable GL_REPEAT for texture wrap
    size_t endcapVerts = (slices + 1) * (stacks + 1);

    for (size_t i = 0; i <= slices; ++i) {
        u = i * du;
        float x = radius * cosf(u);
        float y = radius * sinf(u);

        index = endcapVerts + i;
        mesh->verts[index].pos.x = x;
        mesh->verts[index].pos.y = y;
        mesh->verts[index].pos.z = -0.5;
        mesh->verts[index].tc.x = cosf(u) * 0.5 + 0.5;
        mesh->verts[index].tc.y = sinf(u) * 0.5 + 0.5;
        mesh->verts[index].normal.z = -1;

        index += (slices + 1);
        mesh->verts[index].pos.x = x;
        mesh->verts[index].pos.y = y;
        mesh->verts[index].pos.z = 0.5;
        mesh->verts[index].tc.x = cosf(u) * 0.5 + 0.5;
        mesh->verts[index].tc.y = sinf(u) * 0.5 + 0.5;
        mesh->verts[index].normal.z = 1;
    }

    // Endcap indices, i.e. elements
    size_t endcapIndices = slices * stacks * 6;
    index = endcapIndices;
    for (size_t i = endcapVerts + 1;
        i < endcapVerts + (slices + 1) - 1; ++i) {
            mesh->indices[index++] = endcapVerts;
            mesh->indices[index++] = i + 1;
            mesh->indices[index++] = i;
        }

        for (size_t i = endcapVerts + (slices + 1) + 1;
        i < endcapVerts + (slices + 1) * 2 - 1; ++i) {
            mesh->indices[index++] = i;
            mesh->indices[index++] = i + 1;
            mesh->indices[index++] = endcapVerts + slices + 1;
        }


        return mesh;
    }

    /*
    * Draw a line from a to b, assumes you have already called glBegin
    * and will call glEnd sometime after
    */
    void
    drawLine(Vec3f color, Vec3f a, Vec3f b)
    {
        glPushAttrib(GL_CURRENT_BIT);
        glBegin(GL_LINES);
        submitColor(color);
        submitVertex(a);
        submitVertex(b);
        glEnd();
        glPopAttrib();
    }

    /*
    * Draw a parabola for the specified initial velocity and gravity
    */
    void
    drawParabola(Vec3f color, Vec3f vel, float g, DrawingFlags* flags)
    {
        float tof = (2.0 * vel.y) / g; // time of flight
        float step = tof / (float) flags->tess[0];

        // this loop doesn't draw the last tangent, but usually it won't be
        // visible through the floor so it's not a huge deal
        for (size_t i = 0; i < flags->tess[0]; ++i) {
            float t0 = i * step;
            float t1 = (i + 1) * step;

            Vec3f a = { t0 * vel.x, t0 * vel.y - 0.5 * g * t0 * t0, t0 * vel.z };
            Vec3f b = { t1 * vel.x, t1 * vel.y - 0.5 * g * t1 * t1, t1 * vel.z };

            drawLine(color, a, b);

            if (flags->normals) {
                Vec3f t = { vel.x, vel.y - g * t0, vel.z };
                t = mulVec3f(normaliseVec3f(t), 0.05);
                t = addVec3f(a, t);
                drawLine(CYAN, a, t);
            }
        }
    }


    /* ----------- VBO HELPER METHODS ------------- */

    void bindVBOs(GLuint vbo, GLuint ibo, GLuint tbo){
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    }

    void unbindVBOs(){
        int buffer;

        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
        if (buffer != 0) glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &buffer);
        if (buffer != 0)glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        //glDisableClientState(GL_VERTEX_ARRAY);
    }
