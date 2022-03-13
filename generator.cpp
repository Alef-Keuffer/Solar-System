#define _USE_MATH_DEFINES

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SPHERE "sphere"
#define CUBE "box"
#define CONE "cone"
#define PLANE "plane"

void points_vertex(float x, float y, float z, float *points) {
    points[0] = x;
    points[1] = y;
    points[2] = z;
}

void points_write(const char *filename, const unsigned int nFloats, float points[]) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "failed to open file");
        exit(-1);
    }

    fwrite(&nFloats, sizeof(unsigned int), 1, fp);

    fwrite(points, sizeof(float), nFloats, fp);
    fclose(fp);
}

void model_plane_vertices(float length, unsigned int divisions, float *points) {
    float o = -length / 2.0f;
    float d = length / (float) divisions;

    unsigned int pos = 0;

    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            points_vertex(o + d * (i - 1), 0, o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(o + d * (i - 1), 0, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, 0, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(o + d * i, 0, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(o + d * (i - 1), 0, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, 0, o + d * j, points + (pos += 3)); //P2

            /*Cull face*/
            points_vertex(o + d * (i - 1), 0, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * (i - 1), 0, o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(o + d * i, 0, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(o + d * (i - 1), 0, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, 0, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(o + d * i, 0, o + d * j, points + (pos += 3)); //P2
        }
    }
}

static inline unsigned int model_plane_size(unsigned int divisions) { return divisions * divisions * 12 * 3 + 1; }

void model_plane_write(const char *filepath, float length, unsigned int divisions) {
    const unsigned int n = model_plane_size(divisions);
    float points[n];
    model_plane_vertices(length, divisions, points);
    points_write(filepath, n, points);
}

void model_cube_vertices(float length, unsigned int divisions, float points[]) {
    float o = -length / 2.0f;
    float d = length / (float) divisions;

    unsigned int pos = 0;

    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            // top
            points_vertex(o + d * (i - 1), -o, o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(o + d * (i - 1), -o, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, -o, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(o + d * i, -o, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(o + d * (i - 1), -o, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, -o, o + d * j, points + (pos += 3)); //P2

            // bottom
            points_vertex(o + d * (i - 1), o, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * (i - 1), o, o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(o + d * i, o, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(o + d * (i - 1), o, o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, o, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(o + d * i, o, o + d * j, points + (pos += 3)); //P2

            // left
            points_vertex(o, o + d * (i - 1), o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(o, o + d * (i - 1), o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o, o + d * i, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(o, o + d * i, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(o, o + d * (i - 1), o + d * j, points + (pos += 3)); //P1'z
            points_vertex(o, o + d * i, o + d * j, points + (pos += 3)); //P2

            // right
            points_vertex(-o, o + d * (i - 1), o + d * j, points + (pos += 3)); //P1'z
            points_vertex(-o, o + d * (i - 1), o + d * (j - 1), points + (pos += 3)); //P1
            points_vertex(-o, o + d * i, o + d * (j - 1), points + (pos += 3)); //P1'x

            points_vertex(-o, o + d * (i - 1), o + d * j, points + (pos += 3)); //P1'z
            points_vertex(-o, o + d * i, o + d * (j - 1), points + (pos += 3)); //P1'x
            points_vertex(-o, o + d * i, o + d * j, points + (pos += 3)); //P2

            // front
            points_vertex(o + d * (i - 1), o + d * (j - 1), o, points + (pos += 3)); //P1
            points_vertex(o + d * (i - 1), o + d * j, o, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, o + d * (j - 1), o, points + (pos += 3)); //P1'x

            points_vertex(o + d * i, o + d * (j - 1), o, points + (pos += 3)); //P1'x
            points_vertex(o + d * (i - 1), o + d * j, o, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, o + d * j, o, points + (pos += 3)); //P2

            // back
            points_vertex(o + d * (i - 1), o + d * j, -o, points + (pos += 3)); //P1'z
            points_vertex(o + d * (i - 1), o + d * (j - 1), -o, points + (pos += 3)); //P1
            points_vertex(o + d * i, o + d * (j - 1), -o, points + (pos += 3)); //P1'x

            points_vertex(o + d * (i - 1), o + d * j, -o, points + (pos += 3)); //P1'z
            points_vertex(o + d * i, o + d * (j - 1), -o, points + (pos += 3)); //P1'x
            points_vertex(o + d * i, o + d * j, -o, points + (pos += 3)); //P2
        }
    }
}

unsigned int model_cube_size(const unsigned int divisions) { return divisions * divisions * 36 * 3; }

void model_cube_write(const char *filepath, const float length, const unsigned int divisions) {
    const unsigned int n = divisions * divisions * 36 * 3;
    float points[n];
    model_cube_vertices(length, divisions, points);
    points_write(filepath, n, points);
}

static inline void model_cone_vertex(float r, float height, float theta, float h, float *points) {
    points_vertex(r * h * cos(theta), 2 * (height + h), r * h * sin(theta), points);
}

void model_cone_vertices(const float r, const float height, const unsigned int slices, const unsigned int stacks,
                         float points[]) {
    // https://www.math3d.org/5gLCN9yBz

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = height / (float) stacks;
    float theta = -M_PI;
    float h = -height;

    unsigned int pos = 0;

    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            //base
            points_vertex(0, 0, 0, points + (pos += 3)); //O
            model_cone_vertex(r, height, theta + s * (i - 1), h, points + (pos += 3)); //P1
            model_cone_vertex(r, height, theta + s * i, h, points + (pos += 3)); //P2

            model_cone_vertex(r, height, theta + s * i, h + t * (j - 1), points + (pos += 3)); // P2
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * j, points + (pos += 3)); // P1'
            model_cone_vertex(r, height, theta + s * i, h + t * j, points + (pos += 3)); //P2'

            model_cone_vertex(r, height, theta + s * i, h + t * (j - 1), points + (pos += 3)); // P2
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * (j - 1), points + (pos += 3)); // P1
            model_cone_vertex(r, height, theta + s * (i - 1), h + t * j, points + (pos += 3)); // P1'
        }
    }
}

static inline unsigned int model_cone_size(unsigned int stacks, unsigned int slices) { return slices * stacks * 9; };

void model_cone_write(const char *filepath, const float radius, const float height, const unsigned int slices,
                      const unsigned int stacks) {
    unsigned int n = model_cone_size(stacks, slices);
    float points[n];
    model_cone_vertices(radius, height, slices, stacks, points);
    points_write(filepath, n, points);
}


static inline void model_sphere_vertex(float r, float theta, float phi, float *points) {
    points_vertex(r * sin(theta) * cos(phi), r * sin(phi), r * cos(theta) * cos(phi), points);
}

static void model_sphere_vertices(float r, unsigned int slices, unsigned int stacks, float points[]) {
    // https://www.math3d.org/EumEEZBKe
    // https://www.math3d.org/zE4n6xayX

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = M_PI / (float) stacks;
    float theta = -M_PI;
    float phi = -M_PI / 2.0f;

    unsigned int pos = 0;

    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            model_sphere_vertex(r, theta + s * (i - 1), phi + t * j, points + (pos += 3)); // P1'
            model_sphere_vertex(r, theta + s * i, phi + t * (j - 1), points + (pos += 3)); // P2
            model_sphere_vertex(r, theta + s * i, phi + t * j, points + (pos += 3)); //P2'

            model_sphere_vertex(r, theta + s * (i - 1), phi + t * (j - 1), points + (pos += 3)); // P1
            model_sphere_vertex(r, theta + s * i, phi + t * (j - 1), points + (pos += 3)); // P2
            model_sphere_vertex(r, theta + s * (i - 1), phi + t * j, points + (pos += 3)); // P1'
        }
    }
}

static inline unsigned int model_sphere_size(unsigned int slices, unsigned int stacks) { return slices * stacks * 6; }

void model_sphere_write(const char *filepath, float radius, unsigned int slices, unsigned int stacks) {
    const unsigned int n = model_sphere_size(slices, stacks);
    float points[n];
    model_sphere_vertices(radius, slices, stacks, points);
    points_write(filepath, n, points);
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