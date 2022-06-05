#include <cstdio>

#include <string>
#include <filesystem>
#include <iostream>

#include <vector>

#ifndef USE_SYSTEM
#include <sys/wait.h>
#include <wordexp.h>
#include <cassert>
#else
#include <filesystem>
using std::filesystem::current_path;
#endif

#include "tinyxml2.h"
#include "parsing.h"

/*! @addtogroup Operations
 * @{
 * # Data structure for Operations
 * ⟨position⟩⟨lookAt⟩⟨up⟩⟨projection⟩⟨light⟩⃰
 * @code{.unparsed}
 * ⟨nfloat⟩ ::= ⟨float⟩ ∈ [0,1]
 * ⟨nvec3f⟩ ::= ⟨nfloat⟩⟨nfloat⟩⟨nfloat⟩
 * ⟨vec3f⟩  ::= ⟨float⟩⟨float⟩⟨float⟩
 *
 *
 *
 * ⟨operations⟩ ::= ⟨grouping⟩⁺
 *
 * ⟨grouping⟩ ::= ⟨BEGIN_GROUP⟩⟨elem⟩⁺⟨END_GROUP⟩
 *      ⟨elem⟩ ::= ⟨transformation⟩| ⟨model_loading⟩ | ⟨light⟩
 *                | ⟨position⟩ | ⟨lookAt⟩ | ⟨up⟩ | ⟨projection⟩
 *                | ⟨grouping⟩
 *
 *      ⟨position⟩ ::= ⟨POSITION⟩ ⟨vec3f⟩
 *      ⟨lookAt⟩ ::= ⟨lookAt⟩ ⟨vec3f⟩
 *      ⟨up⟩ ::= ⟨UP⟩ ⟨vec3f⟩
 *      ⟨projection⟩ ::= ⟨PROJECTION⟩ ⟨vec3f⟩

 * ⟨light⟩ ::= ⟨point⟩ | ⟨directional⟩ | ⟨spotlight⟩
 *     ⟨point⟩        ::= ⟨POINT⟩⟨nvec3f⟩
 *     ⟨directional⟩  ::= ⟨DIRECTIONAL⟩⟨nvec3f⟩
 *     ⟨spotlight⟩    ::= ⟨SPOTLIGHT⟩⟨nvec3f⟩⟨nvec3f⟩⟨cutoff⟩
 *     ⟨cutoff⟩       ::= ⟨float⟩ ∈ [0,90] ∪ {180}
 *
 * ⟨transformation⟩ ::= ⟨translation⟩ | ⟨rotation⟩ | ⟨scaling⟩
 *
 *      ⟨translation⟩ ::= ⟨simple_translation⟩ | ⟨extended_translation⟩
 *           ⟨simple_translation⟩   ::= ⟨TRANSLATE⟩⟨vec3f⟩
 *           ⟨extended_translation⟩ ::= ⟨EXTENDED_TRANSLATE⟩⟨time⟩⟨align⟩⟨number_of_points⟩⟨vec3f⟩⁺
 *               ⟨time⟩             ::= ⟨float⟩
 *               ⟨align⟩            ::= ⟨bool⟩
 *               ⟨number_of_points⟩ ::= ⟨int⟩
 *
 *      ⟨rotation⟩ ::= ⟨simple_rotation⟩ | ⟨extended_rotation⟩
 *           ⟨simple_rotation⟩ ::= ⟨ROTATE⟩⟨vec3f⟩[angle]
 *               ⟨angle⟩ ::= ⟨float⟩
 *           ⟨extended_rotation⟩ ::= ⟨EXTENDED_ROTATE⟩⟨vec3f⟩
 *
 *      ⟨scaling⟩ ::= ⟨SCALE⟩⟨vec3f⟩
 *
 *
 * ⟨model_loading⟩ ::= ⟨BEGIN_MODEL⟩ ⟨number of characters⟩ ⟨char⟩⁺ [texture] [color] ⟨END_MODEL⟩
 *      ⟨number of characters⟩ ::= ⟨int⟩
 *
 * ⟨texture⟩ ::= ⟨TEXTURE⟩ ⟨number of characters⟩ ⟨char⟩⁺
 * ⟨color⟩   ::=  (⟨DIFFUSE⟩ | ⟨AMBIENT⟩ | ⟨SPECULAR⟩ | ⟨EMISSIVE⟩) ⟨nvec3f⟩
 *              | ⟨SHININESS⟩ ⟨shininess_float⟩
 *      ⟨shininess_float⟩ ::= ⟨float⟩ ∈ [0, 128]
 *
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
using std::string;

string globalGeneratorExecutable;
bool globalUsingGenerator = false;

using std::string, std::filesystem::exists, std::cerr, std::endl;

void crash_if_file_does_not_exist (const string &filename, const string &additional_info = "")
{
  if (!exists (filename))
    {
      cerr << "[parsing] Failed loading " << filename << " " << additional_info << endl;
      exit (EXIT_FAILURE);
    }
}

void crashIfFailedAttributeQuery (const XMLError error, const XMLElement &element, const string &attributeName)
{
  if (!error)
    return;
  cerr << "[parsing] failed parsing attribute " << attributeName << " of element " << element.Value ()
       << "\nError: " << XMLDocument::ErrorIDToName (error) << endl;
  exit (EXIT_FAILURE);
}

float getFloatAttribute (const XMLElement &e, const string &attributeName)
{
  float attributeValue;
  XMLError error = e.QueryFloatAttribute (attributeName.c_str (), &attributeValue);
  crashIfFailedAttributeQuery (error, e, attributeName);
  return attributeValue;
}


template<typename T>
T getAttribute (const XMLElement &e, const string &attributeName)
{
  T _attributeValue;
  XMLError error;

  if constexpr(std::is_same<string, T>::value)
    {
      const char **attributeValue;
      error = e.QueryStringAttribute (attributeName.c_str (), attributeValue);
      _attributeValue = string{*attributeValue};
    }
  else if constexpr(std::is_same<float, T>::value)
    {
      float attributeValue;
      error = e.QueryFloatAttribute (attributeName.c_str (), &attributeValue);
      _attributeValue = attributeValue;
    }
  else if constexpr(std::is_same<bool, T>::value)
    {
      bool attributeValue;
      error = e.QueryBoolAttribute (attributeName.c_str (), &attributeValue);
      _attributeValue = attributeValue;
    }

  crashIfFailedAttributeQuery (error, e, attributeName);
  return _attributeValue;
}

void operations_push_transform_attributes (
    const XMLElement &transform,
    vector<float> &operations)
{

  float angle;
  {
    const XMLError e = transform.QueryFloatAttribute ("angle", &angle);
    if (e == XML_SUCCESS)
      operations.push_back (angle);
    else if (e != XML_NO_ATTRIBUTE)
      {
        fprintf (stderr, "[parsing] Parsing error %s at %s\n", transform.Value (), XMLDocument::ErrorIDToName (e));
        exit (EXIT_FAILURE);
      }
  }
  for (const string attribute_name : {"x", "y", "z"})
    {
      float attributeValue = getFloatAttribute (transform, attribute_name);
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

  auto time = getAttribute<float> (*extended_translate, "time");
  auto align = getAttribute<bool> (*extended_translate, "align");

  operations.push_back ((float) time);
  operations.push_back ((float) align);
  operations.push_back (0); // create space to insert the number of points
  auto index_of_number_of_points = operations.size () - 1;

  int number_of_points = 0;
  for (auto child = extended_translate->FirstChildElement ("point"); child; child = child->NextSiblingElement ("point"))
    {
      ++number_of_points;
      operations_push_transform_attributes (*child, operations);
    }
  operations[index_of_number_of_points] = (float) number_of_points;
}

void operations_push_transformation (const XMLElement *const transformation, vector<float> &operations)
{
  assert (transformation != nullptr);
  const string transformation_name = transformation->Value ();

  if ("translate" == transformation_name)
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
          operations_push_transform_attributes (*transformation, operations);
        }
    }
  else if ("rotate" == transformation_name)
    {
      if (transformation->Attribute ("time"))
        {
          operations.push_back (EXTENDED_ROTATE);
          cerr << "[parsing] EXTENDED_ROTATE" << endl;
          auto time = getAttribute<float> (*transformation, "time");
          operations.push_back (time);
          operations_push_transform_attributes (*transformation, operations);
        }
      else
        {
          operations.push_back (ROTATE);
          cerr << "[parsing] ROTATE" << endl;
          operations_push_transform_attributes (*transformation, operations);
        }
    }
  else if ("scale" == transformation_name)
    {
      operations.push_back (SCALE);
      cerr << "[parsing] SCALE" << endl;
      operations_push_transform_attributes (*transformation, operations);
    }
  else
    {
      cerr << "[parsing] Unknown transformation: \"" << transformation_name << "\"" << endl;
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
    const XMLElement &element,
    vector<float> &operations,
    string &&attribute_name)
{

  auto element_attribute_value = getAttribute<string> (element, attribute_name);
  int i = 0;
  do
    operations.push_back (element_attribute_value[i++]);
  while (element_attribute_value[i]);
  // add number of characters of element attribute string value
  operations.insert (operations.end () - i, (float) i);
  return i;
}

void operations_push_model (const XMLElement &model, vector<float> &operations)
{
  operations.push_back (BEGIN_MODEL);
  {

    auto model_name = getAttribute<string> (model, "file");
    cerr << "[parsing]: BEGIN_MODEL(" << model_name << ")" << endl;

    // generate model if specified
    const XMLElement *const generator = model.FirstChildElement ("generator");

    if (globalUsingGenerator && generator != nullptr)
      {
        cerr << "[parsing] generating model " << model_name << endl;
        auto generator_argv = getAttribute<string> (*generator, "argv");
#ifndef USE_SYSTEM
        int stat;
        if ((stat = fork ()) == 0)
          {
            wordexp_t p;
            wordexp ("generator", &p, WRDE_NOCMD | WRDE_UNDEF);
            if (wordexp (generator_argv.c_str (), &p, WRDE_NOCMD | WRDE_UNDEF | WRDE_APPEND))
              {
                cerr << "[parsing] failed argv expansion for model " << model_name << endl;
                exit (EXIT_FAILURE);
              }
            execv (globalGeneratorExecutable.c_str (), p.we_wordv);
            perror ("[operations_push_model child] generator failed");
            _exit (EXIT_FAILURE);
          }
        else if (stat == -1)
          {
            perror ("[operations_push_model] ");
            exit (EXIT_FAILURE);
          }
        int status;
        wait (&status);
        if (WIFEXITED(status) && WEXITSTATUS(status))
          {
            perror ("[operations_push_model] generator failed");
            exit (EXIT_FAILURE);
          }
      }
#else
    std::stringstream command;
    command << current_path() << "/" << globalGeneratorExecutable << " " << *generator_argv;
    if(system(command.str().data()))
      {
        cerr << "generator failed at model " << model_name << endl;
        exit(EXIT_FAILURE);
      }
#endif
    const int string_len = operations_push_string_attribute (model, operations, "file");

    if (string_len <= 0)
      {
        cerr << "[parsing] filename is empty" << endl;
        exit (EXIT_FAILURE);
      }
  }

  //texture
  const XMLElement *const texture = model.FirstChildElement ("texture");
  if (texture != nullptr)
    {
      operations.push_back (TEXTURE);
      operations_push_string_attribute (*texture, operations, "file");
    }
  //color (material colors)
  const XMLElement *const color = model.FirstChildElement ("color");
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
              auto R = getAttribute<float> (*color_comp, "R");
              auto G = getAttribute<float> (*color_comp, "G");
              auto B = getAttribute<float> (*color_comp, "B");
              operations.insert (operations.end (), {R / RGB_MAX, G / RGB_MAX, B / RGB_MAX});
            }
        }
      const XMLElement *const shininess = color->FirstChildElement ("shininess");
      if (shininess != nullptr)
        {
          operations.push_back (SHININESS);
          auto value = getAttribute<float> (*shininess, "value");
          operations.push_back (value);
        }
    }

  operations.push_back (END_MODEL);
}

void operations_push_models (const XMLElement *const models, vector<float> &operations)
{
  const XMLElement *model = models->FirstChildElement ("model");
  do
    operations_push_model (*model, operations);
  while ((model = model->NextSiblingElement ("model")));
}
//! @} end of group Models

/*! @addtogroup Groups
 * @{*/
void operations_push_groups (const XMLElement &group, vector<float> &operations)
{
  operations.push_back (BEGIN_GROUP);

  // Inside "transform" tag there can be multiple transformations.
  const XMLElement *const transforms = group.FirstChildElement ("transform");
  const XMLElement *const models = group.FirstChildElement ("models");

  if (transforms)
    operations_push_transforms (transforms, operations);
  if (models)
    operations_push_models (models, operations);

  const XMLElement *childGroup = group.FirstChildElement ("group");
  if (childGroup)
    do
      operations_push_groups (*childGroup, operations);
    while ((childGroup = childGroup->NextSiblingElement ("group")));

  operations.push_back (END_GROUP);
}
//! @} end of group Groups

/*! @addtogroup xml
 *@{*/


void operations_push_lights (const XMLElement &lights, vector<float> &operations)
{
  const XMLElement *light = lights.FirstChildElement ();
  do
    {
      auto lightType = getAttribute<string> (*light, "type");
      if ("point" == lightType)
        {
          operations.push_back (POINT);
          auto posX = getAttribute<float> (*light, "posX");
          auto posY = getAttribute<float> (*light, "posY");
          auto posZ = getAttribute<float> (*light, "posZ");
          operations.insert (operations.end (), {posX, posY, posZ});

        }
      else if ("directional" == lightType)
        {
          operations.push_back (DIRECTIONAL);

          auto dirX = getAttribute<float> (*light, "dirX");
          auto dirY = getAttribute<float> (*light, "dirY");
          auto dirZ = getAttribute<float> (*light, "dirZ");

          operations.insert (operations.end (), {dirX, dirY, dirZ});
        }
      else if ("spotlight" == lightType)
        {
          operations.push_back (SPOTLIGHT);
          auto posX = getAttribute<float> (*light, "posX");
          auto posY = getAttribute<float> (*light, "posY");
          auto posZ = getAttribute<float> (*light, "posZ");

          auto dirX = getAttribute<float> (*light, "dirX");
          auto dirY = getAttribute<float> (*light, "dirY");
          auto dirZ = getAttribute<float> (*light, "dirZ");

          auto cutoff = getAttribute<float> (*light, "cutoff");

          operations.insert (operations.end (), {
              posX, posY, posZ,
              dirX, dirY, dirZ,
              cutoff
          });
        }
      else
        {
          cerr << "[parsing] Unknown light type: " << lightType << endl;
          exit (EXIT_FAILURE);
        }
    }
  while ((light = light->NextSiblingElement ()));
}

void operations_load_xml (const string &filename, vector<float> &operations)
{
  XMLDocument doc;

  if (doc.LoadFile (filename.c_str ()))
    {
      if (doc.ErrorID () == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        cerr << "[parsing] Failed loading file: '" << filename << "'" << endl;
      cerr << doc.ErrorName () << endl;
      exit (EXIT_FAILURE);
    }

  cerr << "[parsing] Loaded file: '" << filename << "'" << endl;
  const XMLElement *const world = doc.FirstChildElement ("world");
  cerr << "[parsing] Loaded element: '" << world->Value () << "'" << endl;

  /*camera*/
  const XMLElement &camera = *world->FirstChildElement ("camera");

  const XMLElement &position = *camera.FirstChildElement ("position");
  operations_push_transform_attributes (position, operations);

  const XMLElement &lookAt = *camera.FirstChildElement ("lookAt");
  operations_push_transform_attributes (lookAt, operations);

  const XMLElement *up = camera.FirstChildElement ("up");
  if (up != nullptr)
    operations_push_transform_attributes (*up, operations);
  else
    operations.insert (operations.end (), {0, 1, 0});

  const XMLElement *const projection = camera.FirstChildElement ("projection");
  if (projection != nullptr)
    {
      auto fov = getAttribute<float> (*projection, "fov");
      auto near = getAttribute<float> (*projection, "near");
      auto far = getAttribute<float> (*projection, "far");
      operations.insert (operations.end (), {fov, near, far});
    }
  else
    operations.insert (operations.end (), {60, 1, 1000});
  /*end of camera*/

  // lights
  const XMLElement *const lights = world->FirstChildElement ("lights");
  if (lights != nullptr)
    operations_push_lights (*lights, operations);

  // find generator if it exists
  const XMLElement *const generator = world->FirstChildElement ("generator");
  if (generator != nullptr)
    {
      auto generator_executable = getAttribute<string> (*generator, "dir");
      crash_if_file_does_not_exist (generator_executable);
      cerr << "[parsing] using generator " << generator_executable << endl;
      globalGeneratorExecutable = generator_executable;
      globalUsingGenerator = true;
    }

  // groups
  const XMLElement *const group = world->FirstChildElement ("group");
  operations_push_groups (*group, operations);
}

//! @} end of group xml

//! @} end of group Operations