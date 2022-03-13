#define _USE_MATH_DEFINES

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SPHERE "sphere"
#define CUBE "box"
#define CONE "cone"
#define PLANE "plane"

void points_vertex(const float x, const float y, const float z, unsigned int *pos, float points[]) {
    points[*pos] = x;
    points[*pos + 1] = y;
    points[*pos + 2] = z;
    *pos += 3;
}

void points_write(const char *filename, const unsigned int nVertices, const float points[]) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "failed to open file");
        exit(1);
    }

    fwrite(&nVertices, sizeof(unsigned int), 1, fp);

    fwrite(points, 3 * sizeof(float), nVertices, fp);
    fclose(fp);
}

void model_plane_vertices(const float length, const unsigned int divisions, float *points) {
    const float o = -length / 2.0f;
    const float d = length / (float) divisions;

    unsigned int pos = 0;

    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            points_vertex(o + d * (i - 1), 0, o + d * (j - 1), &pos, points); //P1
            points_vertex(o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, 0, o + d * j, &pos, points); //P2

            /*Cull face*/
            points_vertex(o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * (i - 1), 0, o + d * (j - 1), &pos, points); //P1
            points_vertex(o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(o + d * i, 0, o + d * j, &pos, points); //P2
        }
    }
}

static inline unsigned int model_plane_nVertices(const unsigned int divisions) { return divisions * divisions * 12; }

void model_plane_write(const char *filepath, const float length, const unsigned int divisions) {
    const unsigned int nVertices = model_plane_nVertices(divisions);
    float points[3 * nVertices];
    model_plane_vertices(length, divisions, points);
    points_write(filepath, nVertices, points);
}

void model_cube_vertices(const float length, const unsigned int divisions, float points[]) {
    const float o = -length / 2.0f;
    const float d = length / (float) divisions;

    unsigned int pos = 0;

    for (unsigned int m = 1; m <= divisions; m++) {
        for (unsigned int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            // top
            points_vertex(o + d * (i - 1), -o, o + d * (j - 1), &pos, points); //P1
            points_vertex(o + d * (i - 1), -o, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, -o, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(o + d * i, -o, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(o + d * (i - 1), -o, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, -o, o + d * j, &pos, points); //P2

            // bottom
            points_vertex(o + d * (i - 1), o, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * (i - 1), o, o + d * (j - 1), &pos, points); //P1
            points_vertex(o + d * i, o, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(o + d * (i - 1), o, o + d * j, &pos, points); //P1'z
            points_vertex(o + d * i, o, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(o + d * i, o, o + d * j, &pos, points); //P2

            // left
            points_vertex(o, o + d * (i - 1), o + d * (j - 1), &pos, points); //P1
            points_vertex(o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
            points_vertex(o, o + d * i, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(o, o + d * i, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
            points_vertex(o, o + d * i, o + d * j, &pos, points); //P2

            // right
            points_vertex(-o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
            points_vertex(-o, o + d * (i - 1), o + d * (j - 1), &pos, points); //P1
            points_vertex(-o, o + d * i, o + d * (j - 1), &pos, points); //P1'x

            points_vertex(-o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
            points_vertex(-o, o + d * i, o + d * (j - 1), &pos, points); //P1'x
            points_vertex(-o, o + d * i, o + d * j, &pos, points); //P2

            // front
            points_vertex(o + d * (i - 1), o + d * (j - 1), o, &pos, points); //P1
            points_vertex(o + d * (i - 1), o + d * j, o, &pos, points); //P1'z
            points_vertex(o + d * i, o + d * (j - 1), o, &pos, points); //P1'x

            points_vertex(o + d * i, o + d * (j - 1), o, &pos, points); //P1'x
            points_vertex(o + d * (i - 1), o + d * j, o, &pos, points); //P1'z
            points_vertex(o + d * i, o + d * j, o, &pos, points); //P2

            // back
            points_vertex(o + d * (i - 1), o + d * j, -o, &pos, points); //P1'z
            points_vertex(o + d * (i - 1), o + d * (j - 1), -o, &pos, points); //P1
            points_vertex(o + d * i, o + d * (j - 1), -o, &pos, points); //P1'x

            points_vertex(o + d * (i - 1), o + d * j, -o, &pos, points); //P1'z
            points_vertex(o + d * i, o + d * (j - 1), -o, &pos, points); //P1'x
            points_vertex(o + d * i, o + d * j, -o, &pos, points); //P2
        }
    }
}

unsigned int model_cube_nVertices(const unsigned int divisions) { return divisions * divisions * 36; }

void model_cube_write(const char *filepath, const float length, const unsigned int divisions) {
    const unsigned int nVertices = model_cube_nVertices(divisions);
    float points[3 * nVertices];
    model_cube_vertices(length, divisions, points);
    points_write(filepath, nVertices, points);
}

static inline void
model_cone_vertex(const float r, const float height, const float theta, const float h, unsigned int *pos,
                  float *points) {
    points_vertex(r * h * cos(theta), 2 * (height + h), r * h * sin(theta), pos, points);
}

void model_cone_vertices(const float r, const float height, const unsigned int slices, const unsigned int stacks,
                         float points[]) {
    // https://www.math3d.org/5gLCN9yBz

    const float s = 2.0f * (float) M_PI / (float) slices;
    const float t = height / (float) stacks;
    const float theta = -M_PI;
    const float h = -height;

    unsigned int pos = 0;

    for (unsigned int m = 1; m <= slices; m++) {
        for (unsigned int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            //base
            points_vertex(0, 0, 0, &pos, points); //O
            model_cone_vertex(r, height, theta + s * (i - 1), h, &pos, points); //P1
            model_cone_vertex(r, height, theta + s * i, h, &pos, points); //P2

            model_cone_vertex(r, height, theta + s * i, h + t * (j - 1), &pos, points); // P2
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * j, &pos, points); // P1'
            model_cone_vertex(r, height, theta + s * i, h + t * j, &pos, points); //P2'

            model_cone_vertex(r, height, theta + s * i, h + t * (j - 1), &pos, points); // P2
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * (j - 1), &pos, points); // P1
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * j, &pos, points); // P1'
        }
    }
}

static inline unsigned int model_cone_nVertices(const unsigned int stacks, const unsigned int slices) {
    return slices * stacks * 9;
};

void model_cone_write(const char *filepath, const float radius, const float height, const unsigned int slices,
                      const unsigned int stacks) {
    const unsigned int nVertices = model_cone_nVertices(stacks, slices);
    float points[3 * nVertices];
    model_cone_vertices(radius, height, slices, stacks, points);
    points_write(filepath, nVertices, points);
}


static inline void
model_sphere_vertex(const float r, const float theta, const float phi, unsigned int *pos, float *points) {
    points_vertex(r * sin(theta) * cos(phi), r * sin(phi), r * cos(theta) * cos(phi), pos, points);
}

static void model_sphere_vertices(const float r, const unsigned int slices, const unsigned int stacks, float points[]) {
    // https://www.math3d.org/EumEEZBKe
    // https://www.math3d.org/zE4n6xayX

    const float s = 2.0f * (float) M_PI / (float) slices;
    const float t = M_PI / (float) stacks;
    const float theta = -M_PI;
    const float phi = -M_PI / 2.0f;

    unsigned int pos = 0;

    for (unsigned int m = 1; m <= slices; m++) {
        for (unsigned int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            model_sphere_vertex(r, theta + s * (i - 1), phi + t * j, &pos, points); // P1'
            model_sphere_vertex(r, theta + s * i, phi + t * (j - 1), &pos, points); // P2
            model_sphere_vertex(r, theta + s * i, phi + t * j, &pos, points); //P2'

            model_sphere_vertex(r, theta + s * (i - 1), phi + t * (j - 1), &pos, points); // P1
            model_sphere_vertex(r, theta + s * i, phi + t * (j - 1), &pos, points); // P2
            model_sphere_vertex(r, theta + s * (i - 1), phi + t * j, &pos, points); // P1'
        }
    }
}

static inline unsigned int model_sphere_nVertices(const unsigned int slices, const unsigned int stacks) {
    return slices * stacks * 6;
}

void model_sphere_write(const char *filepath, float radius, unsigned int slices, unsigned int stacks) {
    const unsigned int nVertices = model_sphere_nVertices(slices, stacks);
    float points[3 * nVertices];
    model_sphere_vertices(radius, slices, stacks, points);
    points_write(filepath, nVertices, points);
}

int main(int argc, char *argv[]) {
    if (argc < 5) { printf("Not enough arguments"); }
    else {
        const char *filepath = argv[argc - 1];
        const char *polygon = argv[1];

        if (!strcmp(PLANE, polygon))
            model_plane_write(filepath, strtof(argv[2], nullptr), strtoul(argv[3], nullptr, 10));
        if (!strcmp(CUBE, polygon)) model_cube_write(filepath, atof(argv[2]), strtoul(argv[3], nullptr, 10));
        if (!strcmp(CONE, polygon))
            model_cone_write(filepath, atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[5]));
        if (!strcmp(SPHERE, polygon)) model_sphere_write(filepath, atof(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    return 1;
}