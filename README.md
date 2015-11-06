# MCRayTrace

A Monte Carlo ray tracer.
  - implemented feature:
    - ability to handle phong reflection and refraction
    - triangles and spheres
    - two samplers: stratified and latincube
    - kd-tree accelerator
    - bump mapping
    - texture mapping
  - todo:
    - sampling the lightsource at each reflection
    - handle more general brdf surfaces
    - hemisphere lightsource
    - ...
  - dependency and environment configuration:
    - this project builds a ray tracer from scratch however 
      - 1) open cv are used to show and save images; 
      - 2) Eigen library are used to perform linear algebra calculations; 
      - 3) some utilities in boost are also used.
    - IDE: visual studio 2013, platform: windows.
