#ifndef PROJ_PARSING_H
#define PROJ_PARSING_H

#include <vector>

void operations_load_xml (const std::string &filename, std::vector<float> &operations);

enum {
  TRANSLATE = 1,
  ROTATE,
  SCALE,
  BEGIN_MODEL,
  END_MODEL,
  BEGIN_GROUP,
  END_GROUP,
  EXTENDED_TRANSLATE,
  EXTENDED_ROTATE,
  TEXTURE,
  DIFFUSE,
  AMBIENT,
  SPECULAR,
  EMISSIVE,
  SHININESS,
  POINT,
  DIRECTIONAL,
  SPOTLIGHT,
  POSITION,
  LIGHT,
  LOOK_AT,
  UP,
  PROJECTION,
  MODEL,
  GROUP,
  GENERATOR,
  OTHER
};

typedef unsigned char operation_t;
#endif //PROJ_PARSING_H
