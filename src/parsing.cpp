#include <cassert>

#include <filesystem>
#include <iostream>

#include <string>
#include <map>
#include <vector>

#include "tinyxml2.h"
#include "parsing.h"

/*! @addtogroup Operations
 * @{
 * # Data structure for Operations
 *
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
 *      ⟨position⟩   ::= ⟨POSITION⟩ ⟨vec3f⟩
 *      ⟨lookAt⟩     ::= ⟨LOOK_AT⟩ ⟨vec3f⟩
 *      ⟨up⟩         ::= ⟨UP⟩ ⟨vec3f⟩
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
 *
 * ⟨color⟩   ::=  (⟨DIFFUSE⟩ | ⟨AMBIENT⟩ | ⟨SPECULAR⟩ | ⟨EMISSIVE⟩) ⟨nvec3f⟩
 *              | ⟨SHININESS⟩ ⟨shininess_float⟩
 *      ⟨shininess_float⟩ ::= ⟨float⟩ ∈ [0, 128]
 *
 * @endcode
 *
 * Note: we'll use an array of floats, therefore we will need to cast to char when reading
 * the model filename characters and to int when reading the number of characters.
 */

using std::vector, std::map;
using std::string, std::filesystem::exists, std::cerr, std::endl;
using tinyxml2::XMLElement, tinyxml2::XMLDocument;
using tinyxml2::XMLError, tinyxml2::XML_SUCCESS;

void crashIfFailedAttributeQuery (const XMLError error, const XMLElement &element, const string &attributeName)
{
  if (!error)
    return;
  cerr << "[parsing] failed parsing attribute " << attributeName << " of element " << element.Value ()
       << "\nError: " << XMLDocument::ErrorIDToName (error) << endl;
  exit (EXIT_FAILURE);
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

void crashIfFileDoesNotExist (const string &filename, const string &additional_info = "")
{
  if (exists (filename))
    return;
  cerr << "[parsing] Failed loading " << filename << " " << additional_info << endl;
  exit (EXIT_FAILURE);
}

int elementToEnum (string &&s)
{
  map<string, int> m{
      {"translate", TRANSLATE},
      {"rotate", ROTATE},
      {"scale", SCALE},
      {"model", MODEL},
      {"texture", TEXTURE},
      {"diffuse", DIFFUSE},
      {"ambient", AMBIENT},
      {"specular", SPECULAR},
      {"emissive", EMISSIVE},
      {"shininess", SHININESS},
      //{"point", POINT}, //conflicts with point in extended translate
      {"directional", DIRECTIONAL},
      {"spotlight", SPOTLIGHT},
      {"position", POSITION},
      {"lookAt", LOOK_AT},
      {"up", UP},
      {"projection", PROJECTION},
      {"group", GROUP},
      {"generator", GENERATOR},
      {"light", LIGHT}
  };
  if (m.count (s))
    return m[s];
  else
    return OTHER;
}

void pushFloatAttributes (const XMLElement &element, vector<float> &operations)
{
  auto attribute = element.FirstAttribute ();
  if (attribute != nullptr)
    do
      {
        float attributeValue;
        if (attribute->QueryFloatValue (&attributeValue) == XML_SUCCESS)
          operations.push_back (attribute->FloatValue ());
      }
    while ((attribute = attribute->Next ()) != nullptr);
}

void operations_push_any (const XMLElement *element, vector<float> &operations)
{
  static string generatorExecutable;
  static bool isUsingGenerator = false;
  do
    {
      auto elementEnum = elementToEnum (element->Value ());
      switch (elementEnum)
        {
          case GROUP:
            {
              switch (element->ClosingType ())
                {
                  case XMLElement::OPEN:
                    operations.push_back (BEGIN_GROUP);
                  break;
                  case XMLElement::CLOSED:
                    break;
                  case XMLElement::CLOSING:
                    operations.push_back (END_GROUP); //never happens...i think
                  break;
                }
              break;
            }
          case GENERATOR:
            {
              if (element->Attribute ("dir"))
                {
                  generatorExecutable = getAttribute<string> (*element, "dir");
                  crashIfFileDoesNotExist (generatorExecutable);
                  cerr << "[parsing] using generator " << generatorExecutable << endl;
                  isUsingGenerator = true;
                }
              else if (isUsingGenerator)
                {
                  auto generator_argv = getAttribute<string> (*element, "argv");
                  std::stringstream command;
                  command << std::filesystem::current_path ()
                          << "/" << generatorExecutable << " " << generator_argv;
                  if (system (command.str ().data ()))
                    {
                      cerr << "[parsing] generator failed at line " << element->GetLineNum () << endl;
                      exit (EXIT_FAILURE);
                    }
                }
              break;
            }
          case MODEL:
            {
              switch (element->ClosingType ())
                {
                  case XMLElement::OPEN:
                    operations.push_back (BEGIN_MODEL);
                  case XMLElement::CLOSED:
                    operations_push_string_attribute (*element, operations, "file");
                  break;
                  case XMLElement::CLOSING: // never happens...i think
                    operations.push_back (END_MODEL);
                  break;
                }
              break;
            }
          case ROTATE:
            {
              if (element->Attribute ("time"))
                operations.push_back (EXTENDED_ROTATE);
              else
                operations.push_back (ROTATE);
              pushFloatAttributes (*element, operations);
              break;
            }
          case TRANSLATE:
            {
              switch (element->ClosingType ())
                {
                  // ⟨extended_translation⟩ ::= ⟨EXTENDED_TRANSLATE⟩⟨time⟩⟨align⟩⟨number_of_points⟩⟨vec3f⟩⁺
                  case XMLElement::OPEN:
                    {
                      operations.push_back (EXTENDED_TRANSLATE);

                      auto time = getAttribute<float> (*element, "time");
                      auto align = getAttribute<bool> (*element, "align");

                      operations.insert (operations.end (), {time, (float) align});
                      operations.push_back (0); // create space to insert the number of points
                      auto index_of_number_of_points = operations.size () - 1;

                      auto number_of_points = 0;
                      for (auto child = element->FirstChildElement ("point"); child;
                           child = child->NextSiblingElement ("point"))
                        {
                          ++number_of_points;
                          pushFloatAttributes (*child, operations);
                        }
                      operations[index_of_number_of_points] = (float) number_of_points;
                    }
                  break;
                  // ⟨simple_translation⟩ ::= ⟨TRANSLATE⟩⟨vec3f⟩
                  case XMLElement::CLOSED:
                    {
                      operations.push_back (TRANSLATE);
                      pushFloatAttributes (*element, operations);
                    }
                  break;
                  case XMLElement::CLOSING:
                    break;
                }
              break;
            }
          case TEXTURE:     // ⟨texture⟩ ::= ⟨TEXTURE⟩ ⟨number of characters⟩ ⟨char⟩⁺
            {
              operations.push_back (elementEnum);
              operations_push_string_attribute (*element, operations, "file");
              break;
            }
          // (⟨DIFFUSE⟩ | ⟨AMBIENT⟩ | ⟨SPECULAR⟩ | ⟨EMISSIVE⟩) ⟨nvec3f⟩
          case DIFFUSE:
          case AMBIENT:
          case SPECULAR:
          case EMISSIVE:
            {
              operations.push_back (elementEnum);
              const float RGB_MAX = 255.0f;
              auto R = getAttribute<float> (*element, "R");
              auto G = getAttribute<float> (*element, "G");
              auto B = getAttribute<float> (*element, "B");
              assert (R >= 0 && G >= 0 && B >= 0);
              assert (R <= RGB_MAX && G <= RGB_MAX && B <= RGB_MAX);
              operations.insert (operations.end (), {R / RGB_MAX, G / RGB_MAX, B / RGB_MAX});
              break;
            }
          case LIGHT: // ⟨point⟩ | ⟨directional⟩ | ⟨spotlight⟩
            {
              elementEnum = elementToEnum (getAttribute<string> (*element, "type"));
              if (elementEnum == OTHER)
                elementEnum = POINT;
            }
          case SHININESS:           // SHININESS⟩ ⟨float ∈ [0, 128]⟩
          case SCALE:               // ⟨scaling⟩ ::= ⟨SCALE⟩⟨vec3f⟩
          case POINT:               // ⟨point⟩        ::= ⟨POINT⟩⟨vec3f⟩
          case DIRECTIONAL:         // ⟨directional⟩  ::= ⟨DIRECTIONAL⟩⟨vec3f⟩
          case SPOTLIGHT:           // ⟨spotlight⟩    ::= ⟨SPOTLIGHT⟩⟨vec3f⟩⟨vec3f⟩⟨cutoff⟩
          case POSITION:            // ⟨position⟩   ::= ⟨POSITION⟩ ⟨vec3f⟩
          case LOOK_AT:             // ⟨lookAt⟩     ::= ⟨LOOK_AT⟩ ⟨vec3f⟩
          case UP:                  // ⟨up⟩         ::= ⟨UP⟩ ⟨vec3f⟩
          case PROJECTION:          // ⟨projection⟩ ::= ⟨PROJECTION⟩ ⟨vec3f⟩
            {
              operations.push_back (elementEnum);
              pushFloatAttributes (*element, operations);
            }
          break;
        }

      auto child = element->FirstChildElement ();
      if (child != nullptr)
        operations_push_any (child, operations);

      switch (elementEnum)
        {
          case MODEL:
            operations.push_back (END_MODEL);
          break;
          case GROUP:
            operations.push_back (END_GROUP);
          break;
        }
    }
  while ((element = element->NextSiblingElement ()) != nullptr);
}

void operations_load_xml (const string &filename, vector<float> &operations)
{
  XMLDocument doc;

  if (doc.LoadFile (filename.c_str ()))
    {
      cerr << "[parsing] Failed loading file: '" << filename << "'"
           << "\n\tERROR: " << doc.ErrorName () << endl;
      exit (EXIT_FAILURE);
    }

  cerr << "[parsing] Loaded file: '" << filename << "'" << endl;
  const XMLElement *const world = doc.FirstChildElement ("world");
  operations_push_any (world, operations);
}

//! @} end of group Operations