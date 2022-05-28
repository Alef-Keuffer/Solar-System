#include <vector>
#include <stdio.h>
#include <string.h>
#include "tinyxml2.h"
#include "parsing.h"
/*I think this is usually done with macros, but I want to know why not do what I'm doing*/

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

void operations_push_transform_attributes (const XMLElement *transform, std::vector<float> *operations)
{
  if (!transform)
    return;

  float angle = transform->FloatAttribute ("angle");
  if ((int) angle)
    operations->push_back (angle);

  operations->push_back (transform->FloatAttribute ("x"));
  operations->push_back (transform->FloatAttribute ("y"));
  operations->push_back (transform->FloatAttribute ("z"));
}

void
operations_push_extended_translate_attributes (const XMLElement *extended_translate, std::vector<float> *operations)
{
  if (!extended_translate)
    {
      fprintf (stderr, "Null transform\n");
      exit (1);
    }

  float time = extended_translate->FloatAttribute ("time");
  bool align = extended_translate->BoolAttribute ("align");
  int number_of_points = 0;

  operations->push_back ((float) time);
  operations->push_back ((float) align);
  operations->push_back (0);
  auto index_of_number_of_points = operations->size () - 1;

  for (auto child = extended_translate->FirstChildElement ("point"); child; child = child->NextSiblingElement ("point"))
    {
      number_of_points++;
      operations_push_transform_attributes (child, operations);
    }
    operations->at (index_of_number_of_points) = number_of_points;
}

void operations_push_transformation (const XMLElement *transformation, vector<float> *operations)
{
  const char *transform_name = transformation->Value ();

  if (!strcmp ("translate", transform_name))
    {
      if (transformation->Attribute ("time") != NULL)
        {
          operations->push_back (EXTENDED_TRANSLATE);
          operations_push_extended_translate_attributes (transformation, operations);
        }
      else
        {
          operations->push_back (TRANSLATE);
          operations_push_transform_attributes (transformation, operations);
        }
    }
  else if (!strcmp ("rotate", transform_name))
    {
      if (transformation->Attribute ("time")) {
        operations->push_back (EXTENDED_ROTATE);
        float time = transformation->FloatAttribute ("time");
        operations->push_back (time);
        operations_push_transform_attributes (transformation, operations);
      }
      else {
        operations->push_back (ROTATE);
        operations_push_transform_attributes (transformation, operations);
      }
    }
  else if (!strcmp ("scale", transform_name)) {
    operations->push_back (SCALE);
    operations_push_transform_attributes (transformation, operations);
  }
  else
    {
      fprintf (stderr, "Unknown transform: \"%s\"", transform_name);
      exit (1);
    }
}

void operations_push_transforms (XMLElement *transforms, std::vector<float> *operations)
{
  tinyxml2::XMLElement *transform = transforms->FirstChildElement ();
  do
      operations_push_transformation(transform,operations);
  while ((transform = transform->NextSiblingElement ()));
}
//! @} end of group Transforms

/*! @addtogroup Models
 * @{
 */

int operations_push_string_attribute (const XMLElement *element, vector<float> *operations, const char *attribute_name)
{
  const char *element_attribute_value = element->Attribute (attribute_name);
  int i = 0;
  do
    operations->push_back (element_attribute_value[i++]);
  while (element_attribute_value[i]);
  // add number of characters of element attribute string value
  operations->insert (operations->end () - i, (float) i);
  return i;
}

void operations_push_model (const XMLElement *model, vector<float> *operations)
{
  operations->push_back (BEGIN_MODEL);

  int i = operations_push_string_attribute (model,operations,"file");

  if (i <= 0)
    {
      fprintf (stderr, "filename is empty");
      exit (1);
    }

    //texture
    const XMLElement *texture = model->FirstChildElement ("texture");
    if(texture != nullptr)
      {
        operations->push_back (TEXTURE);
        operations_push_string_attribute (texture,operations,"file");
      }
    //color (material colors)
    const XMLElement *color = model->FirstChildElement ("color");
    if (color != nullptr)
      {
        const int n = 4;
        const char* colorNames[n] = {"diffuse", "ambient", "specular", "emissive"};
        const operation_t colorTypes[n] = {DIFFUSE, AMBIENT, SPECULAR, EMISSIVE};
        for (int c = 0; c < n; ++c)
          {
            const XMLElement *color_comp = color->FirstChildElement (colorNames[c]);
            if (color_comp != nullptr)
              {
                operations->push_back (colorTypes[c]);
                operations->insert (operations->end (),
                                    {color_comp->FloatAttribute ("R"),
                                     color_comp->FloatAttribute ("G"),
                                     color_comp->FloatAttribute ("B")});
              }
          }
        const XMLElement *shininess = color->FirstChildElement ("shininess");
        if (shininess != nullptr)
          {
            operations->push_back (SHININESS);
            operations->push_back (shininess->FloatAttribute ("value"));
          }
      }

  operations->push_back (END_MODEL);
}

void operations_push_models (XMLElement *models, std::vector<float> *operations)
{
  tinyxml2::XMLElement *model = models->FirstChildElement ("model");
  do
    operations_push_model (model, operations);
  while ((model = model->NextSiblingElement ("model")));
}
//! @} end of group Models

/*! @addtogroup Groups
 * @{*/
void operations_push_groups (tinyxml2::XMLElement *group, std::vector<float> *operations)
{
  operations->push_back (BEGIN_GROUP);

  // Inside "transform" tag there can be multiple transformations.
  tinyxml2::XMLElement *transforms = group->FirstChildElement ("transform");
  tinyxml2::XMLElement *models = group->FirstChildElement ("models");

  if (transforms)
    operations_push_transforms (transforms, operations);
  if (models)
    operations_push_models (models, operations);

  tinyxml2::XMLElement *childGroup = group->FirstChildElement ("group");
  if (childGroup)
    do
      operations_push_groups (childGroup, operations);
    while ((childGroup = childGroup->NextSiblingElement ("group")));

  operations->push_back (END_GROUP);
}
//! @} end of group Groups

/*! @addtogroup xml
 *@{*/


void operations_push_lights (const XMLElement *lights, vector<float> *operations)
{
  const XMLElement *light = lights->FirstChildElement ();
  do
    {
      const char* lightType = light->Attribute ("type");
      if (!strcmp(lightType,"point")) {
        operations->push_back (POINT);
        operations->insert(operations->end(),
                           {light->FloatAttribute ("posX"),
                            light->FloatAttribute ("posY"),
                            light->FloatAttribute ("posZ")});

      }
      else if (!strcmp(lightType,"directional"))
        {
          operations->push_back (DIRECTIONAL);
          operations->insert(operations->end(),
                             {light->FloatAttribute ("dirX"),
                              light->FloatAttribute ("dirY"),
                              light->FloatAttribute ("dirZ")});
        }
      else if (!strcmp(lightType,"spotlight"))
        {
          operations->push_back (SPOTLIGHT);
          operations->insert(operations->end(),
                             {light->FloatAttribute ("posX"),
                              light->FloatAttribute ("posY"),
                              light->FloatAttribute ("posZ"),
                              light->FloatAttribute ("dirX"),
                              light->FloatAttribute ("dirY"),
                              light->FloatAttribute ("dirZ"),
                              light->FloatAttribute ("cutoff")});
        }
      else
        {
          fprintf(stderr, "Unkown light type: %s\n",lightType);
          exit(EXIT_FAILURE);
        }
    }
  while ((light = light->NextSiblingElement ()));
}

void operations_load_xml (const char *filename, std::vector<float> *operations)
{
  tinyxml2::XMLDocument doc;

  if (doc.LoadFile (filename))
    {
      if (doc.ErrorID () == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        fprintf (stderr, "Failed loading file: '%s'\n", filename);
      fprintf (stderr, "%s", doc.ErrorName ());
      exit (1);
    }

  fprintf (stderr, "Loaded file: '%s'\n", filename);
  tinyxml2::XMLElement *world = doc.FirstChildElement ("world");

  /*camera*/
  tinyxml2::XMLElement *camera = world->FirstChildElement ("camera");

  tinyxml2::XMLElement *position = camera->FirstChildElement ("position");
  operations_push_transform_attributes (position, operations);

  tinyxml2::XMLElement *lookAt = camera->FirstChildElement ("lookAt");
  operations_push_transform_attributes (lookAt, operations);

  tinyxml2::XMLElement *up = camera->FirstChildElement ("up");
  if (up)
    operations_push_transform_attributes (up, operations);
  else
    operations->insert (operations->end (), {0, 1, 0});

  tinyxml2::XMLElement *projection = camera->FirstChildElement ("projection");
  if (projection)
    {
      operations->push_back (projection->FloatAttribute ("fov"));
      operations->push_back (projection->FloatAttribute ("near"));
      operations->push_back (projection->FloatAttribute ("far"));
    }
  else
    operations->insert (operations->end (), {60, 1, 1000});
  /*end of camera*/

  // lights
  const XMLElement *lights = world->FirstChildElement ("lights");
  if (lights != nullptr)
    operations_push_lights (lights,operations);

  // groups
  tinyxml2::XMLElement *group = world->FirstChildElement ("group");
  operations_push_groups (group, operations);
}

//! @} end of group xml

/*! @addtogroup Printing
 * @{*/
void operations_print (std::vector<float> *operations)
{
  unsigned int i = 0;
  fprintf (stderr,
           "POSITION(%.2f %.2f %.2f)\n",
           operations->at (i),
           operations->at (i + 1),
           operations->at (i + 2));
  i += 3;
  fprintf (stderr,
           "LOOK_AT(%.2f %.2f %.2f)\n",
           operations->at (i),
           operations->at (i + 1),
           operations->at (i + 2));
  i += 3;
  fprintf (stderr,
           "UP(%.2f %.2f %.2f)\n",
           operations->at (i),
           operations->at (i + 1),
           operations->at (i + 2));
  i += 3;
  fprintf (stderr,
           "PROJECTION(%.2f %.2f %.2f)\n",
           operations->at (i),
           operations->at (i + 1),
           operations->at (i + 2));

  for (i += 3; i < operations->size (); i++)
    {
      switch ((operation_t) operations->at (i))
        {
          case ROTATE:
            fprintf (stderr,
                     "ROTATE(%.2f %.2f %.2f %.2f)\n",
                     operations->at (i + 1),
                     operations->at (i + 2),
                     operations->at (i + 3),
                     operations->at (i + 4));
          i += 4;
          continue;
          case TRANSLATE:
            fprintf (stderr,
                     "TRANSLATE(%.2f %.2f %.2f)\n",
                     operations->at (i + 1),
                     operations->at (i + 2),
                     operations->at (i + 3));
          i += 3;
          continue;
          case SCALE:
            fprintf (stderr,
                     "SCALE(%.2f %.2f %.2f)\n",
                     operations->at (i + 1),
                     operations->at (i + 2),
                     operations->at (i + 3));
          i += 3;
          continue;
          case BEGIN_GROUP:
            fprintf (stderr, "BEGIN_GROUP\n");
          continue;
          case END_GROUP:
            fprintf (stderr, "END_GROUP\n");
          continue;
          case BEGIN_MODEL:
            {
              int stringSize = (int) operations->at (i + 1);
              char model[stringSize + 1];

              int j;
              for (j = 0; j < stringSize; j++)
                model[j] = (char) operations->at (i + 2 + j);

              model[j] = 0;

              fprintf (stderr, "BEGIN_MODEL(%s)\n", model);
              i += 1 + j - 1; //just to be explicit
            }
          continue;
          default:
            {
              fprintf(stderr,"Unknown opeartion %d",(int) operations->at (i));
              exit(EXIT_FAILURE);
            }
        }
    }
}
//! @} end of group Printing
//! @} end of group Operations

void example1 (const char *filename)
{
  tinyxml2::XMLDocument doc;

  if (doc.LoadFile (filename))
    {
      fprintf (stderr, "%s", doc.ErrorName ());
      exit (1);
    }

  std::vector<float> operations;
  operations_load_xml (filename, &operations);
  operations_print (&operations);
}

int main2 (int argc, char *argv[])
{
  example1 ("test_files_phase_2/test_2_3.xml");
  return 1;
}