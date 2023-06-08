// implementation code for World object
// World holds everything we know about the world.
// Code here initializes it based on file input

#include "World.hpp"
#include "Surface.hpp"

#include <math.h>
#include <istream>
#include <iostream>

#ifndef F_PI
#define F_PI 3.1415926536f
#endif

using namespace glm;

// read input file
World::World(std::istream &ifile)
{
    // world state defaults
    eye = vec3(0,-8,0);
    width = height = 512;
    maxdepth = 15;
    cutoff = 0.002;
    background = vec3(0,0,0);
    look = vec3(0, 0, 0);
    up = vec3(0, 1, 0);
    xfov = 45, yfov = 45;
    near = 10.f;
    far = 1000.f;

    // temporary variables while parsing
    std::string surfname;

    // current surface while parsing
    // initialize to valid but impossible to reference surface
    Surface *currentSurface = &surfaceMap[""];

    // parse file one token at a time
    std::cout << "Parsing file... ";
    std::string token;
    ifile >> token;
    while (!ifile.eof()) {
        if (token == "background")
            ifile >> background[0] >> background[1] >> background[2];
        else if (token == "eyep")
            ifile >> eye[0] >> eye[1] >> eye[2];
        else if (token == "lookp")
            ifile >> look[0] >> look[1] >> look[2];
        else if (token == "up")
            ifile >> up[0] >> up[1] >> up[2];
        else if (token == "fov")
            ifile >> xfov >> yfov;
        else if (token == "screen")
            ifile >> width >> height;

        else if (token == "surface") {
            ifile >> surfname;
            currentSurface = &surfaceMap[surfname];
        }

        else if (token == "ambient")
            ifile >> currentSurface->ambient[0] >> currentSurface->ambient[1] >> currentSurface->ambient[2];
        else if (token == "diffuse")
            ifile >> currentSurface->diffuse[0] >> currentSurface->diffuse[1] >> currentSurface->diffuse[2];
        else if (token == "specular")
            ifile >> currentSurface->specular[0] >> currentSurface->specular[1] >> currentSurface->specular[2];
        else if (token == "specpow")
            ifile >> currentSurface->e;

        else if (token == "light") {
            float intensity;
            vec3 position;
            ifile >> intensity >> token >> position[0] >> position[1] >> position[2];
            lights.push_back(Light(vec3(intensity, intensity, intensity), position));
        }

        else if (token == "triangle") {
            ifile >> surfname;
            Triangle* tri = new Triangle(&surfaceMap[surfname]);
            vec3 vert;
            vec3 norm;
            try { // read until stof throws on non-numeric token
                while (!ifile.eof()) {
                    ifile >> token; vert[0] = std::stof(token);
                    ifile >> token; vert[1] = std::stof(token);
                    ifile >> token; vert[2] = std::stof(token);

                    ifile >> token; norm[0] = std::stof(token);
                    ifile >> token; norm[1] = std::stof(token);
                    ifile >> token; norm[2] = std::stof(token);

                    if (ifile.eof()) break;
                    tri->addVertex(vert);
                    tri->addNormal(norm);
                }

            }
            catch (...) {}
            tris.push_back(tri);

            //    // have an unused non-numeric token
            //    // skip token read at end of loop to process next iteration
            continue;
        }

        else if (token == "polygon") {
            ifile >> surfname;
            try { // read until stof throws on non-numeric token
                while (!ifile.eof()) {
                    std::vector<float> quad;
                    Triangle* tri1 = new Triangle(&surfaceMap[surfname]);
                    Triangle* tri2 = new Triangle(&surfaceMap[surfname]);
                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));

                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));

                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));

                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));
                    ifile >> token; quad.push_back(std::stof(token));

                    if (ifile.eof()) break;

                    vec3 v1 = vec3(quad[0], quad[1], quad[2]);
                    vec3 v2 = vec3(quad[3], quad[4], quad[5]);
                    vec3 v3 = vec3(quad[6], quad[7], quad[8]);
                    vec3 v4 = vec3(quad[9], quad[10], quad[11]);

                    // create 1st triangle
                    tri1->addVertex(vec3(quad[0], quad[1], quad[2]));
                    tri1->addVertex(vec3(quad[3], quad[4], quad[5]));
                    tri1->addVertex(vec3(quad[6], quad[7], quad[8]));
                    // normal is cross product of diagonal vertices
                    tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tris.push_back(tri1);

                    // create 2nd triangle
                    tri2->addVertex(vec3(quad[6], quad[7], quad[8]));
                    tri2->addVertex(vec3(quad[9], quad[10], quad[11]));
                    tri2->addVertex(vec3(quad[0], quad[1], quad[2]));
                    // normal is cross product of diagonal vertices
                    tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                    tris.push_back(tri2);
                }

            }
            catch (...) {}

            // have an unused non-numeric token
            // skip token read at end of loop to process next iteration
            continue;
        }

        else if (token == "sphere") {
            float radius;
            vec3 center;
            ifile >> surfname >> radius >> center[0] >> center[1] >> center[2];

            // assn3 code for triangulating a sphere
            int w = 50, h = 25;
            std::vector<vec2> uv;
            std::vector<vec3> norm, vert;
            // build vertex, normal and texture coordinate arrays
            for (unsigned int y = 0; y <= h; ++y) {
                for (unsigned int x = 0; x <= w; ++x) {
                    // Texture coordinates scaled from x and y. Be sure to cast before division!
                    float u = float(x) / float(w), v = float(y) / float(h);
                    uv.push_back(vec2(u, v));

                    // normal for sphere is normalized position in spherica coordinates
                    float cx = cosf(2.f * F_PI * u), sx = sinf(2.f * F_PI * u);
                    float cy = cosf(F_PI * v), sy = sinf(F_PI * v);
                    vec3 N = vec3(cx * sy, sx * sy, cy);
                    norm.push_back(N);

                    // 3d vertex location scaled by sphere size
                    vert.push_back((radius * N) + center);
                }
            }

            // build index array linking sets of three vertices into triangles
            std::vector<int> indices;
            for (unsigned int y = 0; y < h; ++y) {
                for (unsigned int x = 0; x < w; ++x) {
                    indices.push_back((w + 1) * y + x);
                    indices.push_back((w + 1) * y + x + 1);
                    indices.push_back((w + 1) * (y + 1) + x + 1);

                    indices.push_back((w + 1) * y + x);
                    indices.push_back((w + 1) * (y + 1) + x + 1);
                    indices.push_back((w + 1) * (y + 1) + x);
                }
            }

            // traverse indices in order to create triangles
            for (int i = 0; i < indices.size(); i+=3) {
                Triangle* tri = new Triangle(&surfaceMap[surfname]);
                tri->addVertex(vert[indices[i]]);
                tri->addVertex(vert[indices[i+1]]);
                tri->addVertex(vert[indices[i+2]]);
                tri->addNormal(norm[indices[i]]);
                tri->addNormal(norm[indices[i+1]]);
                tri->addNormal(norm[indices[i+2]]);
                tris.push_back(tri);
            }
        }

        else if (token == "cone") {
            float radius1, radius2;
            vec3 center1, center2;
            ifile >> surfname >> radius1 >> center1[0] >> center1[1] >> center1[2] >> radius2 >> center2[0] >> center2[1] >> center2[2];
            
            int divisions = 50; // the number of evenly spaced points around each radius
            vec3 axis = normalize(center1 - center2); // vector from the first center to second center (cone axis)
            
            // place evenly spaced points along the circumference of each given radius
            std::vector<vec3> verts1, verts2;
            for (int i = 0; i < divisions; i++) {
                float x1, x2, y1, y2;
                // place points along each circumference on the xy plane
                x1 = cosf((2 * F_PI * i) / divisions) * radius1;
                y1 = sinf((2 * F_PI * i) / divisions) * radius1;
                x2 = cosf((2 * F_PI * i) / divisions) * radius2;
                y2 = sinf((2 * F_PI * i) / divisions) * radius2;

                // move points to the center of each radius. still need to figure out how to rotate them normal to cone axis
                verts1.push_back(vec3(x1, y1, 0) + center1);
                verts2.push_back(vec3(x2, y2, 0) + center2);
            }

            std::vector<vec3> quad;
            // connect adjacent points to each other, and points 
            // from the 1st radius to the 2nd radius to form quads
            for (int i = 0; i < divisions; i++) {
                int j = (i + 1 == divisions ? 0 : i + 1); // last vertices connect to the first ones

                quad.push_back(verts1[i]);    // first vertex on 1st radius
                quad.push_back(verts1[j]);    // second vertex on 1st radius
                quad.push_back(verts2[j]);    // second vertex on 2nd radius
                quad.push_back(verts2[i]);    // first vertex on 2nd radius
            }

            // split each quad into two triangles
            for (int i = 0; i < quad.size(); i += 4) {
                Triangle* tri1 = new Triangle(&surfaceMap[surfname]);
                Triangle* tri2 = new Triangle(&surfaceMap[surfname]);

                vec3 v1 = vec3(quad[i]);
                vec3 v2 = vec3(quad[i+1]);
                vec3 v3 = vec3(quad[i+2]);
                vec3 v4 = vec3(quad[i+3]);

                // create 1st triangle
                tri1->addVertex(v1);
                tri1->addVertex(v2);
                tri1->addVertex(v3);
                // normal is cross product of diagonal vertices
                tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tri1->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tris.push_back(tri1);

                // create 2nd triangle
                tri2->addVertex(v3);
                tri2->addVertex(v4);
                tri2->addVertex(v1);
                // normal is cross product of diagonal vertices
                tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tri2->addNormal(normalize(cross((v2 - v4), (v3 - v1))));
                tris.push_back(tri2);
            }
        }

        ifile >> token;
    }
    std::cout << "done" << std::endl;

    // compute view basis
    w = eye - look;
    dist = length(w);
    w = normalize(w);
    u = normalize(cross(up, w));
    v = cross(w, u);

    // solve for screen edges
    right = dist * tanf(xfov * F_PI/360);
    left = -right;
    top = dist * tanf(yfov * F_PI/360);
    bottom = -top;
}
