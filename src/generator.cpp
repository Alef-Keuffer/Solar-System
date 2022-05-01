#define _USE_MATH_DEFINES

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <tuple>
#include <fplll/nr/matrix.h>
using glm::mat4, glm::vec4, glm::vec3, std::vector, std::tuple, std::string, std::ifstream, std::ios, std::stringstream, std::array, glm::mat4x3;

const char *SPHERE = "sphere";
const char *CUBE = "box";
const char *CONE = "cone";
const char *PLANE = "plane";
const char *BEZIER = "bezier";

using namespace std;

/*! @addtogroup generator
* @{*/

/*! @addtogroup points
 * @{*/
void points_vertex (const float x, const float y, const float z, unsigned int *pos, float points[])
{
  points[*pos] = x;
  points[*pos + 1] = y;
  points[*pos + 2] = z;
  *pos += 3;
}

void points_write (const char *filename, const unsigned int nVertices, const float points[])
{
  FILE *fp = fopen (filename, "w");
  if (!fp)
    {
      fprintf (stderr, "failed to open file");
      exit (1);
    }

  fwrite (&nVertices, sizeof (unsigned int), 1, fp);
  fwrite (points, 3 * sizeof (float), nVertices, fp);

  fclose (fp);
}
//!@} end of group points

/*! @addtogroup model
 * @{*/

/*! @addtogroup plane
* @{*/
void model_plane_vertices (const float length, const unsigned int divisions, float *points)
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  unsigned int pos = 0;

  for (unsigned int m = 1; m <= divisions; m++)
    {
      for (unsigned int n = 1; n <= divisions; n++)
        {
          float i = (float) m;
          float j = (float) n;

          points_vertex (o + d * (i - 1), 0, o + d * (j - 1), &pos, points); //P1
          points_vertex (o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, 0, o + d * j, &pos, points); //P2

          /*Cull face*/
          points_vertex (o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * (i - 1), 0, o + d * (j - 1), &pos, points); //P1
          points_vertex (o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (o + d * (i - 1), 0, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, 0, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (o + d * i, 0, o + d * j, &pos, points); //P2
        }
    }
}

inline unsigned int model_plane_nVertices (const unsigned int divisions)
{ return divisions * divisions * 12; }

void model_plane_write (const char *filepath, const float length, const unsigned int divisions)
{
  const unsigned int nVertices = model_plane_nVertices (divisions);
  float points[3 * nVertices];
  model_plane_vertices (length, divisions, points);
  points_write (filepath, nVertices, points);
}
//!@} end of group plane

/*! @addtogroup cube
* @{*/
void model_cube_vertices (const float length, const unsigned int divisions, float points[])
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  unsigned int pos = 0;

  for (unsigned int m = 1; m <= divisions; m++)
    {
      for (unsigned int n = 1; n <= divisions; n++)
        {
          float i = (float) m;
          float j = (float) n;

          // top
          points_vertex (o + d * (i - 1), -o, o + d * (j - 1), &pos, points); //P1
          points_vertex (o + d * (i - 1), -o, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, -o, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (o + d * i, -o, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (o + d * (i - 1), -o, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, -o, o + d * j, &pos, points); //P2

          // bottom
          points_vertex (o + d * (i - 1), o, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * (i - 1), o, o + d * (j - 1), &pos, points); //P1
          points_vertex (o + d * i, o, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (o + d * (i - 1), o, o + d * j, &pos, points); //P1'z
          points_vertex (o + d * i, o, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (o + d * i, o, o + d * j, &pos, points); //P2

          // left
          points_vertex (o, o + d * (i - 1), o + d * (j - 1), &pos, points); //P1
          points_vertex (o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
          points_vertex (o, o + d * i, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (o, o + d * i, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
          points_vertex (o, o + d * i, o + d * j, &pos, points); //P2

          // right
          points_vertex (-o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
          points_vertex (-o, o + d * (i - 1), o + d * (j - 1), &pos, points); //P1
          points_vertex (-o, o + d * i, o + d * (j - 1), &pos, points); //P1'x

          points_vertex (-o, o + d * (i - 1), o + d * j, &pos, points); //P1'z
          points_vertex (-o, o + d * i, o + d * (j - 1), &pos, points); //P1'x
          points_vertex (-o, o + d * i, o + d * j, &pos, points); //P2

          // front
          points_vertex (o + d * (i - 1), o + d * (j - 1), o, &pos, points); //P1
          points_vertex (o + d * (i - 1), o + d * j, o, &pos, points); //P1'z
          points_vertex (o + d * i, o + d * (j - 1), o, &pos, points); //P1'x

          points_vertex (o + d * i, o + d * (j - 1), o, &pos, points); //P1'x
          points_vertex (o + d * (i - 1), o + d * j, o, &pos, points); //P1'z
          points_vertex (o + d * i, o + d * j, o, &pos, points); //P2

          // back
          points_vertex (o + d * (i - 1), o + d * j, -o, &pos, points); //P1'z
          points_vertex (o + d * (i - 1), o + d * (j - 1), -o, &pos, points); //P1
          points_vertex (o + d * i, o + d * (j - 1), -o, &pos, points); //P1'x

          points_vertex (o + d * (i - 1), o + d * j, -o, &pos, points); //P1'z
          points_vertex (o + d * i, o + d * (j - 1), -o, &pos, points); //P1'x
          points_vertex (o + d * i, o + d * j, -o, &pos, points); //P2
        }
    }
}

unsigned int model_cube_nVertices (const unsigned int divisions)
{ return divisions * divisions * 36; }

void model_cube_write (const char *filepath, const float length, const unsigned int divisions)
{
  const unsigned int nVertices = model_cube_nVertices (divisions);
  float points[3 * nVertices];
  model_cube_vertices (length, divisions, points);
  points_write (filepath, nVertices, points);
}

//!@} end of group cube

/*! @addtogroup cone
* @{*/

/*!
 * \f{aligned}{
 * x &= r⋅\frac{h}{\textrm{height}} ⋅ \cos(θ)\\[2em]
 * y &= h + \textrm{height}\\[2em]
 * z &= r⋅\frac{h}{\textrm{height}} ⋅ \sin(θ)
 * \f}\n
 *
 * \f{aligned}{
 *  r &≥ 0\\
 *  θ &∈ \left\{-π      + i⋅s : s = \frac{2π}{\textrm{slices}}      ∧ i ∈ \{0,...,\textrm{slices}\} \right\}\\
 *  h &∈ \left\{- \textrm{height} + j⋅t : t =  \frac{\textrm{height}}{\textrm{stacks}} ∧ j ∈ \{0,...,\textrm{stacks}\} \right\}
 *  \f}
 *
 *  See the [3d model](https://www.math3d.org/7oeSkmuns).
 */
static inline void
model_cone_vertex (const float r, const float height, const float theta, const float h, unsigned int *pos,
                   float *points)
{
  /*
     x = r ⋅ (h/height) ⋅ cos(θ)
     y = 2 ⋅ (height + h)
     z = r ⋅ (h/height) ⋅ sin(θ)

     r ≥ 0
     θ ∈ {-π      + i⋅s : s = 2π/slices      ∧ i ∈ {0,...,slices} }
     h ∈ {-height + j⋅t : t =  height/stacks ∧ j ∈ {0,...,stacks} }

     check:
         1. https://www.math3d.org/7oeSkmuns
   */
  points_vertex (r * h / height * cos (theta), height + h, r * h / height * sin (theta), pos, points);
}

void model_cone_vertices (const float r, const float height, const unsigned int slices, const unsigned int stacks,
                          float points[])
{

  const float s = 2.0f * (float) M_PI / (float) slices;
  const float t = height / (float) stacks;
  const float theta = -M_PI;
  const float h = -height;

  unsigned int pos = 0;

  for (unsigned int m = 1; m <= slices; m++)
    {
      for (unsigned int n = 1; n <= stacks; n++)
        {
          float i = (float) m;
          float j = (float) n;

          //base
          points_vertex (0, 0, 0, &pos, points); //O
          model_cone_vertex (r, height, theta + s * (i - 1), h, &pos, points); //P1
          model_cone_vertex (r, height, theta + s * i, h, &pos, points); //P2

          model_cone_vertex (r, height, theta + s * i, h + t * (j - 1), &pos, points); // P2
          model_cone_vertex (r, height, theta + s * (i - 1), h + t * j, &pos, points); // P1'
          model_cone_vertex (r, height, theta + s * i, h + t * j, &pos, points); //P2'

          model_cone_vertex (r, height, theta + s * i, h + t * (j - 1), &pos, points); // P2
          model_cone_vertex (r, height, theta + s * (i - 1), h + t * (j - 1), &pos, points); // P1
          model_cone_vertex (r, height, theta + s * (i - 1), h + t * j, &pos, points); // P1'
        }
    }
}

static inline unsigned int model_cone_nVertices (const unsigned int stacks, const unsigned int slices)
{
  return slices * stacks * 9;
}

void model_cone_write (const char *filepath, const float radius, const float height, const unsigned int slices,
                       const unsigned int stacks)
{
  const unsigned int nVertices = model_cone_nVertices (stacks, slices);
  float points[3 * nVertices];
  model_cone_vertices (radius, height, slices, stacks, points);
  points_write (filepath, nVertices, points);
}

//!@} end of group cone

/*! @addtogroup sphere
* @{*/

static inline void
model_sphere_vertex (const float r, const float theta, const float phi, unsigned int *pos, float *points)
{
  /*
      x = r ⋅ sin(θ)cos(φ)
      y = r ⋅ sin(φ)
      z = r ⋅ cos(θ)cos(φ)

      r ≥ 0
      θ ∈ {-π +   i⋅s : s = 2π/slices ∧ i ∈ {0,...,slices} }
      ϕ ∈ {-π/2 + j⋅t : t =  π/stacks ∧ j ∈ {0,...,stacks} }

      check
          1. https://www.math3d.org/EumEEZBKe
          2. https://www.math3d.org/zE4n6xayX
   */
  points_vertex (r * sin (theta) * cos (phi), r * sin (phi), r * cos (theta) * cos (phi), pos, points);
}

static void model_sphere_vertices (const float r, const unsigned int slices, const unsigned int stacks, float points[])
{
  // https://www.math3d.org/EumEEZBKe
  // https://www.math3d.org/zE4n6xayX

  const float s = 2.0f * (float) M_PI / (float) slices;
  const float t = M_PI / (float) stacks;
  const float theta = -M_PI;
  const float phi = -M_PI / 2.0f;

  unsigned int pos = 0;

  for (unsigned int m = 1; m <= slices; m++)
    {
      for (unsigned int n = 1; n <= stacks; n++)
        {
          float i = (float) m;
          float j = (float) n;

          model_sphere_vertex (r, theta + s * (i - 1), phi + t * j, &pos, points); // P1'
          model_sphere_vertex (r, theta + s * i, phi + t * (j - 1), &pos, points); // P2
          model_sphere_vertex (r, theta + s * i, phi + t * j, &pos, points); //P2'

          model_sphere_vertex (r, theta + s * (i - 1), phi + t * (j - 1), &pos, points); // P1
          model_sphere_vertex (r, theta + s * i, phi + t * (j - 1), &pos, points); // P2
          model_sphere_vertex (r, theta + s * (i - 1), phi + t * j, &pos, points); // P1'
        }
    }
}

/*
*******************************************************************************
Bezier patches
*******************************************************************************
*/

const auto Mcr = mat4 (
    -0.5f, 1.5f, -1.5f, 0.5f,
    1.0f, -2.5f, 2.0f, -0.5f,
    -0.5f, 0.0f, 0.5f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f);

const auto Mb = mat4 (
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 3, 0, 0,
    1, 0, 0, 0);

vector<array<vec3, 16>> read_Bezier (string &patch)
{
  string buffer;
  ifstream myFile;

  myFile.open (patch, ios::in | ios::out);
  getline (myFile, buffer);
  // Número de patches presentes no ficheiro.
  int n_patches = stoi (buffer);

  // Vetor de vetores de índices.
  vector<vector<int>> patches;

  // Ciclo externo lê uma linha (patch) de cada vez
  for (int j = 0; j < n_patches; j++)
    {
      vector<int> patchIndexes;
      /*
      Ciclo interno lê os índices dos pontos de controlo de cada patch, sabendo que cada
      patch terá 16 pontos de controlo.
      */
      for (int i = 0; i < 16; i++)
        {
          getline (myFile, buffer, ',');
          patchIndexes.push_back (stoi (buffer));
        }
      patches.push_back (patchIndexes);
    }

  getline (myFile, buffer);
  // Número de pontos presentes no ficheiro.
  int pts = stoi (buffer);

  // Vetor que guardará as coordenadas de pontos de controlo para superfície de Bézier.
  vector<vec3> control;
  for (int j = 0; j < pts; j++)
    {
      vec3 v;
      getline (myFile, buffer, ',');
      v.x = stof (buffer);
      getline (myFile, buffer, ',');
      v.y = stof (buffer);
      getline (myFile, buffer, ',');
      v.z = stof (buffer);
      control.push_back (v);
    }

  /*
  Percorrem-se os vetores que, para cada patch, guardam os seus índices de pontos de controlo.
  Para cada patch, constroi-se um vetor com as coordenadas dos seus pontos de controlo.
  */
  vector<array<vec3, 16>> pointsInPatches;
  for (auto &patche : patches)
    {
      array<vec3, 16> pointsInPatch{};
      for (int j = 0; j < 16; j++)
        {
          pointsInPatch[j] = control[patche[j]];
        }
      pointsInPatches.push_back (pointsInPatch);
    }

  myFile.close ();
  return pointsInPatches;
}

template<typename T> inline vec4 dec_polynomial (T n)
{
  return {pow (n, 3), pow (n, 2), n, 1};
}
/*!
 * @param a a[m][p]
 * @param b b[p][q]
 * @param[out] r r[m][q]
 * @param m
 * @param q
 * @param p
 */
template<typename T1, typename T2, typename T3>
void mult (const T1 &a, const T2 &b, T3 &r, const int m, const int p, const int q)
{
  for (auto i = 0; i < m; ++i)
    for (auto j = 0; j < q; ++j)
      {
        r[m * i + j] *= 0;
        for (auto k = 0; k < p; ++k)
          r[m * i + j] += a[m * i + k] * b[p * k + j];
      }
}

glm::vec3 get_bezier_point (const float u, const float v, const array<glm::vec3, 16> &P)
{
  // B(u,v) = U M P Mᵀ V
  glm::vec4 U = dec_polynomial (u);
  glm::vec4 V = dec_polynomial (v);

  vector<glm::vec3> VMP;
  auto VM = V * glm::transpose (Mb); //1x4x1
  mult (VM, P, VMP, 1, 4, 4);

  vector<glm::vec3> VMPMU;
  auto MU = Mb * U;
  mult (VMP, MU, VMPMU, 1, 4, 1);
  return VMPMU[0];
}

vector<glm::vec3> get_bezier_patch (array<glm::vec3, 16> control_points, int int_tesselation) {
  vector<glm::vec3> pontos;
  float float_tesselation = (float) int_tesselation;
  for (int tv = 0; tv < int_tesselation; tv++) {
      float v = (float) tv / float_tesselation;
      for (int tu = 0; tu < int_tesselation; tu++) {
          float u = (float) tu / (float) float_tesselation;
          // triângulo superior
          pontos.push_back (get_bezier_point ((u + (1.0f / float_tesselation)), (v + (1.0f / float_tesselation)), control_points));
          pontos.push_back (get_bezier_point (u, (v + (1.0f / float_tesselation)), control_points));
          pontos.push_back (get_bezier_point (u, v, control_points));
          // triângulo inferior
          pontos.push_back (get_bezier_point (u, v, control_points));
          pontos.push_back (get_bezier_point ((u + (1.0f / float_tesselation)), v, control_points));
          pontos.push_back (get_bezier_point ((u + (1.0f / float_tesselation)), (v + (1.0f / float_tesselation)), control_points));
        }
    }
  return pontos;
}

vector<glm::vec3> get_bezier_surface (vector<array<glm::vec3, 16>> control_set, int int_tesselation) {
    vector<glm::vec3> pts;
    for (int i = 0; i < control_set.size(); i++) {
        vector<glm::vec3> temp = get_bezier_patch(control_set[i], int_tesselation);
        pts.insert(pts.end(), temp.begin(), temp.end());
    }

    return pts;
}

void model_bezier_write(string filepath, int tesselation) {
    vector<array<glm::vec3, 16>> control_points = read_Bezier(filepath);
    vector<glm::vec3> vertices = get_bezier_surface(control_points, tesselation);
    const unsigned int nVertices = vertices.size();
    vector<float> coords;
    for (int i = 0; i < vertices.size(); i++) {
        coords.push_back(vertices[i].x);
        coords.push_back(vertices[i].y);
        coords.push_back(vertices[i].z);
    }
    points_write(filepath.c_str(), nVertices, coords.data());
}


/*
*******************************************************************************
End of Bezier patches
*******************************************************************************
*/


static inline unsigned int model_sphere_nVertices (const unsigned int slices, const unsigned int stacks)
{
  return slices * stacks * 6;
}

void model_sphere_write (const char *filepath, float radius, unsigned int slices, unsigned int stacks)
{
  const unsigned int nVertices = model_sphere_nVertices (slices, stacks);
  float points[3 * nVertices];
  model_sphere_vertices (radius, slices, stacks, points);
  points_write (filepath, nVertices, points);
}
//!@} end of group sphere

//!@} end of group model

//!@} end of group generator

int main(int argc, char *argv[]) {
    if (argc < 4) { printf("Not enough arguments"); }
    else {
        const char *filepath = argv[argc - 1];
        const char *polygon = argv[1];

        if (!strcmp(PLANE, polygon))
            model_plane_write(filepath, strtof(argv[2], nullptr), strtoul(argv[3], nullptr, 10));
        if (!strcmp(CUBE, polygon)) model_cube_write(filepath, atof(argv[2]), strtoul(argv[3], nullptr, 10));
        if (!strcmp(CONE, polygon))
            model_cone_write(filepath, atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[5]));
        if (!strcmp(SPHERE, polygon)) model_sphere_write(filepath, atof(argv[2]), atoi(argv[3]), atoi(argv[4]));
        if (!strcmp(BEZIER, polygon)) model_bezier_write(filepath, atoi(argv[2]));
    }
  return 1;
}