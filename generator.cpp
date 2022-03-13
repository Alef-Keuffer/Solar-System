#define _USE_MATH_DEFINES

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SPHERE "sphere"
#define CUBE "box"
#define CONE "cone"
#define PLANE "plane"

static FILE *globalFD;

void render3d(const char *path) {
    FILE *fp;
    float p[3];
    fp = fopen(path, "r");

    // float *q = malloc(sizeof(float));

    while (fread(p, sizeof(float), 3, fp))
        printf("%f %f %f\n", p[0], p[1], p[2]);
    fclose(fp);
}

void writeVertex(float x, float y, float z) {
    /*writes vertex (x,y,z) to globalFD*/
    float p[3] = {x, y, z};
    fwrite(p, sizeof(float), 3, globalFD);
}

void cylinderVertex(float radius, float height, float theta) {
    writeVertex(radius * sin(theta), height, radius * cos(theta));
}

void coneVertex(float r, float height, float theta, float h) {
    writeVertex(r * h * cos(theta), 2 * (height + h), r * h * sin(theta));
}

void drawCone(float r, float height, unsigned int slices, unsigned int stacks) {
    // https://www.math3d.org/5gLCN9yBz

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = height / (float) stacks;
    float theta = -M_PI;
    float h = -height;

    unsigned int nVertices = slices * stacks * 9;
    fwrite(&nVertices, sizeof(unsigned int), 1, globalFD);

    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            //base
            writeVertex(0, 0, 0); //O
            coneVertex(r, height, theta + s * (i - 1), h); //P1
            coneVertex(r, height, theta + s * i, h); //P2

            coneVertex(r, height, theta + s * i, h + t * (j - 1)); // P2
            coneVertex(r, height, theta + s * (i - 1), h + t * j); // P1'
            coneVertex(r, height, theta + s * i, h + t * j); //P2'

            coneVertex(r, height, theta + s * i, h + t * (j - 1)); // P2
            coneVertex(r, height, theta + s * (i - 1), h + t * (j - 1)); // P1
            coneVertex(r, height, theta + s * (i - 1), h + t * j); // P1'
        }
    }
}

void sphereVertex(float r, float theta, float phi) {
    writeVertex(r * sin(theta) * cos(phi), r * sin(phi), r * cos(theta) * cos(phi));
}

void drawSphere(float r, unsigned int slices, unsigned int stacks) {
    // https://www.math3d.org/EumEEZBKe
    // https://www.math3d.org/zE4n6xayX

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = M_PI / (float) stacks;
    float theta = -M_PI;
    float phi = -M_PI / 2.0f;

    unsigned int nVertices = slices * stacks * 6;
    fwrite(&nVertices, sizeof(unsigned int), 1, globalFD);

    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            sphereVertex(r, theta + s * (i - 1), phi + t * j); // P1'
            sphereVertex(r, theta + s * i, phi + t * (j - 1)); // P2
            sphereVertex(r, theta + s * i, phi + t * j); //P2'

            sphereVertex(r, theta + s * (i - 1), phi + t * (j - 1)); // P1
            sphereVertex(r, theta + s * i, phi + t * (j - 1)); // P2
            sphereVertex(r, theta + s * (i - 1), phi + t * j); // P1'
        }
    }

}

void drawPlane(float length, unsigned int divisions) {
    float o = -length / 2.0f;
    float d = length / (float) divisions;

    unsigned int nVertices = divisions * divisions * 12;
    fwrite(&nVertices, sizeof(unsigned int), 1, globalFD);

    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            writeVertex(o + d * (i - 1), 0, o + d * (j - 1)); //P1
            writeVertex(o + d * (i - 1), 0, o + d * j); //P1'z
            writeVertex(o + d * i, 0, o + d * (j - 1)); //P1'x

            writeVertex(o + d * i, 0, o + d * (j - 1)); //P1'x
            writeVertex(o + d * (i - 1), 0, o + d * j); //P1'z
            writeVertex(o + d * i, 0, o + d * j); //P2

            /*Cull face*/
            writeVertex(o + d * (i - 1), 0, o + d * j); //P1'z
            writeVertex(o + d * (i - 1), 0, o + d * (j - 1)); //P1
            writeVertex(o + d * i, 0, o + d * (j - 1)); //P1'x

            writeVertex(o + d * (i - 1), 0, o + d * j); //P1'z
            writeVertex(o + d * i, 0, o + d * (j - 1)); //P1'x
            writeVertex(o + d * i, 0, o + d * j); //P2
        }
    }
}

void drawCube(float length, unsigned int divisions) {
    float o = -length / 2.0f;
    float d = length / (float) divisions;

    unsigned int nVertices = divisions * divisions * 36;
    fwrite(&nVertices, sizeof(unsigned int), 1, globalFD);

    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            // top
            writeVertex(o + d * (i - 1), -o, o + d * (j - 1)); //P1
            writeVertex(o + d * (i - 1), -o, o + d * j); //P1'z
            writeVertex(o + d * i, -o, o + d * (j - 1)); //P1'x

            writeVertex(o + d * i, -o, o + d * (j - 1)); //P1'x
            writeVertex(o + d * (i - 1), -o, o + d * j); //P1'z
            writeVertex(o + d * i, -o, o + d * j); //P2

            // bottom
            writeVertex(o + d * (i - 1), o, o + d * j); //P1'z
            writeVertex(o + d * (i - 1), o, o + d * (j - 1)); //P1
            writeVertex(o + d * i, o, o + d * (j - 1)); //P1'x

            writeVertex(o + d * (i - 1), o, o + d * j); //P1'z
            writeVertex(o + d * i, o, o + d * (j - 1)); //P1'x
            writeVertex(o + d * i, o, o + d * j); //P2

            // left
            writeVertex(o, o + d * (i - 1), o + d * (j - 1)); //P1
            writeVertex(o, o + d * (i - 1), o + d * j); //P1'z
            writeVertex(o, o + d * i, o + d * (j - 1)); //P1'x

            writeVertex(o, o + d * i, o + d * (j - 1)); //P1'x
            writeVertex(o, o + d * (i - 1), o + d * j); //P1'z
            writeVertex(o, o + d * i, o + d * j); //P2

            // right
            writeVertex(-o, o + d * (i - 1), o + d * j); //P1'z
            writeVertex(-o, o + d * (i - 1), o + d * (j - 1)); //P1
            writeVertex(-o, o + d * i, o + d * (j - 1)); //P1'x

            writeVertex(-o, o + d * (i - 1), o + d * j); //P1'z
            writeVertex(-o, o + d * i, o + d * (j - 1)); //P1'x
            writeVertex(-o, o + d * i, o + d * j); //P2

            // front
            writeVertex(o + d * (i - 1), o + d * (j - 1), o); //P1
            writeVertex(o + d * (i - 1), o + d * j, o); //P1'z
            writeVertex(o + d * i, o + d * (j - 1), o); //P1'x

            writeVertex(o + d * i, o + d * (j - 1), o); //P1'x
            writeVertex(o + d * (i - 1), o + d * j, o); //P1'z
            writeVertex(o + d * i, o + d * j, o); //P2

            // back
            writeVertex(o + d * (i - 1), o + d * j, -o); //P1'z
            writeVertex(o + d * (i - 1), o + d * (j - 1), -o); //P1
            writeVertex(o + d * i, o + d * (j - 1), -o); //P1'x

            writeVertex(o + d * (i - 1), o + d * j, -o); //P1'z
            writeVertex(o + d * i, o + d * (j - 1), -o); //P1'x
            writeVertex(o + d * i, o + d * j, -o); //P2
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) { render3d("box.3d"); }
    else {
        globalFD = fopen(argv[argc - 1], "w");

        if (!strcmp(SPHERE, argv[1])) drawSphere(atof(argv[2]), atoi(argv[3]), atoi(argv[4]));
        if (!strcmp(CUBE, argv[1])) drawCube(atof(argv[2]), atoi(argv[3]));
        if (!strcmp(CONE, argv[1])) drawCone(atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[5]));
        if (!strcmp(PLANE, argv[1])) drawPlane(atof(argv[2]), atoi(argv[3]));
        fclose(globalFD);
    }
    return 1;
}