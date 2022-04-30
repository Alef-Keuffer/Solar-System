#define _USE_MATH_DEFINES

#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *SPHERE = "sphere";
const char *CUBE = "box";
const char *CONE = "cone";
const char *PLANE = "plane";
const char *BEZIER = "bezier";

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <tuple>

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

struct Ponto {
  float x;
  float y;
  float z;
};

vector<vector<float>> read_Bezier (string patch)
{
  string buffer;
  ifstream myFile;

  myFile.open (patch, ios::in | ios::out);
  getline (myFile, buffer);
  // Número de patches presentes no ficheiro.
  int p = stoi (buffer);

  // Vetor de vetores de índices.
  vector<vector<int>> patches;

  // Ciclo externo lê uma linha (patch) de cada vez
  for (int j = 0; j < p; j++)
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
  vector<Ponto> control;
  for (int j = 0; j < pts; j++)
    {
      Ponto ponto;
      getline (myFile, buffer, ',');
      ponto.x = stof (buffer);
      getline (myFile, buffer, ',');
      ponto.y = stof (buffer);
      getline (myFile, buffer, ',');
      ponto.z = stof (buffer);

      control.push_back (ponto);
    }

  /*
  Percorrem-se os vetores que, para cada patch, guardam os seus índices de pontos de controlo.
  Para cada patch, constroi-se um vetor com as coordenadas dos seus pontos de controlo.
  */
  vector<vector<float>> pointsInPatches;
  for (int i = 0; i < patches.size (); i++)
    {
      vector<float> pointsInPatch;
      for (int j = 0; j < 16; j++)
        {
          Ponto p = control[patches[i][j]];
          pointsInPatch.push_back (p.x);
          pointsInPatch.push_back (p.y);
          pointsInPatch.push_back (p.z);
        }
    }

  myFile.close ();
  return pointsInPatches;
}

tuple<float, float, float> getBezierPoint (float u, float v, vector<float> x)
{
  float bernsteinU[4] = {powf (1 - u, 3), 3 * u * powf (1 - u, 2), 3 * powf (u, 2) * (1 - u), powf (u, 3)};
  float bernsteinV[4] = {powf (1 - v, 3), 3 * v * powf (1 - v, 2), 3 * powf (v, 2) * (1 - v), powf (v, 3)};
  tuple<float, float, float> ponto;
  get<0> (ponto) = 0.0;
  get<1> (ponto) = 0.0;
  get<2> (ponto) = 0.0;
  for (int j = 0; j < 4; j++)
    {
      for (int i = 0; i < 4; i++)
        {
          int indexCP = j * 12 + i * 3;
          get<0> (ponto) = get<0> (ponto) + x[indexCP] * bernsteinU[j] * bernsteinV[i];
          get<1> (ponto) = get<1> (ponto) + x[indexCP + 1] * bernsteinU[j] * bernsteinV[i];
          get<2> (ponto) = get<2> (ponto) + x[indexCP + 2] * bernsteinU[j] * bernsteinV[i];
        }
    }
  return ponto;
}

string gen_Bezier (string patch, int tesselation)
{
  vector<float> control = read_Bezier (patch, tesselation);
  stringstream res;
  ifstream myFile;
  string buffer;
  vector<tuple<float, float, float>> pontos;
  vector<float> x;
  myFile.open (patch, ios::in | ios::out);
  getline (myFile, buffer);
  int npatches = stoi (buffer);
  for (int i = 0; i < npatches; i += 1)
    {
      for (int p = 0; p < 15; p += 1)
        {
          getline (myFile, buffer, ',');
          x.push_back (control[stoi (buffer) * 3]);
          x.push_back (control[stoi (buffer) * 3 + 1]);
          x.push_back (control[stoi (buffer) * 3 + 2]);
        }
      getline (myFile, buffer);
      x.push_back (control[stoi (buffer) * 3]);
      x.push_back (control[stoi (buffer) * 3 + 1]);
      x.push_back (control[stoi (buffer) * 3 + 2]);
      for (int tv = 0; tv < tesselation; tv++)
        {
          float v = (float) tv / tesselation;

          for (int tu = 0; tu < tesselation; tu++)
            {
              float u = (float) tu / tesselation;
              // triângulo superior
              pontos.push_back (getBezierPoint ((u + (1.0f / tesselation)), (v + (1.0f / tesselation)), x));
              pontos.push_back (getBezierPoint (u, (v + (1.0f / tesselation)), x));
              pontos.push_back (getBezierPoint (u, v, x));
              // triângulo inferior
              pontos.push_back (getBezierPoint (u, v, x));
              pontos.push_back (getBezierPoint ((u + (1.0f / tesselation)), v, x));
              pontos.push_back (getBezierPoint ((u + (1.0f / tesselation)), (v + (1.0f / tesselation)), x));
              for (int k = 0; k < 6; k++)
                {
                  res << get<0> (pontos[k]) << " " << get<1> (pontos[k]) << " " << get<2> (pontos[k]) << "\n";
                }
              pontos.clear ();
            }
        }
      x.clear ();
    }
  myFile.close ();
  return res.str ();
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

int main (int argc, char *argv[])
{
  if (argc < 5)
    {
      printf ("Not enough arguments");
    }
  else
    {
      const char *filepath = argv[argc - 1];
      const char *polygon = argv[1];

      if (!strcmp (PLANE, polygon))
        model_plane_write (filepath, strtof (argv[2], nullptr), strtoul (argv[3], nullptr, 10));
      if (!strcmp (CUBE, polygon))
        model_cube_write (filepath, atof (argv[2]), strtoul (argv[3], nullptr, 10));
      if (!strcmp (CONE, polygon))
        model_cone_write (filepath, atof (argv[2]), atof (argv[3]), atoi (argv[4]), atoi (argv[5]));
      if (!strcmp (SPHERE, polygon))
        model_sphere_write (filepath, atof (argv[2]), atoi (argv[3]), atoi (argv[4]));
    }
  return 1;
}