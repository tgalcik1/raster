// Everything we know about the world
#pragma once

// other classes we use DIRECTLY in our interface
#include "Surface.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using namespace glm;

struct Light {
    vec3 col;                   // light color
    vec3 pos;                   // light position
    Light() : col(1,1,1), pos(0,0,0) {}
    Light(vec3 _col, vec3 _pos) : col(_col), pos(_pos) {}
};
typedef std::vector<Light> LightList;

struct Triangle {
    //vec3 vert[3] = { vec3(0,0,0), vec3(0,0,0), vec3(0,0,0) };
    std::vector<vec3> vert;
    std::vector<vec3> norm;
    const Surface* surface;
    Triangle(Surface* _surface) : surface(_surface) {}
    void addVertex(vec3 _vert) {
        vert.push_back(_vert);
    }
    void addNormal(vec3 _norm) {
        norm.push_back(_norm);
    }
};
typedef std::vector<Triangle*> TriangleList;

class World {
public: // public data
    // image size
    int width, height;

    // view origin and basis parameters
    vec3 eye, look, up, w, u, v;
    float dist, left, right, bottom, top;
    float xfov, yfov;
    float near, far;

    // ray recursion termination
    int maxdepth;
    float cutoff;

    // map of surface names to colors
    std::map<std::string, Surface> surfaceMap;

    // background color
    vec3 background;

    // list of triangles in the scene
    TriangleList tris;

    // list of lights
    LightList lights;

public:                                 
    // constructor: read world data from a file
    World(std::istream &ifile); 
};
