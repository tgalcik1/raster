# Barycentric Triangle Rasterization

This program  handles triangles, polygons, spheres, and cones. Each primitive is able to
be turned into triangles with per-vertex normals as well so that Blinn-Phong shading
done correctly for the objects in the scene.

I used glm for vector and matrix operations. I created the RasterFromWorld matrix by multiplying 
a lookAt matrix by a perspective matrix to get to NDC space, followed by a hand-made
RasterFromNDC matrix (via the Transform/Viewing lecture slides). I then iterate over
every triangle, computing the bounding box and calculating the barycentric coordinates
to check if the triangle contains the pixel. If so, I update the z-buffer using an
interpolation of the barycentric coordinates on the z value of each vertex of the
triangle. If the computed z value is less than the current z value at that pixel, then
I continue on to calculating the lighting (mostly the same as assn2).

As for handling primitives, triangles were straightforward as the vertices and normals
are read directly from a rayshade file. Polygons are split into two triangles whose normals 
are the normalized cross product of diagonal vertices. Spheres are turned into a mesh of
triangle vertices and normals, and afterwards I just iterate through a for loop for
every computed index linking the triangles together. Since the vertices and normals
were already computed, I just created a new triangle and add the vertices and normals.

My approach for triangulating a cone was to take each radius and place points around
the circumference at that radius. The number of points will impact how many triangles
the cone is comprised of. Once I placed these points in a circle for each radius, I
created quads where the vertices were as follows:
	1. First vertex of first radius
	2. Second vertex of first radius
	3. Second vertex of second radius
	4. First vertex of second radius
This creates quads for the entire cone in counter-clockwise order. Then, I treated
the quads exactly as I did when handling polygons.
