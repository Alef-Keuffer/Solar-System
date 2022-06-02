/*! @addtogroup engine
 * @{*/

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glew.h>
#include <GL/glut.h>

#endif

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <tuple>
#include <map>
#include <string>

#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "parsing.h"
#include "curves.h"

using std::vector, std::tuple, std::map;
using glm::mat4, glm::vec4, glm::vec3, glm::cross, glm::value_ptr;
using std::cerr, std::endl, glm::to_string, std::string;

/*rotation*/
const unsigned int DEFAULT_GLOBAL_ANGLE_STEP = 16;
static float globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
static float globalAngle = 0;
static float globalRotateX = 0;
static float globalRotateY = 0;
static float globalRotateZ = 0;

/*translation*/
const unsigned int DEFAULT_GLOBAL_TRANSLATE_STEP = 1;
static float globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
static float globalTranslateX = 0;
static float globalTranslateY = 0;
static float globalTranslateZ = 0;

/*scaling*/
const unsigned int DEFAULT_GLOBAL_SCALE_STEP = 1;
float globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
static float globalScaleX = 1;
static float globalScaleY = 1;
static float globalScaleZ = 1;


/*! @addtogroup camera
 * @{*/

/*! @addtogroup position
 * @{*/
const unsigned int DEFAULT_GLOBAL_EYE_STEP = 1;
static float DEFAULT_GLOBAL_EYE_X = 0;
static float DEFAULT_GLOBAL_EYE_Y = 0;
static float DEFAULT_GLOBAL_EYE_Z = 0;

static double globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
static double globalEyeX = DEFAULT_GLOBAL_EYE_X;
static double globalEyeY = DEFAULT_GLOBAL_EYE_Y;
static double globalEyeZ = DEFAULT_GLOBAL_EYE_Z;
//!@} end of group position

/*! @addtogroup lookAt
 * @{*/
const unsigned int DEFAULT_GLOBAL_CENTER_STEP = 1.0;
static float DEFAULT_GLOBAL_CENTER_X = 0;
static float DEFAULT_GLOBAL_CENTER_Y = 0;
static float DEFAULT_GLOBAL_CENTER_Z = 0;

float globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
static float globalCenterX = DEFAULT_GLOBAL_CENTER_X;
static float globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
static float globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;
//!@} end of group lookAt

/*! @addtogroup up
 * @{*/
const unsigned int DEFAULT_GLOBAL_UP_STEP = 1;
static float DEFAULT_GLOBAL_UP_X = 0;
static float DEFAULT_GLOBAL_UP_Y = 1;
static float DEFAULT_GLOBAL_UP_Z = 0;

float globalUpStep = DEFAULT_GLOBAL_UP_STEP;
static float globalUpX = DEFAULT_GLOBAL_UP_X;
static float globalUpY = DEFAULT_GLOBAL_UP_Y;
static float globalUpZ = DEFAULT_GLOBAL_UP_Z;
//!@} end of group up

/*! @addtogroup projection
 * @{*/
static float DEFAULT_GLOBAL_FOV = 60;
static float DEFAULT_GLOBAL_NEAR = 1;
static float DEFAULT_GLOBAL_FAR = 1000;

static float globalFOV = DEFAULT_GLOBAL_FOV;
static float globalNear = DEFAULT_GLOBAL_NEAR;
static float globalFar = DEFAULT_GLOBAL_FAR;
//!@} end of group projection

/*! @addtogroup spherical
 * @{ */
double DEFAULT_GLOBAL_RADIUS = 1;
double DEFAULT_GLOBAL_AZIMUTH = 0;
double DEFAULT_GLOBAL_ELEVATION = 0;

double globalRadius = DEFAULT_GLOBAL_RADIUS;
double globalAzimuth = DEFAULT_GLOBAL_AZIMUTH;
double globalElevation = DEFAULT_GLOBAL_ELEVATION;
bool globalMouseLeftButton = false;

void
spherical2Cartesian (const double radius, const double elevation, const double azimuth, double *x, double *y, double *z)
{

  *x = radius * cos (elevation) * sin (azimuth) + globalCenterX;
  *y = radius * sin (elevation) + globalCenterY;
  *z = radius * cos (elevation) * cos (azimuth) + globalCenterZ;
}

void
cartesian2Spherical (const double x, const double y, const double z, double *radius, double *azimuth, double *elevation)
{
  *radius = abs (sqrt (pow (x, 2) + pow (y, 2) + pow (z, 2)));
  *elevation = asin (y / (*radius));
  *azimuth = asin (x / (*radius * cos (*elevation)));
}

//! @} end of group spherical
//! @} end of group camera

void env_load_defaults ()
{
  /*rotation*/
  globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
  globalAngle = 0;
  globalRotateX = 0;
  globalRotateY = 0;
  globalRotateZ = 0;

  /*translation*/
  globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
  globalTranslateX = 0;
  globalTranslateY = 0;
  globalTranslateZ = 0;

  /*scaling*/
  globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
  globalScaleX = 1;
  globalScaleY = 1;
  globalScaleZ = 1;

  /*position*/
  globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
  globalEyeX = DEFAULT_GLOBAL_EYE_X;
  globalEyeY = DEFAULT_GLOBAL_EYE_Y;
  globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

  globalRadius = DEFAULT_GLOBAL_RADIUS;
  globalAzimuth = DEFAULT_GLOBAL_AZIMUTH;
  globalElevation = DEFAULT_GLOBAL_ELEVATION;

  /*lookAt*/
  globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
  globalCenterX = DEFAULT_GLOBAL_CENTER_X;
  globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
  globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

  /*up*/

  globalUpStep = DEFAULT_GLOBAL_UP_STEP;
  globalUpX = DEFAULT_GLOBAL_UP_X;
  globalUpY = DEFAULT_GLOBAL_UP_Y;
  globalUpZ = DEFAULT_GLOBAL_UP_Z;

  /*projection*/
  globalFOV = DEFAULT_GLOBAL_FOV;
  globalNear = DEFAULT_GLOBAL_NEAR;
  globalFar = DEFAULT_GLOBAL_FAR;
}

//! @defgroup modelEngine Model

/*! @addtogroup modelEngine
 * @{*/

const auto RGB_MAX = 255.0;
struct model {
  GLsizei nVertices{};
  GLuint vbo{};
  GLuint normals{};
  struct {
    vec4 diffuse = {200.0 / RGB_MAX, 200.0 / RGB_MAX, 200.0 / RGB_MAX, 1};
    vec4 ambient{50.0 / RGB_MAX, 50.0 / RGB_MAX, 50.0 / RGB_MAX, 1};
    vec4 specular{0, 0, 0, 1};
    vec4 emissive{0, 0, 0, 1};
    GLfloat shininess = 0;
  } material{};
  // 0 default value means it's optional with 0 meaning it's not being used by a particular model.
  GLuint tbo = 0; // texture buffer object
  GLuint tc = 0; // texture coordinates
};

static std::vector<struct model> globalModels;
static std::vector<float> globalOperations;

struct model allocModel (const char *model3dFilePath)
{
  FILE *fp = fopen (model3dFilePath, "r");
  if (!fp)
    {
      fprintf (stderr, "failed to open model: %s\n", model3dFilePath);
      exit (EXIT_FAILURE);
    }

  cerr << "[allocModel] model file = " << model3dFilePath << endl;
  // read number of vertices
  GLsizei nVertices;
  fread (&nVertices, sizeof (nVertices), 1, fp);
  cerr << "[allocModel] nVertices = " << nVertices << endl;

  // read vertices
  auto *arrayOfVertices = (float *) malloc (3 * nVertices * sizeof (float));
  const size_t nVerticesRead = fread (arrayOfVertices, 3 * sizeof (float), nVertices, fp);
  if (nVerticesRead != nVertices)
    {
      fprintf (stderr, "%zu = nVerticesRead != nVertices = %ul", nVerticesRead, nVertices);
      exit (EXIT_FAILURE);
    }

  // read normals
  auto *arrayOfNormals = (float *) malloc (3 * nVertices * sizeof (float));
  const size_t nNormalsRead = fread (arrayOfNormals, 3 * sizeof (float), nVertices, fp);
  if (nNormalsRead != nVertices)
    {
      fprintf (stderr, "%zu = nNormalsRead != nVertices = %ul", nNormalsRead, nVertices);
      exit (EXIT_FAILURE);
    }

  // read texture coordinates
  auto *arrayOfTextureCoordinates = (float *) malloc (2 * nVertices * sizeof (float));
  const size_t nTextureCoordinatesRead = fread (arrayOfTextureCoordinates, 2 * sizeof (float), nVertices, fp);
  if (nTextureCoordinatesRead != nVertices)
    {
      fprintf (stderr, "%zu = nTextureCoordinatesRead != nVertices = %ul", nTextureCoordinatesRead, nVertices);
      exit (EXIT_FAILURE);
    }

  fclose (fp);

  struct model model;
  model.nVertices = nVertices;

  // vertices buffer object array
  const GLsizei sizeOfVertexArray = (GLsizei) sizeof (arrayOfVertices[0]) * 3 * model.nVertices;
  glGenBuffers (1, &model.vbo);
  glBindBuffer (GL_ARRAY_BUFFER, model.vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeOfVertexArray, arrayOfVertices, GL_STATIC_DRAW);
  free (arrayOfVertices);

  // normals buffer object array
  const GLsizei sizeOfNormalsArray = (GLsizei) sizeof (arrayOfNormals[0]) * 3 * model.nVertices;
  glGenBuffers (1, &model.normals);
  glBindBuffer (GL_ARRAY_BUFFER, model.normals);
  glBufferData (GL_ARRAY_BUFFER, sizeOfNormalsArray, arrayOfNormals, GL_STATIC_DRAW);
  free (arrayOfNormals);

  // texture coordinates buffer object array
  const GLsizei sizeOfTextureCoordinateArray = (GLsizei) sizeof (arrayOfVertices[0]) * 2 * model.nVertices;
  glGenBuffers (1, &model.tc);
  glBindBuffer (GL_ARRAY_BUFFER, model.tc);
  glBufferData (GL_ARRAY_BUFFER, sizeOfTextureCoordinateArray, arrayOfTextureCoordinates, GL_STATIC_DRAW);
  free (arrayOfTextureCoordinates);

  // unbind array buffer
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  return model;
}

void addTexture (struct model &m, const char *const path)
{

  static bool isFirstTimeBeingExecuted = true;
  if (isFirstTimeBeingExecuted)
    {
      // DevIL setup - done once (slide 5) [class11]
      ilInit ();
      ilEnable (IL_ORIGIN_SET);
      ilOriginFunc (IL_ORIGIN_LOWER_LEFT);
    }

  // for each image (slide 5) [class11]
  ILuint image;
  ilGenImages (1, &image);
  ilBindImage (image);
  ilLoadImage ((ILstring) path);

  // convert to RGBA (slide 6) [class11]
  ilConvertImage (IL_RGBA, IL_UNSIGNED_BYTE);

  // texture creation in OpenGL (slide 8) [class11]
  // create a texture slot (slide 8) [class11]
  glGenTextures (1, &m.tbo);

  // bind the slot (slide 8) [class11]
  glBindTexture (GL_TEXTURE_2D, m.tbo);

  // define texture parameters (slide 8) [class11]
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  // get the required info (slide 7) [class11]
  const ILubyte *const texData = ilGetData ();
  const GLsizei texture_width = ilGetInteger (IL_IMAGE_WIDTH);
  const GLsizei texture_height = ilGetInteger (IL_IMAGE_HEIGHT);

  // send texture data to OpenGL (slide 8) [class11]
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA,
                texture_width, texture_height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, texData);

  glGenerateMipmap (GL_TEXTURE_2D);

  // unbind texture
  glBindTexture (GL_TEXTURE_2D, 0);

  isFirstTimeBeingExecuted = false;
}

void renderModel (const struct model &model)
{
  if (!model.nVertices % 3)
    {
      fprintf (stderr, "Number of coordinates is not divisible by 3");
      exit (1);
    }

  // vertex buffer object (slide 14) [class11]
  glBindBuffer (GL_ARRAY_BUFFER, model.vbo);
  glVertexPointer (3, GL_FLOAT, 0, nullptr);

  // normals (slide 14) [class11]
  glBindBuffer (GL_ARRAY_BUFFER, model.normals);
  glNormalPointer (GL_FLOAT, 0, nullptr);

  // texture coordinates (slide 14) [class11]
  glBindBuffer (GL_ARRAY_BUFFER, model.tc);
  glTexCoordPointer (2, GL_FLOAT, 0, nullptr);

  // texture buffer object (slide 14) [class11]
  glBindTexture (GL_TEXTURE_2D, model.tbo);

  // define a material for the object(s) (slide 8) [class9]
  glMaterialfv (GL_FRONT, GL_DIFFUSE, value_ptr (model.material.diffuse));
  glMaterialfv (GL_FRONT, GL_AMBIENT, value_ptr (model.material.ambient));
  glMaterialfv (GL_FRONT, GL_SPECULAR, value_ptr (model.material.specular));
  glMaterialfv (GL_FRONT, GL_EMISSION, value_ptr (model.material.emissive));
  glMaterialf (GL_FRONT, GL_SHININESS, model.material.shininess);

  // drawing
  glDrawArrays (GL_TRIANGLES, 0, model.nVertices);

  // unbind array buffer
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  // unbind texture (slide 10) [class11]
  glBindTexture (GL_TEXTURE_2D, 0);
}

//!@} end of group modelEngine

void changeSize (const int w, int h)
{

  // Prevent a divide by zero, when window is too short
  // (you can't make a window with zero width).
  if (h == 0)
    h = 1;

  // compute window's aspect ratio
  float ratio = (float) w * 1.0f / (float) h;

  // Set the projection matrix as current
  glMatrixMode (GL_PROJECTION);

  // Load Identity Matrix
  glLoadIdentity ();

  // Set the viewport to be the entire window
  glViewport (0, 0, w, h);

  // Set perspective
  gluPerspective (globalFOV, ratio, globalNear, globalFar); //fox,near,far

  // return to the model view matrix mode
  glMatrixMode (GL_MODELVIEW);
}
//!@} end of group engine

void advance_in_rotation (const float rotation_time, const vec3 &axis_of_rotation)
{
  const float gt = fmodf ((float) glutGet (GLUT_ELAPSED_TIME), (float) (rotation_time * 1000)) / (rotation_time * 1000);
  const float angle = 360 * gt;
  glRotatef (angle, axis_of_rotation.x, axis_of_rotation.y, axis_of_rotation.z);
}

//! @ingroup Operations
void operations_render (vector<float> &operations)
{
  unsigned int i = 0;
  static bool isFirstTimeBeingExecuted = true;
  static bool hasPushedModels = false;
  static bool hasLoadedCurves = false;
  static unsigned char nLights = 0;
  assert (nLights < 8);
  static vector<vector<vec3>> curves;

  DEFAULT_GLOBAL_EYE_X = operations[i];
  DEFAULT_GLOBAL_EYE_Y = operations[i + 1];
  DEFAULT_GLOBAL_EYE_Z = operations[i + 2];

  cartesian2Spherical (
      DEFAULT_GLOBAL_EYE_X, DEFAULT_GLOBAL_EYE_Y, DEFAULT_GLOBAL_EYE_Z,
      &DEFAULT_GLOBAL_RADIUS, &DEFAULT_GLOBAL_AZIMUTH, &DEFAULT_GLOBAL_ELEVATION);
  i += 3;

  DEFAULT_GLOBAL_CENTER_X = operations[i];
  DEFAULT_GLOBAL_CENTER_Y = operations[i + 1];
  DEFAULT_GLOBAL_CENTER_Z = operations[i + 2];
  i += 3;

  DEFAULT_GLOBAL_UP_X = operations[i];
  DEFAULT_GLOBAL_UP_Y = operations[i + 1];
  DEFAULT_GLOBAL_UP_Z = operations[i + 2];
  i += 3;

  DEFAULT_GLOBAL_FOV = operations[i];
  DEFAULT_GLOBAL_NEAR = operations[i + 1];
  DEFAULT_GLOBAL_FAR = operations[i + 2];

  unsigned int model_num = 0;

  for (i += 3; i < operations.size (); i++)
    {
      switch ((int) operations[i])
        {
          // transformations
          case ROTATE:
            {
              const float rotation_angle = operations[i + 1];
              const vec3 axis_of_rotation (operations[i + 2],
                                           operations[i + 3],
                                           operations[i + 4]);
              glRotatef (operations[i + 1],
                         operations[i + 2],
                         operations[i + 3],
                         operations[i + 4]);
              if (isFirstTimeBeingExecuted)
                cerr << "ROTATE (" << "rotation_angle:" << rotation_angle
                     << ", axis of rotatation: " << to_string (axis_of_rotation) << ")" << endl;
              i += 4; //angle, axis_of_rotation}
            }
          continue;
          case EXTENDED_ROTATE:
            {
              const float rotation_time = operations[i + 1];
              const vec3 axis_of_rotation = {
                  operations[i + 2],
                  operations[i + 3],
                  operations[i + 4]
              };
              advance_in_rotation (rotation_time, axis_of_rotation);
              i += 4; //time, axis_of_rotation
              if (isFirstTimeBeingExecuted)
                cerr << "EXTENDED_ROTATE (rotation_time: " << rotation_time
                     << " seconds, axis_of_rotation: " << to_string (axis_of_rotation) << ")" << endl;
            }
          continue;
          case TRANSLATE:
            {
              const vec3 translation (operations[i + 1],
                                      operations[i + 2],
                                      operations[i + 3]);
              glTranslatef (translation[0],
                            translation[1],
                            translation[2]);
              if (isFirstTimeBeingExecuted)
                cerr << "TRANSLATE (" << to_string (translation) << ")" << endl;
              i += 3;
            }
          continue;
          case EXTENDED_TRANSLATE:
            {
              const int number_of_points = (int) operations[i + 3];
              if (!hasLoadedCurves)
                {
                  vector<vec3> new_curve (number_of_points);
                  for (int j = 0; j < number_of_points; ++j)
                    {
                      const int idx = 3 * j;
                      new_curve.at (j)[0] = operations[i + 4 + idx];
                      new_curve.at (j)[1] = operations[i + 4 + idx + 1];
                      new_curve.at (j)[2] = operations[i + 4 + idx + 2];
                    }
                  curves.push_back (new_curve);
                }
              auto curve = curves.back ();
              renderCurve (Mcr, curve);
              const float translation_time = operations[i + 1];
              const bool align = (bool) operations[i + 2];
              advance_in_curve (translation_time, align, Mcr, curves.back ());
              if (isFirstTimeBeingExecuted)
                cerr << "EXTENDED_TRANSLATE ("
                     << "translation_time: " << translation_time
                     << ", align: " << align
                     << ", number of points: " << number_of_points
                     << ")" << endl;
              i += 3 + number_of_points * 3; // 3 - time,align,number_of_points,points...
            }
          continue;
          case SCALE:
            {
              const vec3 scale (operations[i + 1],
                                operations[i + 2],
                                operations[i + 3]);
              glScalef (scale[0],
                        scale[1],
                        scale[2]);
              i += 3;
            }
          continue;
          // grouping
          case BEGIN_GROUP:
            {
              if (isFirstTimeBeingExecuted)
                cerr << "BEGIN_GROUP" << endl;
              glPushMatrix ();
            }
          continue;
          case END_GROUP:
            {
              if (isFirstTimeBeingExecuted)
                cerr << "END_GROUP" << endl;
              glPopMatrix ();
            }
          continue;
          // texture
          case TEXTURE:
            {
              const int stringSize = (int) operations[i + 1];
              if (!hasPushedModels)
                {
                  char textureFilePath[stringSize + 1];
                  int j;
                  for (j = 0; j < stringSize; ++j)
                    textureFilePath[j] = (char) operations[i + 2 + j];
                  textureFilePath[j] = '\0';
                  addTexture (globalModels.back (), textureFilePath);
                  if (isFirstTimeBeingExecuted)
                    cerr << "TEXTURE (" << textureFilePath << ")" << endl;
                }
              i += stringSize + 1; //just to be explicit
            }
          continue;
          // object material components
          case DIFFUSE:
            {
              auto &diffuse = globalModels.back ().material.diffuse;
              diffuse[0] = operations[i + 1];
              diffuse[1] = operations[i + 2];
              diffuse[2] = operations[i + 3];
              if (isFirstTimeBeingExecuted)
                cerr << "DIFFUSE (" << to_string (diffuse) << ")" << endl;
              i += 3;
            }
          continue;
          case AMBIENT:
            {
              auto &ambient = globalModels.back ().material.ambient;
              ambient[0] = operations[i + 1];
              ambient[1] = operations[i + 2];
              ambient[2] = operations[i + 3];
              if (isFirstTimeBeingExecuted)
                cerr << "AMBIENT (" << to_string (ambient) << ")" << endl;
              i += 3;
            }
          continue;
          case SPECULAR:
            {
              auto &specular = globalModels.back ().material.specular;
              specular[0] = operations[i + 1];
              specular[1] = operations[i + 2];
              specular[2] = operations[i + 3];
              if (isFirstTimeBeingExecuted)
                cerr << "SPECULAR (" << to_string (specular) << ")" << endl;
              i += 3;
            }
          continue;
          case EMISSIVE:
            {
              auto &emissive = globalModels.back ().material.emissive;
              emissive[0] = operations[i + 1];
              emissive[1] = operations[i + 2];
              emissive[2] = operations[i + 3];
              if (isFirstTimeBeingExecuted)
                cerr << "EMISSIVE (" << to_string (emissive) << ")" << endl;
              i += 3;
            }
          continue;
          case SHININESS:
            {
              float shininess = (globalModels.back ().material.shininess = operations[i + 1]);
              if (isFirstTimeBeingExecuted)
                cerr << "SHININESS (" << shininess << ")" << endl;
              i += 1;
            }
          continue;
          // model
          case BEGIN_MODEL:
            {
              int stringSize = (int) operations[i + 1];
              if (!hasPushedModels)
                {
                  char modelName[stringSize + 1];
                  int j;
                  for (j = 0; j < stringSize; ++j)
                    modelName[j] = (char) operations[i + 2 + j];
                  modelName[j] = '\0';

                  globalModels.push_back (allocModel (modelName));
                  if (isFirstTimeBeingExecuted)
                    cerr << "BEGIN_MODEL (" << modelName << ")" << endl;
                }
              ++model_num;
              i += stringSize + 1; //just to be explicit
            }
          continue;
          case END_MODEL:
            {
              if (isFirstTimeBeingExecuted)
                cerr << "END_MODEL" << endl;
              renderModel (globalModels.back ());
            }
          continue;
          // light sources
          case POINT:
            {
              const vec4 pos = {operations[i + 1],
                                operations[i + 2],
                                operations[i + 3],
                                0.0};
              if (isFirstTimeBeingExecuted)
                cerr << "POINT (" << to_string (pos) << ")" << endl;
              glLightfv (GL_LIGHT0 + nLights, GL_POSITION, value_ptr (pos));
              if (isFirstTimeBeingExecuted)
                ++nLights;
              i += 3;
            }
          continue;
          case DIRECTIONAL:
            {
              const vec4 dir = {operations[i + 1],
                                operations[i + 2],
                                operations[i + 3],
                                0.0};
              if (isFirstTimeBeingExecuted)
                cerr << "DIRECTIONAL (" << to_string (dir) << ")" << endl;

              glLightfv (GL_LIGHT0 + nLights, GL_POSITION, value_ptr (dir));
              if (isFirstTimeBeingExecuted)
                ++nLights;
              i += 3;
            }
          continue;
          case SPOTLIGHT:
            {
              const vec4 pos = {operations[i + 1],
                                operations[i + 2],
                                operations[i + 3],
                                1.0};
              const vec4 dir = {operations[i + 4],
                                operations[i + 5],
                                operations[i + 6],
                                1.0};
              const float cutoff = operations[i + 7];
              if (isFirstTimeBeingExecuted)
                cerr << "SPOTLIGHT:"
                        "\n\t(pos: " << to_string (pos) << ")"
                                                           "\n\t(dir: " << to_string (dir) << ")"
                                                                                              "\n\t(cutoff: " << cutoff
                     << ")" << endl;
              glLightfv (GL_LIGHT0 + nLights, GL_POSITION, value_ptr (pos));
              if (isFirstTimeBeingExecuted)
                ++nLights;
              glLightfv (GL_LIGHT0 + nLights, GL_SPOT_DIRECTION, value_ptr (dir));
              if (isFirstTimeBeingExecuted)
                ++nLights;
              glLightf (GL_LIGHT0 + nLights, GL_SPOT_CUTOFF, cutoff);
              if (isFirstTimeBeingExecuted)
                ++nLights;
              i += 7;
              continue;
            }
        }
    }
  hasPushedModels = true;
  hasLoadedCurves = true;
  isFirstTimeBeingExecuted = false;
}

void draw_axes ()
{
  /*draw absolute (before any transformation) axes*/
  glBegin (GL_LINES);
  /*X-axis in red*/
  glColor3f (1, 0, 0);
  glVertex3f (100, 0, 0);
  glVertex3f (-100, 0, 0);

  /*Y-Axis in Green*/
  glColor3f (0, 1, 0);
  glVertex3f (0, 100, 0);
  glVertex3f (0, -100, 0);

  /*Z-Axis in Blue*/
  glColor3f (0, 0, 1);
  glVertex3f (0, 0, 100);
  glVertex3f (0, 0, -100);
  glColor3d (1, 1, 1);
  glEnd ();
  /*end of draw absolute (before any transformation) axes*/
}

/*!@addtogroup engine
 * @{*/
void redisplay ()
{
  spherical2Cartesian (globalRadius, globalElevation, globalAzimuth, &globalEyeX, &globalEyeY, &globalEyeZ);
  glutPostRedisplay ();
}

void renderGreenPlane ()
{
  glColor3f (0.2f, 0.8f, 0.2f);
  glBegin (GL_TRIANGLES);
  glVertex3f (100.0f, 0, -100.0f);
  glVertex3f (-100.0f, 0, -100.0f);
  glVertex3f (-100.0f, 0, 100.0f);

  glVertex3f (100.0f, 0, -100.0f);
  glVertex3f (-100.0f, 0, 100.0f);
  glVertex3f (100.0f, 0, 100.0f);
  glEnd ();
  glColor3f (1, 1, 1);
}

int timebase = 0, frame = 0;
void renderScene ()
{
  float fps;
  int time;
  char s[64];

  // clear buffers
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // helps in avoiding rounding mistakes, discards old matrix, read more
  glLoadIdentity ();

  /*draw absolute (before any transformation) axes*/
  draw_axes ();

  // set the camera
  gluLookAt (globalEyeX, globalEyeY, globalEyeZ,
             globalCenterX, globalCenterY, globalCenterZ,
             globalUpX, globalUpY, globalUpZ);

  // put the geometric transformations here
  glRotatef (globalAngle, globalRotateX, globalRotateY, globalRotateZ);
  glTranslatef (globalTranslateX, globalTranslateY, globalTranslateZ);
  glScalef (globalScaleX, globalScaleY, globalScaleZ);

  operations_render (globalOperations);

  frame++;
  time = glutGet (GLUT_ELAPSED_TIME);
  if (time - timebase > 1000)
    {
      fps = frame * 1000.0 / (time - timebase);
      timebase = time;
      frame = 0;
      sprintf (s, "FPS: %f6.2", fps);
      glutSetWindowTitle (s);
    }

  // End of frame
  redisplay ();
  glutSwapBuffers ();
}

void xml_load_and_set_env (const char *const filename)
{
  operations_load_xml (filename, globalOperations);
  operations_render (globalOperations);
  env_load_defaults ();
}
/*! @addtogroup input
 * @{*/
void keyboardFunc (unsigned char key, int xmouse, int ymouse)
{
  /*
      Rotation:
          x y z       x↓ y↓ z↓
          X Y Z       x↑ y↑ z↑
      angle:
          , .     θ-=s    θ+=s
          < >     s↓     s↑
      translation:
          q w e       y↓ z↑ y↑
          a s d       x↓ z↓ x↑
      scaling:
          u i o       y↓ z↑ y↑
          j k l       x↓ z↓ x↑
      background color: r g b R G B
      camera:
          1 2 3 ⎫       ! @ # ⎫        EyeX     EyeY     EyeZ
          4 5 6 ⎬↓      $ % ^ ⎬↑       CenterX  CenterY  CenterZ
          7 8 9 ⎭       & * ( ⎭        UpX      UpY      UpZ
   */
  switch (key)
    {
      /*Rotation*/
      case 'x':
        globalRotateX -= 1;
      break;

      case 'X':
        globalRotateX += 1;
      break;

      case 'y':
        globalRotateY -= 1;
      break;

      case 'Y':
        globalRotateY += 1;
      break;

      case 'z':
        globalRotateZ -= 1;
      break;

      case 'Z':
        globalRotateZ += 1;
      break;

      case ',':
        globalAngle -= globalAngleStep;
      break;

      case '.':
        globalAngle += globalAngleStep;
      break;

      case '<':
        globalAngleStep /= 2;
      break;

      case '>':
        globalAngleStep *= 2;
      break;

      /*translation*/
      case 'T':
        globalTranslateStep *= 2;
      break;
      case 't':
        if (globalTranslateStep > 1)
          globalTranslateStep /= 2;
      break;

      /*z-axis*/
      case 'w':
        globalTranslateZ += globalTranslateStep;
      break;
      case 's':
        globalTranslateZ -= globalTranslateStep;
      break;

      /*x-axis*/
      case 'a':
        globalTranslateX -= globalTranslateStep;
      break;
      case 'd':
        globalTranslateX += globalTranslateStep;
      break;

      /*y-axis*/
      case 'q':
        globalTranslateY -= globalTranslateStep;
      break;
      case 'e':
        globalTranslateY += globalTranslateStep;
      break;
      /*end of translation*/

      /*scaling*/

      /*z-axis*/
      case 'i':
        globalScaleZ += globalScaleStep;
      break;
      case 'k':
        globalScaleZ -= globalScaleStep;
      break;

      /*x-axis*/
      case 'j':
        globalScaleX -= globalScaleStep;
      break;
      case 'l':
        globalScaleX += globalScaleStep;
      break;

      /*y-axis*/
      case 'u':
        globalScaleY -= globalScaleStep;
      break;
      case 'o':
        globalScaleY += globalScaleStep;
      break;
      /*end of scaling*/

      /*camera*/
      case '`':
        globalEyeStep *= 2;
      break;
      case '~':
        if (globalEyeStep > 1)
          globalEyeStep /= 2;
      break;

      case '1':
        globalEyeX -= globalEyeStep;
      break;
      case '!':
        globalEyeX += globalEyeStep;
      break;
      case '2':
        globalEyeY -= globalEyeStep;
      break;
      case '@':
        globalEyeY += globalEyeStep;
      break;
      case '3':
        globalEyeZ -= globalEyeStep;
      break;
      case '#':
        globalEyeZ += globalEyeStep;
      break;

      case '4':
        globalCenterX -= globalCenterStep;
      break;
      case '$':
        globalCenterX += globalCenterStep;
      break;
      case '5':
        globalCenterY -= globalCenterStep;
      break;
      case '%':
        globalCenterY += globalCenterStep;
      break;
      case '6':
        globalCenterZ -= globalCenterStep;
      break;
      case '^':
        globalCenterZ += globalCenterStep;
      break;
      case '7':
        globalUpX -= globalUpStep;
      break;
      case '&':
        globalUpX += globalUpStep;
      break;
      case '8':
        globalUpY -= globalUpStep;
      break;
      case '*':
        globalUpY += globalUpStep;
      break;
      case '9':
        globalUpZ -= globalUpStep;
      break;
      case '(':
        globalUpZ += globalUpStep;
      break;

      /*reset environment*/
      case '0':
        env_load_defaults ();
      break;

      default:
        break;
    }
  redisplay (); //request display() call ASAP
}

int globalXPrev = 0;
int globalYPrev = 0;

void mouseFunc (int button, int state, int x, int y)
{
  // Wheel reports as button 3(scroll up) and button 4(scroll down)
  if ((button == 3) || (button == 4)) // It's a wheel event
    {
      // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
      if (state == GLUT_UP)
        return; // Disregard redundant GLUT_UP events

      if (button == 3)
        {
          globalRadius *= .8f;
          if (globalRadius < 1.0f)
            globalRadius = 1.0f;
        }
      else
        globalRadius *= 1.5;
      printf ("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", x, y);
    }
  else
    {  // normal button event
      printf ("Button %s At %d %d\n", (state == GLUT_DOWN) ? "Down" : "Up", x, y);
      if (button == GLUT_LEFT_BUTTON)
        {
          if (state == GLUT_DOWN)
            {
              globalMouseLeftButton = true;
              globalXPrev = x;
              globalYPrev = y;
            }
          if (state == GLUT_UP)
            {
              globalMouseLeftButton = false;
            }
        }
      else if (button == GLUT_RIGHT_BUTTON)
        {
          if (state == GLUT_DOWN)
            {
              int window_width = glutGet (GLUT_WINDOW_WIDTH);
              int window_height = glutGet (GLUT_WINDOW_HEIGHT);

              GLbyte color[4];
              GLfloat depth;
              GLuint index;

              //glReadPixels (x, window_height - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
              glReadPixels (x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
              //glReadPixels (x, window_height - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

              double ox, oy, oz;

              double model[16];
              glGetDoublev (GL_MODELVIEW_MATRIX, model);
              double proj[16];
              glGetDoublev (GL_PROJECTION_MATRIX, proj);
              int view[4];
              glGetIntegerv (GL_VIEWPORT, view);
              gluUnProject (x, window_height - y - 1, depth,
                            model,
                            proj,
                            view,
                            &ox, &oy, &oz);
              fprintf (stderr, "%d %d\n", x, y);
              //globalTranslateX = -ox;
              //globalTranslateZ = -oz;
              /*cartesian2Spherical (ox, globalEyeY, oz,
                                   &globalRadius, &globalAzimuth, &globalElevation);*/
              //              globalTranslateX = ox;
              //              globalTranslateZ = oz;
              globalCenterX = ox;
              globalCenterZ = oz;

              fprintf (stderr, "%f %f %f\n", ox, oy, oz);
            }
        }
    }

  redisplay ();
}

void motionFunc (int x, int y)
{
  if (globalMouseLeftButton)
    {
      const double stepAlfa = 0.035;
      const double stepBeta = 0.035;

      if (x < globalXPrev)
        globalAzimuth += stepAlfa;
      else if (x > globalXPrev)
        globalAzimuth -= stepAlfa;

      if (y < globalYPrev)
        {
          globalElevation -= stepBeta;
          if (globalElevation < -1.5)
            globalElevation = -1.5;
        }
      else if (y > globalYPrev)
        {
          globalElevation += stepBeta;
          if (globalElevation > 1.5)
            globalElevation = 1.5;
        }

      globalXPrev = x;
      globalYPrev = y;
    }
  redisplay ();
}

//! @} end of group input
void engine_run (int argc, char **argv)
{

  if (argc != 2)
    {
      fprintf (stderr, "Number of arguments is not one\n");
      exit (1);
    }

  // init GLUT and the window
  glutInit (&argc, argv);

  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition (100, 100);
  glutInitWindowSize (800, 800);
  glutCreateWindow ("engine");

  // Required callback registry
  glutDisplayFunc (renderScene);
  glutReshapeFunc (changeSize);

  // Callback registration for keyboard processing
  glutKeyboardFunc (keyboardFunc);

  // Callback registration for mouse processing
  glutMouseFunc (mouseFunc);
  glutMotionFunc (motionFunc);

  //  OpenGL settings
  glEnable (GL_DEPTH_TEST);

  // activate 2D texturing (slide 10) [class11]
  glEnable (GL_TEXTURE_2D);

  glEnable (GL_RESCALE_NORMAL);

  // activate lighting (done once in initialization) (slides 5) [class9]
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_LIGHT1);
  glEnable (GL_LIGHT2);
  glEnable (GL_LIGHT3);
  glEnable (GL_LIGHT4);
  glEnable (GL_LIGHT5);
  glEnable (GL_LIGHT6);
  glEnable (GL_LIGHT7);

  // activate arrays (slide 12) [class11]
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  //const float amb[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  //glLightModelfv (GL_LIGHT_MODEL_AMBIENT, amb);
  // glEnable (GL_CULL_FACE);
  //glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);


  // enter GLUT's main cycle
  glewInit ();

  xml_load_and_set_env (argv[1]);
  glutMainLoop ();
}

int main (int argc, char **argv)
{
  engine_run (argc, argv);
  return 1;
}
//!@} end of group engine

