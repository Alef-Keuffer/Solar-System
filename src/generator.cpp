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
#include <csignal>

#include "curves.h"

using glm::mat4, glm::vec4, glm::vec3, glm::vec2, glm::mat4x3;
using glm::normalize, glm::cross;

using std::vector, std::tuple, std::array;

using std::string, std::ifstream, std::ios, std::stringstream;
using std::cerr, std::endl, glm::to_string;

template<class T>
concept arithmetic =  std::is_integral<T>::value or std::is_floating_point<T>::value;

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

void
model_write (const char *const filename,
             const vector<vec3> &vertices,
             const vector<vec3> &normals,
             const vector<vec2> &texture)
{
  FILE *fp = fopen (filename, "w");

  if (!fp)
    {
      fprintf (stderr, "failed to open file: %s", filename);
      exit (1);
    }

  assert(vertices.size () < INT_MAX);
  const int nVertices = vertices.size ();
  const int nNormals = normals.size ();
  const int nTextures = texture.size ();
  fwrite (&nVertices, sizeof (nVertices), 1, fp);
  fwrite (vertices.data (), sizeof (vertices.back ()), nVertices, fp);
  fwrite (normals.data (), sizeof (normals.back ()), nNormals, fp);
  fwrite (texture.data (), sizeof (texture.back ()), nTextures, fp);

  fclose (fp);

  cerr << "[generator] Wrote "
       << nVertices << " vertices, "
       << nNormals << " normals, "
       << nTextures << " textures to "
       << filename << endl;
}

//!@} end of group points

/*! @addtogroup model
 * @{*/

/*! @addtogroup plane
* @{*/
void model_plane_vertices (const float length,
                           const unsigned int divisions,
                           vector<vec3> &points,
                           vector<vec3> &normals,
                           vector<vec2> &texture)
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  for (unsigned int uidiv1 = 1; uidiv1 <= divisions; ++uidiv1)
    {
      for (unsigned int uidiv2 = 1; uidiv2 <= divisions; ++uidiv2)
        {
          auto const fdiv1 = (float) uidiv1;
          auto const fdiv2 = (float) uidiv2;
          auto const fdivisions = (float) divisions;

          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * (fdiv2 - 1)); //P1
          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * fdiv2); //P1'z
          points.emplace_back (o + d * fdiv1, 0, o + d * (fdiv2 - 1)); //P1'x

          points.emplace_back (o + d * fdiv1, 0, o + d * (fdiv2 - 1)); //P1'x
          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * fdiv2); //P1'z
          points.emplace_back (o + d * fdiv1, 0, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, 1, 0);
          for (auto e : {
              vec2 (-1.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),

              vec2 (0.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);

          /*Cull face*/
          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * fdiv2); //P1'z
          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * (fdiv2 - 1)); //P1
          points.emplace_back (o + d * fdiv1, 0, o + d * (fdiv2 - 1)); //P1'x

          points.emplace_back (o + d * (fdiv1 - 1), 0, o + d * fdiv2); //P1'z
          points.emplace_back (o + d * fdiv1, 0, o + d * (fdiv2 - 1)); //P1'x
          points.emplace_back (o + d * fdiv1, 0, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, -1, 0);
          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);
        }
    }
}

static inline unsigned int model_plane_nVertices (const unsigned int divisions)
{ return divisions * divisions * 12; }

void model_plane_write (const char *filepath, const float length, const unsigned int divisions)
{
  const unsigned int nVertices = model_plane_nVertices (divisions);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_plane_vertices (length, divisions, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}
//!@} end of group plane

/*! @addtogroup cube
* @{*/
void model_cube_vertices (const float length,
                          const unsigned int divisions,
                          vector<vec3> &vertices,
                          vector<vec3> &normals,
                          vector<vec2> &texture)
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  for (unsigned int uidiv1 = 1; uidiv1 <= divisions; uidiv1++)
    {
      for (unsigned int uidiv2 = 1; uidiv2 <= divisions; uidiv2++)
        {
          auto const fdiv1 = (float) uidiv1;
          auto const fdiv2 = (float) uidiv2;
          auto const fdivisions = (float) divisions;

          // y+
          vertices.emplace_back (o + d * (fdiv1 - 1), -o, o + d * (fdiv2 - 1));//P1
          vertices.emplace_back (o + d * (fdiv1 - 1), -o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * fdiv1, -o, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (o + d * fdiv1, -o, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (o + d * (fdiv1 - 1), -o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * fdiv1, -o, o + d * fdiv2); //P2


          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, 1, 0);
          for (auto e : {
              vec2 (-1.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),

              vec2 (0.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);


          // y-
          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (o + d * fdiv1, o, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * fdiv1, o, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (o + d * fdiv1, o, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, -1, 0);
          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);


          // x-
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (o, o + d * fdiv1, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (-1, 0, 0);

          for (auto e : {
              vec2 (-1.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),

              vec2 (0.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);


          // x+
          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (-o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (-o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (-o, o + d * fdiv1, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (1, 0, 0);

          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);


          // z-
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * (fdiv2 - 1), o); //P1
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), o); //P1'x

          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), o); //P1'x
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * fdiv2, o); //P2

          for (auto e : {
              vec2 (-1.0f, -1.0f),//P1
              vec2 (-1.0f, 0.0f),//P1'z
              vec2 (0.0f, -1.0f),//P1'x

              vec2 (0.0f, -1.0f),//P1'x
              vec2 (-1.0f, 0.0f),//P1'z
              vec2 (0.0f, 0.0f)//P2
          })
            {
              normals.emplace_back (0, 0, -1);
              texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);
            }



          // z+
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, -o); //P1'z
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * (fdiv2 - 1), -o); //P1
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), -o); //P1'x

          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, -o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), -o); //P1'x
          vertices.emplace_back (o + d * fdiv1, o + d * fdiv2, -o); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, 0, 1);

          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)
          })
            texture.emplace_back ((fdiv1 - e[0]) / fdivisions, (fdiv2 - e[1]) / fdivisions);
        }
    }
}

static inline unsigned int model_cube_nVertices (const unsigned int divisions)
{ return divisions * divisions * 36; }

void model_cube_write (const char *const filepath,
                       const float length,
                       const unsigned int divisions)
{
  const unsigned int nVertices = model_cube_nVertices (divisions);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);

  model_cube_vertices (length, divisions, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
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

template<typename T>
    requires arithmetic<T>
static inline void
model_cone_vertex (const T r,
                   const T height,
                   const T theta,
                   const T h,
                   vector<vec3> &vertices)
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
  vertices.emplace_back (r * h / height * cos (theta), height + h, r * h / height * sin (theta));
}

template<typename T>
    requires arithmetic<T>
void model_cone_vertices (const T r,
                          const T height,
                          const unsigned int slices,
                          const unsigned int stacks,
                          vector<vec3> &vertices,
                          vector<vec3> &normals,
                          vector<vec2> &texture)
{

  const float s = 2.0f * (float) M_PI / (float) slices;
  const float t = height / (float) stacks;
  const float theta = -M_PI;
  const float h = -height;

  for (unsigned int slice = 1; slice <= slices; ++slice)
    {
      for (unsigned int stack = 1; stack <= stacks; ++stack)
        {
          auto const fslice = (float) slice;
          auto const fstack = (float) stack;
          auto const fslices = (float) slices;
          auto const fstacks = (float) stacks;

          //base
          vertices.emplace_back (0, 0, 0); //O
          model_cone_vertex (r, height, theta + s * (fslice - 1), h, vertices); //P1
          model_cone_vertex (r, height, theta + s * fslice, h, vertices); //P2

          for (int k = 0; k < 3; ++k)
            normals.emplace_back (0, -1, 0);

          //
          model_cone_vertex (r, height, theta + s * fslice, h + t * (fstack - 1), vertices); // P2
          model_cone_vertex (r, height, theta + s * (fslice - 1), h + t * fstack, vertices); // P1'
          model_cone_vertex (r, height, theta + s * fslice, h + t * fstack, vertices); //P2'

          //
          model_cone_vertex (r, height, theta + s * fslice, h + t * (fstack - 1), vertices); // P2
          model_cone_vertex (r, height, theta + s * (fslice - 1), h + t * (fstack - 1), vertices); // P1
          model_cone_vertex (r, height, theta + s * (fslice - 1), h + t * fstack, vertices); // P1'

          const auto P1 = vertices.end ()[-2];
          const auto P2 = vertices.end ()[-3];
          const auto P1_prime = vertices.end ()[-1];
          for (int k = 0; k < 6; ++k)
            normals.push_back (normalize (cross (P2 - P1_prime, P1 - P1_prime)));

          texture.emplace_back (0, 0);
          texture.emplace_back (-1, 0);
          texture.emplace_back (0, 0);

          for (int k = 0; k < 6; ++k)
            texture.emplace_back (fslice / fslices, fstack / fstacks);
        }
    }
}

static inline unsigned int model_cone_nVertices (const unsigned int stacks, const unsigned int slices)
{
  return slices * stacks * 9;
}

void model_cone_write (const char *const filepath,
                       const float radius,
                       const float height,
                       const unsigned int slices,
                       const unsigned int stacks)
{
  const unsigned int nVertices = model_cone_nVertices (stacks, slices);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_cone_vertices (radius, height, slices, stacks, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
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
                     vector<vec3> &normals)
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
  vertices.emplace_back (r * sin (theta) * cos (phi), r * sin (phi), r * cos (theta) * cos (phi));
  normals.emplace_back (sin (theta) * cos (phi), sin (phi), cos (theta) * cos (phi));
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

  auto fslices = (float) slices;
  auto fstacks = (float) stacks;

  for (unsigned int slice = 1; slice <= slices; ++slice)
    {
      for (unsigned int stack = 1; stack <= stacks; ++stack)
        {
          auto fslice = (float) slice;
          auto fstack = (float) stack;

          texture.emplace_back ((fslice - 1) / fslices, fstack / fstacks); // P1'
          texture.emplace_back (fslice / fslices, (fstack - 1) / fstacks); // P2
          texture.emplace_back (fslice / fslices, fstack / fstacks); // P2'

          texture.emplace_back ((fslice - 1) / fslices, (fstack - 1) / fstacks); // P1
          texture.emplace_back (fslice / fslices, (fstack - 1) / fstacks); // P2
          texture.emplace_back ((fslice - 1) / fslices, fstack / fstacks); // P1'

          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * fstack, vertices, normals); // P1'
          model_sphere_vertex (r, theta + s * fslice, phi + t * (fstack - 1), vertices, normals); // P2
          model_sphere_vertex (r, theta + s * fslice, phi + t * fstack, vertices, normals); // P2'

          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * (fstack - 1), vertices, normals); // P1
          model_sphere_vertex (r, theta + s * fslice, phi + t * (fstack - 1), vertices, normals); // P2
          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * fstack, vertices, normals); // P1'
        }
    }
}

void model_sphere_write (const char *const filepath,
                         const float radius,
                         const unsigned int slices,
                         const unsigned int stacks)
{

  const unsigned int nVertices = model_sphere_nVertices (slices, stacks);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_sphere_vertices (radius, slices, stacks, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}
//!@} end of group sphere

//!@} end of group model

/*! @addtogroup bezier
 * @{ */
vector<array<vec3, 16>> read_Bezier (const char *const patch)
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

void model_bezier_write (const int tesselation, const char *const in_patch_file, const char *const out_3d_file)
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

int main (const int argc, const char * const argv[])
{
  if (argc < 4)
    {
      cerr << "[generator] Not enough arguments" << endl;
      exit (EXIT_FAILURE);
    }
  else
    {
      const char *const out_file_path = argv[argc - 1];
      cerr << "[generator] output filepath: '" << out_file_path << "'" << endl;
      const char *const polygon = argv[1];
      cerr << "[generator] polygon to generate: " << polygon << endl;

      if (!strcmp (PLANE, polygon))
        {
          const float length = strtof (argv[2], nullptr);
          if (length <= 0.0)
            {
              cerr << "[generator] invalid length(" << length << ") for plane" << endl;
              exit (EXIT_FAILURE);
            }
          const int divisions = std::stoi (argv[3], nullptr, 10);
          if (divisions <= 0)
            {
              cerr << "[generator] invalid number of divisions(" << divisions << ") for plane" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] PLANE(length: " << length << ", divisions: " << divisions << ")" << endl;
          model_plane_write (out_file_path, length, divisions);
        }

      else if (!strcmp (CUBE, polygon))
        {
          const float length = strtof (argv[2], nullptr);
          if (length <= 0.0)
            {
              cerr << "[generator] invalid length(" << length << ") for cube" << endl;
              exit (EXIT_FAILURE);
            }
          const int divisions = std::stoi (argv[3], nullptr, 10);
          if (divisions <= 0)
            {
              cerr << "[generator] invalid number of divisions(" << divisions << ") for cube" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] CUBE(length: " << length << ", divisions: " << divisions << ")" << endl;
          model_cube_write (out_file_path, length, divisions);
        }
      else if (!strcmp (CONE, polygon))
        {
          const float radius = strtof (argv[2], nullptr);
          if (radius <= 0.0)
            {
              cerr << "[generator] invalid radius(" << radius << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const float height = strtof (argv[3], nullptr);
          if (height <= 0.0)
            {
              cerr << "[generator] invalid height(" << radius << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const int slices = std::stoi (argv[4], nullptr, 10);
          if (slices <= 0)
            {
              cerr << "[generator] invalid slices(" << slices << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const int stacks = std::stoi (argv[4], nullptr, 10);
          if (stacks <= 0)
            {
              cerr << "[generator] invalid stacks(" << stacks << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] CONE(radius: " << radius
               << ", height: " << height
               << ", slices: " << slices
               << ", stacks: " << stacks << ")" << endl;
          model_cone_write (out_file_path, radius, height, slices, stacks);
        }
      else if (!strcmp (SPHERE, polygon))
        {
          const float radius = strtof (argv[2], nullptr);
          if (radius <= 0.0)
            {
              cerr << "[generator] invalid radius(" << radius << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          const int slices = std::stoi (argv[3], nullptr, 10);
          if (slices <= 0)
            {
              cerr << "[generator] invalid slices(" << slices << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          const int stacks = std::stoi (argv[4], nullptr, 10);
          if (stacks <= 0)
            {
              cerr << "[generator] invalid stacks(" << stacks << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] SPHERE(radius: " << radius
               << ", slices: " << slices
               << ", stacks: " << stacks << ")"
               << endl;
          model_sphere_write (out_file_path, radius, slices, stacks);
        }
      else if (!strcmp (BEZIER, polygon))
        {
          const int tesselation = std::stoi (argv[2], nullptr, 10);
          if (tesselation <= 0)
            {
              cerr << "[generator] invalid tesselation(" << tesselation << ") for bezier patch" << endl;
              exit (EXIT_FAILURE);
            }
          const char *const input_patch_file_path = argv[3];
          if (access (input_patch_file_path, F_OK))
            {
              cerr << "[generator] file " << input_patch_file_path << " for bezier patch not found" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "BEZIER(tesselation: " << tesselation << "input file: " << input_patch_file_path << ")" << endl;
          model_bezier_write (tesselation, input_patch_file_path, out_file_path);
        }
      else
        {
          cerr << "[generator] Unkown object type: " << polygon << endl;
          exit (EXIT_FAILURE);
        }
    }
  return 0;
}