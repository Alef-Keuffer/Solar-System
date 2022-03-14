#ifndef PROJ_PARSING_H
#define PROJ_PARSING_H

#include <vector>

void operations_load_xml(const char *filename, std::vector<float> *operations);

const int TRANSLATE = 1;
const int ROTATE = 2;
const int SCALE = 3;
const int LOAD_MODEL = 4;
const int BEGIN_GROUP = 5;
const int END_GROUP = 6;
#endif //PROJ_PARSING_H
