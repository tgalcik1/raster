#include "Color.hpp"
#include "Config.hpp"
#include "World.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <map>
#include <array>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int main(int argc, char** argv)
{
    auto startTime = chrono::high_resolution_clock::now();

    // parse command line arguments
    if (argv[1] == nullptr) {
        cerr << "No file provided, provide a file as a command-line argument\n";
        return 1;
    }
    char* filename = argv[1];

    // find and open input file
    string ifname = PROJECT_DATA_DIR;
    if (filename[0] == '/') // absolute path
        ifname = filename;
    else                    // file in data directory
        ifname += filename;

    ifstream infile(ifname);
    if (!infile) {
        cerr << "Error opening " << ifname << '\n';
        return 1;
    }

    // parse the input into everything we know about the world
    World world(infile);

    // pixels initialized to background color
    vector<vector<array<float, 3>>> pixels;
    for (int i = 0; i < world.width; i++)
    {
        vector<array<float, 3>> row(world.width);
        for (int j = 0; j < world.height; j++)
        {
            array<float, 3> color = { world.background[0], world.background[1], world.background[2] };
            row[j] = color;
        }
        pixels.push_back(row);
    }

    // construct RasterFromWorld matrix
    mat4 ViewFromWorld = lookAt(world.eye, world.look, -world.up);

    mat4 NDCFromView = perspective(radians(world.yfov), (float)world.width / world.height, world.near, world.far);

    mat4 RasterFromNDC;
    RasterFromNDC[0] = vec4(world.width / 2, 0, 0, 0);
    RasterFromNDC[1] = vec4(0, world.height / 2, 0, 0);
    RasterFromNDC[2] = vec4(0, 0, 1, 0);
    RasterFromNDC[3] = vec4(world.width / 2, world.height / 2, 1, 1);

    mat4 RasterFromWorld = RasterFromNDC * NDCFromView * ViewFromWorld;

    // create z-buffer and initialize to infinity
    vector<vector<float>> zbuffer;
    for (int i = 0; i < world.width; i++)
    {
        vector<float> row(world.width);
        for (int j = 0; j < world.height; j++)
        {
            row[j] = INFINITY;
        }
        zbuffer.push_back(row);
    }

    int count = 0;
    // for each triangle
    for (auto tri : world.tris) {

        // print current progress
        ++count;
        if (count % (world.tris.size() / 10) == 0)
            std::cout << "Triangle " << count << "/" << world.tris.size() << std::endl;

        // get xmin, xmax, ymin, ymax for the triangle
        float xmin = INFINITY, ymin = INFINITY;
        float xmax = 0, ymax = 0;
        for (int i = 0; i < tri->vert.size(); i++) {
            vec4 p = RasterFromWorld * vec4(tri->vert[i], 1);
            float px = p.x / p.w;
            float py = p.y / p.w;

            if (px < xmin)
                xmin = px;
            if (py < ymin)
                ymin = py;
            if (px > xmax)
                xmax = px;
            if (py > ymax)
                ymax = py;
        }

        // barycentric triangle rasterization
        for (int y = ymin; y < ymax; y++) {
            for (int x = xmin; x < xmax; x++) {
                vec4 v0 = RasterFromWorld * vec4(tri->vert[0], 1);
                v0.x /= v0.w;
                v0.y /= v0.w;
                v0.z /= v0.w;
                vec4 v1 = RasterFromWorld * vec4(tri->vert[1], 1);
                v1.x /= v1.w;
                v1.y /= v1.w;
                v1.z /= v1.w;
                vec4 v2 = RasterFromWorld * vec4(tri->vert[2], 1);
                v2.x /= v2.w;
                v2.y /= v2.w;
                v2.z /= v2.w;

                // compute a,b,g for x,y using Green's Theorem
                float area_tri = (v0.x * v1.y) - (v0.y * v1.x) +
                                 (v1.x * v2.y) - (v1.y * v2.x) +
                                 (v2.x * v0.y) - (v2.y * v0.x);

                float area_alpha = (x * v1.y) - (y * v1.x) +
                                   (v1.x * v2.y) - (v1.y * v2.x) +
                                   (v2.x * y) - (v2.y * x);

                float area_beta = (x * v2.y) - (y * v2.x) +
                                  (v2.x * v0.y) - (v2.y * v0.x) +
                                  (v0.x * y) - (v0.y * x);

                float area_gamma = (x * v0.y) - (y * v0.x) +
                                   (v0.x * v1.y) - (v0.y * v1.x) +
                                   (v1.x * y) - (v1.y * x);

                float alpha = area_alpha / area_tri;
                float beta = area_beta / area_tri;
                float gamma = area_gamma / area_tri;

                // pixel is in the triangle
                if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                    
                    // barycentric interpolation of the z coordinates of each vertex
                    float z = alpha * v0.z + beta * v1.z + gamma * v2.z;

                    // update z-buffer
                    if (x >= 0 && y >= 0 && x < world.width && y < world.height && z < zbuffer[x][y]) {
                        zbuffer[x][y] = z;

                        // starting color
                        vec3 color = vec3(0,0,0);
                        
                        // view ray
                        vec3 V = normalize(world.eye);

                        // position and normal at intersection
                        vec3 P = inverse(RasterFromWorld) * vec4(x,y,z,1);
                        vec3 N = normalize((alpha * tri->norm[0]) + (beta * tri->norm[1]) + (gamma * tri->norm[2]));   // barycentric interpolation of world-space normals

                        for (auto &li : world.lights) {
                            color = color + li.col * tri->surface->ambient;

                            vec3 L = li.pos - P;   // light vector
                            float LLen = length(L);
                            vec3 Ln = L / LLen;

                            float N_dot_L = dot(N, Ln);

                            // check for negative dot product first to avoid shadow cast
                            if (N_dot_L > 0) {
                                color = color + li.col * tri->surface->diffuse * N_dot_L;
                                if (tri->surface->e > 0.f) {
                                    // normalized L and H
                                    vec3 H = normalize(V + Ln);
                                    float N_dot_H = dot(N, H);
                                    if (N_dot_H > 0) {
                                        color = color + li.col * tri->surface->specular * pow(N_dot_H, tri->surface->e) * N_dot_L;
                                    }

                                }
                            }
                        }
                        array<float, 3> final_color = { color[0], color[1], color[2] };
                        pixels[x][y] = final_color;
                    }
                }
            }
        }
    }

    // turn 2d vector of pixels (x,y) into 1d vector of pixels
    vector<uColor> image;
    for (int col = 0; col < pixels[0].size(); col++) {
        for (int row = pixels.size() - 1; row >= 0; row--) {

            array<float, 3> color = { pixels[row][col] };

            // clamp
            if (pixels[row][col][0] < 0) color[0] = 0;
            if (pixels[row][col][1] < 0) color[1] = 0;
            if (pixels[row][col][2] < 0) color[2] = 0;
            if (pixels[row][col][0] > 1) color[0] = 1;
            if (pixels[row][col][1] > 1) color[1] = 1;
            if (pixels[row][col][2] > 1) color[2] = 1;

            // gamma correction
            color[0] = pow(color[0], 1 / 2.2);
            color[1] = pow(color[1], 1 / 2.2);
            color[2] = pow(color[2], 1 / 2.2);

            uColor color_corrected = { int(color[0] * 255), int(color[1] * 255), int(color[2] * 255) };
            image.push_back(color_corrected);
        }
    }
    std::cout << "Triangle " << count << "/" << world.tris.size() << std::endl;

    // write ppm file of pixels
    string ofname = PROJECT_BUILD_DIR "raster.ppm";
    ofstream output(ofname, ofstream::out | ofstream::binary);
    output << "P6\n" << world.width << ' ' << world.height << '\n' << 255 << '\n';
    output.write((char*)(&image[0]), world.width * world.height * sizeof(uColor));

    // deallocate memory
    std::cout << "Cleaning up" << std::endl;
    for (auto tri : world.tris) {
        delete tri;
    }

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<float> elapsed = endTime - startTime;
    cout << elapsed.count() << " seconds\n";
    return 0;
}
