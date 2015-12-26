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
  - Images (more in the image/* folder)
    Texture mapping / depth of field
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image8.png)
    Demonstration of sampling technique: cornell box rendered with 10,000 samples per pixel
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image62.png)
    Rendering of special surface interaction and total internal reflection
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image66.png)
 - Acknowledgement
    Thanks to Wang Cunguang, who provided the 3D model of our school gate.
