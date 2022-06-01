#define _USE_MATH_DEFINES

#include <cmath>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <tuple>
#include <iostream>

#include "curves.h"

using glm::mat4, glm::vec4, glm::vec3, glm::vec2, glm::mat4x3;
using glm::normalize;

using std::vector, std::tuple, std::array;

using std::string, std::ifstream, std::ios, std::stringstream;
using std::cerr, std::endl, glm::to_string;

const char *SPHERE = "sphere";
const char *CUBE = "box";
const char *CONE = "cone";
const char *PLANE = "plane";
const char *BEZIER = "bezier";
/*! @addtogroup generator
* @{*/

struct baseModel {
  int nVertices;
  float *vertices;
  float *normals;
  float *texture_coordinates;
};

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
      fprintf (stderr, "failed to open file: %s", filename);
      exit (1);
    }

  fwrite (&nVertices, sizeof (unsigned int), 1, fp);
  fwrite (points, 3 * sizeof (float), nVertices, fp);

  fclose (fp);
}

void model_write (const char *filename, const vector<vec3> &points, const vector<vec3> &normals, const vector<vec2> &texture)
{
  FILE *fp = fopen (filename, "w");

  if (!fp)
    {
      fprintf (stderr, "failed to open file: %s", filename);
      exit (1);
    }

  const size_t nVertices = points.size();
  fwrite (&nVertices, sizeof (unsigned int), 1, fp);
  fwrite (points.data(), sizeof(points.back()), points.size(), fp);
  fwrite (normals.data(), sizeof(normals.back()), normals.size(), fp);
  fwrite (texture.data(), sizeof(texture.back()), texture.size(), fp);

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

static inline unsigned int model_sphere_nVertices (const unsigned int slices, const unsigned int stacks)
{
  return slices * stacks * 6;
}

static inline void
model_sphere_vertex (const float r,
                     const float theta,
                     const float phi,
                     vector<vec3> &vertices,
                     vector<vec3> &normals,
                     vector<vec2> &texture)
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
  vertices.emplace_back(r * sin (theta) * cos (phi), r * sin (phi), r * cos (theta) * cos (phi));
  normals.emplace_back (sin (theta) * cos (phi), sin (phi), cos (theta) * cos (phi));
  texture.emplace_back(phi/M_2_PI,theta/M_PI);
}

static void model_sphere_vertices (const float r,
                                   const unsigned int slices,
                                   const unsigned int stacks,
                                   vector<vec3> &vertices,
                                   vector<vec3> &normals,
                                   vector<vec2> &texture)
{
  // https://www.math3d.org/EumEEZBKe
  // https://www.math3d.org/zE4n6xayX

  const float s = 2.0f * (float) M_PI / (float) slices;
  const float t = M_PI / (float) stacks;
  const float theta = -M_PI;
  const float phi = -M_PI / 2.0f;

  for (unsigned int m = 1; m <= slices; ++m)
    {
      for (unsigned int n = 1; n <= stacks; ++n)
        {
          float sl = (float) m;
          float st = (float) n;

          model_sphere_vertex (r, theta + s * (sl - 1), phi + t * st, vertices, normals, texture); // P1'
          model_sphere_vertex (r, theta + s * sl, phi + t * (st - 1), vertices, normals, texture); // P2
          model_sphere_vertex (r, theta + s * sl, phi + t * st, vertices, normals, texture); //P2'

          model_sphere_vertex (r, theta + s * (sl - 1), phi + t * (st - 1), vertices, normals, texture); // P1
          model_sphere_vertex (r, theta + s * sl, phi + t * (st - 1), vertices, normals, texture); // P2
          model_sphere_vertex (r, theta + s * (sl - 1), phi + t * st, vertices, normals, texture); // P1'
        }
    }
}



void model_sphere_write (const char * const filepath,
                         const float radius,
                         const unsigned int slices,
                         const unsigned int stacks)
{

  const unsigned int nVertices = model_sphere_nVertices (slices, stacks);
  vector<vec3> vertices;
  vertices.reserve(nVertices);
  vector<vec3> normals;
  normals.reserve(nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_sphere_vertices (radius, slices, stacks, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}
//!@} end of group sphere

//!@} end of group model

/*! @addtogroup bezier
 * @{ */
vector<array<vec3, 16>> read_Bezier (const char * const patch)
{
  string buffer;
  ifstream myFile;

  myFile.open (patch, ios::in | ios::out);
  getline (myFile, buffer);
  // Número de patches presentes no ficheiro.
  const int n_patches = stoi (buffer);

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
      for (int i = 0; i < 15; i++)
        {
          getline (myFile, buffer, ',');
          patchIndexes.push_back (stoi (buffer));
        }
      getline (myFile, buffer);
      patchIndexes.push_back (stoi (buffer));
      patches.push_back (patchIndexes);
    }

  getline (myFile, buffer);
  // Número de pontos presentes no ficheiro.
  const int pts = stoi (buffer);

  // Vetor que guardará as coordenadas de pontos de controlo para superfície de Bézier.
  vector<vec3> control;
  for (int j = 0; j < pts; j++)
    {
      vec3 v;
      getline (myFile, buffer, ',');
      v[0] = stof (buffer);
      getline (myFile, buffer, ',');
      v[1] = stof (buffer);
      getline (myFile, buffer);
      v[2] = stof (buffer);
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

  /*std::cout << "read_Bezier read:" << std::endl;
  for (auto arr : pointsInPatches)
    for (auto p : arr)
      std::cout << glm::to_string (p) << std::endl;*/
  return pointsInPatches;
}

void model_bezier_write (const int tesselation, const char * const in_patch_file, const char * const out_3d_file)
{
  vector<array<glm::vec3, 16>> control_points = read_Bezier (in_patch_file);
  vector<glm::vec3> vertices = get_bezier_surface (control_points, tesselation);
  const unsigned int nVertices = vertices.size ();
  vector<float> coords;
  for (auto &vertice : vertices)
    {
      coords.push_back (vertice.x);
      coords.push_back (vertice.y);
      coords.push_back (vertice.z);
    }
  points_write (out_3d_file, nVertices, coords.data ());
}
//!@} end of group bezier

//!@} end of group generator

int main (int argc, char *argv[])
{
  if (argc < 4)
    {
      cerr << "Not enough arguments" << endl;
      exit(EXIT_FAILURE);
    }
  else
    {
      const char * const out_file_path = argv[argc - 1];
      cerr << "output filepath: '" << out_file_path << "'" << endl;
      const char * const polygon = argv[1];
      cerr << "polgyon to generate: " << polygon << endl;

      if (!strcmp (PLANE, polygon))
        model_plane_write (out_file_path, strtof (argv[2], nullptr), strtoul (argv[3], nullptr, 10));
      else if (!strcmp (CUBE, polygon))
        model_cube_write (out_file_path, atof (argv[2]), strtoul (argv[3], nullptr, 10));
      else if (!strcmp (CONE, polygon))
        model_cone_write (out_file_path, atof (argv[2]), atof (argv[3]), atoi (argv[4]), atoi (argv[5]));
      else if (!strcmp (SPHERE, polygon))
        {
          const float radius = strtof (argv[2],nullptr);
          if (radius <= 0.0)
            {
              cerr << "invalid radius(" << radius << ") for sphere" << endl;
              exit(EXIT_FAILURE);
            }
          const unsigned long slices = strtoul(argv[3],nullptr,10);
          if (slices <= 0 || slices == ULONG_MAX)
            {
              cerr << "invalid slices(" << slices << ") for sphere" << endl;
              exit(EXIT_FAILURE);
            }
          const unsigned long stacks = strtoul (argv[4],nullptr,10);
          if (stacks <= 0 || stacks == ULONG_MAX)
            {
              cerr << "invalid stacks(" << stacks << ") for sphere" << endl;
              exit(EXIT_FAILURE);
            }
          cerr << "SPHERE(radius: " << radius << ", slices: " << slices << ", stacks: " << stacks << ")"  << endl;
          model_sphere_write (out_file_path, radius, slices, stacks);
        }
      else if (!strcmp (BEZIER, polygon))
        model_bezier_write (atoi (argv[2]), argv[3], out_file_path);
      else
        {
          cerr << "Unkown object type: " << polygon << endl;
          exit(EXIT_FAILURE);
        }
    }
  return 0;
}