#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

#include <tinyxml2.h>
#include <stdio.h>
#include <vector>

/*rotation*/
static float globalAngle = 0;
static float globalRotateX = 0;
static float globalRotateY = 0;
static float globalRotateZ = 0;

/*translation*/
static float globalTranslateX = 0;
static float globalTranslateY = 0;
static float globalTranslateZ = 0;

/*scaling*/
static float globalScaleX = 1;
static float globalScaleY = 1;
static float globalScaleZ = 1;

/*camera*/
#define DEFAULT_GLOBAL_EYE_STEP 1.0f
#define DEFAULT_GLOBAL_EYE_X 1.0f
#define DEFAULT_GLOBAL_EYE_Y (-5.0f)
#define DEFAULT_GLOBAL_EYE_Z (-30.0f)

#define DEFAULT_GLOBAL_CENTER_STEP 1.0f
#define DEFAULT_GLOBAL_CENTER_X 0.0f
#define DEFAULT_GLOBAL_CENTER_Y 0.0f
#define DEFAULT_GLOBAL_CENTER_Z 0.0f

#define DEFAULT_GLOBAL_UP_STEP 1.0f
#define DEFAULT_GLOBAL_UP_X 0.0f
#define DEFAULT_GLOBAL_UP_Y 1.0f
#define DEFAULT_GLOBAL_UP_Z 0.0f

static float globalEyeX = DEFAULT_GLOBAL_EYE_X;
static float globalEyeY = DEFAULT_GLOBAL_EYE_Y;
static float globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

static float globalCenterX = DEFAULT_GLOBAL_CENTER_X;
static float globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
static float globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

static float globalUpX = DEFAULT_GLOBAL_UP_X;
static float globalUpY = DEFAULT_GLOBAL_UP_Y;
static float globalUpZ = DEFAULT_GLOBAL_UP_Z;

static float globalFOV = 45.0f;
static float globalNear = 1.0f;
static float globalFar = 1000.0f;

typedef struct model {
    float *vertices;
    unsigned int nVertices;
} *Model;

static std::vector<Model> globalModels;

Model allocModel(const char *path) {
    FILE *fp = fopen(path, "r");

    unsigned int nVertices;
    fread(&nVertices, sizeof(unsigned int), 1, fp);
    float *modelBuf = (float *) malloc(3 * nVertices * sizeof(float));

    fread(modelBuf, 3 * sizeof(float), nVertices, fp);
    fclose(fp);


    Model model = (Model) malloc(sizeof(Model));

    model->nVertices = nVertices;
    model->vertices = modelBuf;

    return model;
}

void renderModel(Model model) {
    if (!model->nVertices % 3) {
        fprintf(stderr, "Number of coordinates is not divisible by 3");
        exit(-1);
    }
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < 3 * model->nVertices; i += 3)
        glVertex3f(model->vertices[i], model->vertices[i + 1], model->vertices[i + 2]);
    glEnd();
}

void renderAllModels() {
    for (Model m: globalModels) renderModel(m);
}

size_t readModelToBuffer(const char *path, float *p, unsigned int n) {
    FILE *fp = fopen(path, "r");
    size_t r = fread(p, sizeof(float), n, fp);
    fclose(fp);
    return r;
}

void renderTriangleVertexSeq(float *p, unsigned int nVertices) {
    if (!nVertices % 3) {
        fprintf(stderr, "Number of coordinates is not divisible by 3");
        exit(-1);
    }
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < nVertices; i += 3)
        glVertex3f(p[i], p[i + 1], p[i + 2]);
    glEnd();
}

void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = (float) w * 1.0f / (float) h;

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(globalFOV, ratio, globalNear, globalFar); //fox,near,far

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

void renderScene() {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    // helps in avoiding rounding mistakes, discards old matrix, read more

    gluLookAt(globalEyeX, globalEyeY, globalEyeZ,
              globalCenterX, globalCenterY, globalCenterZ,
              globalUpX, globalUpY, globalUpZ);
    // put the geometric transformations here
    glRotatef(globalAngle, globalRotateX, globalRotateY, globalRotateZ);
    glTranslatef(globalTranslateX, globalTranslateY, globalTranslateZ);
    glScalef(globalScaleX, globalScaleY, globalScaleZ);

    renderAllModels();

    // End of frame
    glutSwapBuffers();
}

int load_xml(FILE *xmlFILE) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xmlFILE);

    globalEyeX = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "position")->FloatAttribute("x");
    globalEyeY = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "position")->FloatAttribute("y");
    globalEyeZ = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "position")->FloatAttribute("z");

    globalCenterX = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "lookAt")->FloatAttribute("x");
    globalCenterY = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "lookAt")->FloatAttribute("y");
    globalCenterZ = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "lookAt")->FloatAttribute("z");

    globalUpX = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "up")->FloatAttribute("x");
    globalUpY = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "up")->FloatAttribute("y");
    globalUpZ = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "up")->FloatAttribute("z");
    globalFOV = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "projection")->FloatAttribute("fov");
    globalNear = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "projection")->FloatAttribute("near");
    globalFar = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "projection")->FloatAttribute("far");

    globalModels.push_back(allocModel(
            doc.FirstChildElement("world")->FirstChildElement("group")->FirstChildElement(
                    "models")->FirstChildElement("model")->Attribute("file")));

    tinyxml2::XMLElement *model2 = doc.FirstChildElement("world")->FirstChildElement("group")->FirstChildElement(
            "models")->FirstChildElement("model")->NextSiblingElement("model");

    if (model2) globalModels.push_back(allocModel(model2->Attribute("file")));

    return 1;
}

int main(int argc, char **argv) {

    if (argc > 0) {
        FILE *xmlFILE = fopen(argv[1], "r");
        load_xml(xmlFILE);
        fclose(xmlFILE);
    }

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("engine");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT, GL_LINE);

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
