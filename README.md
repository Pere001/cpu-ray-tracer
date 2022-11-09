# Real Time CPU Raytracer
A simple CPU raytracer made in a week.


Features:
* Controls: WASD to move, Space to ascend, Shift to descend, left click to orient camera, Escape to exit.

* Uses WINAPI for input, threads, and window stuff.

* Worker threads render groups of rows of pixels into a common frame buffer which is then sent to the GPU, and each frame rendered via OpenGL.

* It only supports spheres and axis-aligned planes.

* Each pixel that hits a shape shoots one light ray and one reflection ray. The reflection doesn't bounce and isn't shaded. Pixels are shaded using the Blinn-Phong reflectivity model. The shading could easily and cheaply be improved to make more different materials and allow lights of different colors.

* Soft shadows are computed via a hack I came up with, which only works for spherical lights and spherical blockers. You project each blocker sphere into the plane perpendicular to the light ray, which contains the sphere's center. Imagine a "cone of vision", which is a truncated cone extending from the pixel position to the light position, defining the space where objects would block the pixel's light. So, compute the radius of the section of the "cone of vision" that's on the plane we projected the sphere to. Now that we have the cone's projected circle and the sphere's projected circle, to find out how much light is blocked we just need to find how much of the area of the cone's circle intersects the sphere's circle. To do that, we use a cheap approximation using the distance that the sphere's circle penetrates the cone's circle. Basically we take this distance and we square it.

  If multiple spheres block some light, the final value of light for the pixel will be a mix of 3 different ways of accumulating that blocked light: the maximum light blocked by a single sphere, the sum (clamped to 0), and the sum divided by the number of spheres that blocked any light. I just experimented a bit and came up with these values and their weights to reduce some artifacts that ocurred when only using one value.

* The coordinate system is left-handed: +X is right, +Z is forward, and +Y is up. This means that the cross product follows the left hand rule. Angles are counterclockwise and follow the right hand rule (thumb points to the direction of the axis of rotation).

* Six spheres and one plane render at 60 FPS, at a 640x480 resolution, in my not so good 2019 laptop with 8 logical cores.



To compile with MSVC:
- Open a command prompt.
- Locate the ``vcvarsall.bat`` file in your VS installation directory and call it with argument ``x64``.
- Call ``build.bat``
