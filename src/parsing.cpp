#include <cstdio>
#include <cstring>

#include <vector>
#include <iostream>
#include <csignal>

#include "tinyxml2.h"
#include "parsing.h"

/*! @addtogroup Operations
 * @{
 * # Data structure for Operations
 *
 * @code{.unparsed}
 * ⟨operations⟩ ::= ⟨position⟩⟨lookAt⟩⟨up⟩⟨projection⟩⟨light⟩⃰ ⟨grouping⟩⁺
 *      ⟨position⟩,⟨lookAt⟩,⟨up⟩,⟨projection⟩ ::= ⟨vec3f⟩
 *       ⟨light⟩ ::= ⟨point⟩ | ⟨directional⟩ | ⟨spotlight⟩
 *            ⟨point⟩ ::= ⟨POINT⟩⟨vec3f⟩
 *            ⟨directional⟩ ::= ⟨DIRECTIONAL⟩⟨vec3f⟩
 *            ⟨spotlight⟩ ::= ⟨SPOTLIGHT⟩⟨vec3f⟩⟨vec3f⟩⟨cutoff⟩
 *                ⟨cutoff⟩ ::= ⟨float⟩ ∈ [0,90] ∪ {180}
 *
 * ⟨grouping⟩ ::= ⟨BEGIN_GROUP⟩⟨elem⟩⁺⟨END_GROUP⟩
 *      ⟨elem⟩ ::= ⟨transformation⟩ | ⟨model_loading⟩ | ⟨grouping⟩
 *
 * ⟨transformation⟩ ::= ⟨translation⟩ | ⟨rotation⟩ | ⟨scaling⟩
 *      ⟨translation⟩ ::= ⟨simple_translation⟩ | ⟨extended_translation⟩
 *           ⟨simple_translation⟩ ::= ⟨TRANSLATE⟩⟨float⟩⟨float⟩⟨float⟩
 *           ⟨extended_translation⟩ ::= ⟨EXTENDED_TRANSLATE⟩⟨time⟩⟨align⟩⟨number_of_points⟩⟨vec3f⟩⁺
 *               ⟨time⟩  ::= ⟨float⟩
 *               ⟨align⟩ ::= ⟨bool⟩
 *               ⟨number_of_points⟩ ::= ⟨int⟩
 *      ⟨rotation⟩ ::= ⟨simple_rotation⟩ | ⟨extended_rotation⟩
 *           ⟨simple_rotation⟩ ::= ⟨ROTATE⟩⟨float⟩⟨float⟩⟨float⟩[angle]
 *               ⟨angle⟩ ::= ⟨float⟩
 *           ⟨extended_rotation⟩ ::= ⟨EXTENDED_ROTATE⟩⟨vec3f⟩
 *      ⟨scaling⟩ ::= ⟨SCALE⟩⟨float⟩⟨float⟩⟨float⟩
 *
 * ⟨model_loading⟩ ::= ⟨BEGIN_MODEL⟩ ⟨number of characters⟩ ⟨char⟩⁺ [texture] [color] ⟨END_MODEL⟩
 *      ⟨number of characters⟩ ::= ⟨int⟩
 *
 * ⟨texture⟩ ::= ⟨TEXTURE⟩ ⟨number of characters⟩ ⟨char⟩⁺
 * ⟨color⟩   ::=  (⟨DIFFUSE⟩ | ⟨AMBIENT⟩ | ⟨SPECULAR⟩ | ⟨EMISSIVE⟩) ⟨color_vec3f⟩
 *              | ⟨SHININESS⟩ ⟨shininess_float⟩
 *      ⟨color_vec3f⟩ ::= ⟨red⟩⟨green⟩⟨blue⟩
 *          ⟨red⟩,⟨green⟩,⟨blue⟩ ::= ⟨float⟩ ∈ {0,...,255}
 *      ⟨shininess_float⟩ ::= ⟨float⟩ ∈ [0, 128]
 *
 * ⟨vec3f⟩ ::= ⟨float⟩⟨float⟩⟨float⟩
 * @endcode
 *
 * Note: we'll use an array of floats, therefore we will need to cast to char when reading
 * the model filename characters and to int when reading the number of characters.
 */

/*! @addtogroup Transforms
 * @{*/

using std::vector;
using tinyxml2::XMLElement;
using tinyxml2::XMLDocument, tinyxml2::XML_SUCCESS, tinyxml2::XMLError, tinyxml2::XML_NO_ATTRIBUTE;
using std::cerr, std::endl;

void operations_push_transform_attributes (
    const XMLElement *const transform,
    vector<float> &operations)
{
  if (!transform)
    {
      fprintf (stderr, "[parsing] operations_push_transform_attributes: null transform\n");
      exit (EXIT_FAILURE);
    }

  float angle;
  {
    const XMLError e = transform->QueryFloatAttribute ("angle", &angle);
    if (e == XML_SUCCESS)
      operations.push_back (angle);
    else if (e != XML_NO_ATTRIBUTE)
      {
        fprintf (stderr, "[parsing] Parsing error %s at %s\n", transform->GetText (), XMLDocument::ErrorIDToName (e));
        exit (EXIT_FAILURE);
      }
  }
  for (const char *const attribute_name : {"x", "y", "z"})
    {
      float attributeValue;
      XMLError e;
      if ((e = transform->QueryFloatAttribute (attribute_name, &attributeValue)) != XML_SUCCESS)
        {
          fprintf (stderr, "[parsing] Parsing error %s at %s\n", transform->GetText (), XMLDocument::ErrorIDToName (e));
          exit (EXIT_FAILURE);
        }
      operations.push_back (attributeValue);
    }

}

void
operations_push_extended_translate_attributes (
    const XMLElement *const extended_translate,
    vector<float> &operations)
{
  if (!extended_translate)
    {
      fprintf (stderr, "[parsing] Null transform\n");
      exit (EXIT_FAILURE);
    }

  float time;
  if (extended_translate->QueryFloatAttribute ("time", &time))
    {
      cerr << "[parsing] failed parsing time" << endl;
      exit (EXIT_FAILURE);
    }
  bool align;
  if (extended_translate->QueryBoolAttribute ("align", &align))
    {
      cerr << "[parsing] failed parsing align" << endl;
      exit (EXIT_FAILURE);
    }

  operations.push_back ((float) time);
  operations.push_back ((float) align);
  operations.push_back (0); // create space to insert the number of points
  auto index_of_number_of_points = operations.size () - 1;

  int number_of_points = 0;
  for (auto child = extended_translate->FirstChildElement ("point"); child; child = child->NextSiblingElement ("point"))
    {
      ++number_of_points;
      operations_push_transform_attributes (child, operations);
    }
  operations[index_of_number_of_points] = (float) number_of_points;
}

void operations_push_transformation (const XMLElement *const transformation, vector<float> &operations)
{
  const char *const transformation_name = transformation->Value ();

  if (!strcmp ("translate", transformation_name))
    {
      if (transformation->Attribute ("time") != nullptr)
        {
          operations.push_back (EXTENDED_TRANSLATE);
          cerr << "[parsing] EXTENDED_TRANSLATE" << endl;
          operations_push_extended_translate_attributes (transformation, operations);

        }
      else
        {
          operations.push_back (TRANSLATE);
          cerr << "[parsing] TRANSLATE" << endl;
          operations_push_transform_attributes (transformation, operations);
        }
    }
  else if (!strcmp ("rotate", transformation_name))
    {
      if (transformation->Attribute ("time"))
        {
          operations.push_back (EXTENDED_ROTATE);
          cerr << "[parsing] EXTENDED_ROTATE" << endl;
          float time;
          if (transformation->QueryFloatAttribute ("time", &time))
            {
              cerr << "[parsing] failed parsing time" << endl;
              exit (EXIT_FAILURE);
            }
          operations.push_back (time);
          operations_push_transform_attributes (transformation, operations);
        }
      else
        {
          operations.push_back (ROTATE);
          cerr << "[parsing] ROTATE" << endl;
          operations_push_transform_attributes (transformation, operations);
        }
    }
  else if (!strcmp ("scale", transformation_name))
    {
      operations.push_back (SCALE);
      cerr << "[parsing] SCALE" << endl;
      operations_push_transform_attributes (transformation, operations);
    }
  else
    {
      fprintf (stderr, "[parsing] Unknown transformation: \"%s\"", transformation_name);
      exit (EXIT_FAILURE);
    }
}

void operations_push_transforms (const XMLElement *const transforms, vector<float> &operations)
{
  const XMLElement *transform = transforms->FirstChildElement ();
  do
    operations_push_transformation (transform, operations);
  while ((transform = transform->NextSiblingElement ()));
}
//! @} end of group Transforms

/*! @addtogroup Models
 * @{
 */

int operations_push_string_attribute (
    const XMLElement *const element,
    vector<float> &operations,
    const char *const attribute_name)
{

  const char *const element_attribute_value = element->Attribute (attribute_name);
  if (element_attribute_value == nullptr)
    {
      cerr << "[parsing] [operations_push_string_attribute] failed: attrbute " << attribute_name << " does not exist"
           << endl;
      exit (EXIT_FAILURE);
    }
  if (access (element_attribute_value, F_OK))
    {
      cerr << "[parsing] file " << element_attribute_value << " not found" << endl;
      exit (EXIT_FAILURE);
    }
  int i = 0;
  do
    operations.push_back (element_attribute_value[i++]);
  while (element_attribute_value[i]);
  // add number of characters of element attribute string value
  operations.insert (operations.end () - i, (float) i);
  return i;
}

void operations_push_model (const XMLElement *const model, vector<float> &operations)
{
  operations.push_back (BEGIN_MODEL);

  {
    const int string_len = operations_push_string_attribute (model, operations, "file");

    if (string_len <= 0)
      {
        fprintf (stderr, "[parsing] filename is empty");
        exit (EXIT_FAILURE);
      }
  }

  //texture
  const XMLElement *const texture = model->FirstChildElement ("texture");
  if (texture != nullptr)
    {
      operations.push_back (TEXTURE);
      operations_push_string_attribute (texture, operations, "file");
    }
  //color (material colors)
  const XMLElement *const color = model->FirstChildElement ("color");
  if (color != nullptr)
    {
      const int n = 4;
      const char *const colorNames[n] = {"diffuse", "ambient", "specular", "emissive"};
      const operation_t colorTypes[n] = {DIFFUSE, AMBIENT, SPECULAR, EMISSIVE};
      for (int c = 0; c < n; ++c)
        {
          const XMLElement *const color_comp = color->FirstChildElement (colorNames[c]);
          if (color_comp != nullptr)
            {
              const float RGB_MAX = 255.0f;
              operations.push_back (colorTypes[c]);
              float R;
              if (color_comp->QueryFloatAttribute ("R", &R))
                {
                  cerr << "[parsing] failed parsing R component" << endl;
                  exit (EXIT_FAILURE);
                }
              float G;
              if (color_comp->QueryFloatAttribute ("G", &G))
                {
                  cerr << "[parsing] failed parsing G component" << endl;
                  exit (EXIT_FAILURE);
                }
              float B;
              if (color_comp->QueryFloatAttribute ("B", &B))
                {
                  cerr << "[parsing] failed parsing B component" << endl;
                  exit (EXIT_FAILURE);
                }
              operations.insert (operations.end (),
                                 {R / RGB_MAX, G / RGB_MAX, B / RGB_MAX});
            }
        }
      const XMLElement *const shininess = color->FirstChildElement ("shininess");
      if (shininess != nullptr)
        {
          operations.push_back (SHININESS);
          float value;
          if (shininess->QueryFloatAttribute ("value", &value))
            {
              cerr << "[parsing] failed parsing value attribute of shininess" << endl;
              exit (EXIT_FAILURE);
            }
          operations.push_back (value);
        }
    }

  operations.push_back (END_MODEL);
}

void operations_push_models (const XMLElement *const models, vector<float> &operations)
{
  const XMLElement *model = models->FirstChildElement ("model");
  do
    operations_push_model (model, operations);
  while ((model = model->NextSiblingElement ("model")));
}
//! @} end of group Models

/*! @addtogroup Groups
 * @{*/
void operations_push_groups (const XMLElement *const group, vector<float> &operations)
{
  operations.push_back (BEGIN_GROUP);

  // Inside "transform" tag there can be multiple transformations.
  const XMLElement *const transforms = group->FirstChildElement ("transform");
  const XMLElement *const models = group->FirstChildElement ("models");

  if (transforms)
    operations_push_transforms (transforms, operations);
  if (models)
    operations_push_models (models, operations);

  const XMLElement *childGroup = group->FirstChildElement ("group");
  if (childGroup)
    do
      operations_push_groups (childGroup, operations);
    while ((childGroup = childGroup->NextSiblingElement ("group")));

  operations.push_back (END_GROUP);
}
//! @} end of group Groups

/*! @addtogroup xml
 *@{*/


void operations_push_lights (const XMLElement *const lights, vector<float> &operations)
{
  const XMLElement *light = lights->FirstChildElement ();
  do
    {
      const char *lightType;
      if (light->QueryAttribute ("type", &lightType))
        {
          cerr << "failed parsing type attribute of light" << endl;
          exit (EXIT_FAILURE);
        }
      if (!strcmp (lightType, "point"))
        {
          operations.push_back (POINT);
          float posX, posY, posZ;
          if (light->QueryFloatAttribute ("posX", &posX)
              | light->QueryFloatAttribute ("posY", &posY)
              | light->QueryFloatAttribute ("posZ", &posZ))
            {
              cerr << "[parsing] Failed parsing POINT posX or posY or posZ" << endl;
              exit (EXIT_FAILURE);
            }
          operations.insert (operations.end (), {posX, posY, posZ});

        }
      else if (!strcmp (lightType, "directional"))
        {
          operations.push_back (DIRECTIONAL);
          float dirX, dirY, dirZ;
          if (light->QueryFloatAttribute ("dirX", &dirX)
              | light->QueryFloatAttribute ("dirY", &dirY)
              | light->QueryFloatAttribute ("dirZ", &dirZ))
            {
              cerr << "[parsing] Failed parsing DIRECTIONAL dirX or dirY or dirZ" << endl;
              exit (EXIT_FAILURE);
            }
          operations.insert (operations.end (), {dirX, dirY, dirZ});
        }
      else if (!strcmp (lightType, "spotlight"))
        {
          operations.push_back (SPOTLIGHT);
          float posX, posY, posZ;
          if (light->QueryFloatAttribute ("posX", &posX)
              | light->QueryFloatAttribute ("posY", &posY)
              | light->QueryFloatAttribute ("posZ", &posZ))
            {
              cerr << "[parsing] Failed parsing SPOTLIGHT posX or posY or posZ" << endl;
              exit (EXIT_FAILURE);
            }

          float dirX, dirY, dirZ;
          if (light->QueryFloatAttribute ("dirX", &dirX)
              | light->QueryFloatAttribute ("dirY", &dirY)
              | light->QueryFloatAttribute ("dirZ", &dirZ))
            {
              cerr << "[parsing] Failed parsing SPOTLIGHT dirX or dirY or dirZ" << endl;
              exit (EXIT_FAILURE);
            }
          float cutoff;
          if (light->QueryFloatAttribute ("cutoff", &cutoff))
            {
              cerr << "[parsing] Failed parsing SPOTLIGHT cutoff" << endl;
              exit (EXIT_FAILURE);
            }
          operations.insert (operations.end (), {
              posX, posY, posZ,
              dirX, dirY, dirZ,
              cutoff
          });
        }
      else
        {
          fprintf (stderr, "[parsing] Unkown light type: %s\n", lightType);
          exit (EXIT_FAILURE);
        }
    }
  while ((light = light->NextSiblingElement ()));
}

void operations_load_xml (const char *const filename, vector<float> &operations)
{
  XMLDocument doc;

  if (doc.LoadFile (filename))
    {
      if (doc.ErrorID () == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        fprintf (stderr, "[parsing] Failed loading file: '%s'\n", filename);
      fprintf (stderr, "%s", doc.ErrorName ());
      exit (EXIT_FAILURE);
    }

  fprintf (stderr, "[parsing] Loaded file: '%s'\n", filename);
  const XMLElement *const world = doc.FirstChildElement ("world");

  /*camera*/
  const XMLElement *const camera = world->FirstChildElement ("camera");

  const XMLElement *const position = camera->FirstChildElement ("position");
  operations_push_transform_attributes (position, operations);

  const XMLElement *const lookAt = camera->FirstChildElement ("lookAt");
  operations_push_transform_attributes (lookAt, operations);

  const XMLElement *const up = camera->FirstChildElement ("up");
  if (up)
    operations_push_transform_attributes (up, operations);
  else
    operations.insert (operations.end (), {0, 1, 0});

  const XMLElement *const projection = camera->FirstChildElement ("projection");
  if (projection)
    {
      float fov;
      if (projection->QueryFloatAttribute ("fov", &fov))
        {
          cerr << "[parsing] Failed parsing fov" << endl;
          exit (EXIT_FAILURE);
        }
      float near;
      if (projection->QueryFloatAttribute ("near", &near))
        {
          cerr << "[parsing] Failed parsing near" << endl;
          exit (EXIT_FAILURE);
        }
      float far;
      if (projection->QueryFloatAttribute ("far", &far))
        {
          cerr << "[parsing] Failed parsing far" << endl;
          exit (EXIT_FAILURE);
        }
      operations.insert (operations.end (), {fov, near, far});
    }
  else
    operations.insert (operations.end (), {60, 1, 1000});
  /*end of camera*/

  // lights
  const XMLElement *const lights = world->FirstChildElement ("lights");
  if (lights != nullptr)
    operations_push_lights (lights, operations);

  // groups
  const XMLElement *const group = world->FirstChildElement ("group");
  operations_push_groups (group, operations);
}

//! @} end of group xml

//! @} end of group Operations