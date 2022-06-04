#ifndef _CURVES_H_
#define _CURVES_H_
#include <array>
#include <vector>
#include <glm/glm.hpp>

extern const glm::mat4 Mcr, Mb;
void renderCurve (glm::mat4 M, const std::vector<glm::vec3> &control_points, unsigned int tesselation = 100);
void get_bezier_surface (const std::vector<std::array<glm::vec3, 16>> &control_set,
                                      int int_tesselation,
                                      std::vector<glm::vec3> &vertices,
                                      std::vector<glm::vec3> &normals,
                                      std::vector<glm::vec2> &texture);
void get_curve (float time, const glm::mat4 &M, const std::array<glm::vec3, 4> &control_points, glm::vec3 &pos, glm::vec3 &deriv);
void advance_in_curve (float translation_time, bool align, const glm::mat4 &M, const std::vector<glm::vec3> &global_control_points);
#endif //_CURVES_H_
