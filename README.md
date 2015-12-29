# MCRayTrace

A Monte Carlo ray tracer.
  - implemented features:
    - ability to handle phong reflection and refraction
    - triangles and spheres
    - two samplers: stratified and latincube
    - kd-tree accelerator
    - bump mapping, texture mapping
    - importance sampling, direct lighting
    - a tiny physics engine framework with partial implementation
    - scattering media
    - ...
  - dependency and environment configuration:
    - this project builds a ray tracer from scratch however 
      - 1) open cv is used for image io; 
      - 2) Eigen library is used to perform vector calculations; 
      - 3) some utilities in boost are also used.
    - IDE: visual studio 2013, platform: windows.
  - Sample pictures (more in the images/* folder)
    - Texture mapping / depth of field
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image8.png)
    - Demonstration of sampling technique: cornell box rendered with 10,000 samples per pixel
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image62.png)
    - Rendering of anisotropic surface interaction and total internal reflection
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image25.png)
    ![alt tag](https://github.com/Crispher/MCRayTrace/blob/master/images/Image66.png)
    - Sample video of physics engine: [link](https://github.com/Crispher/MCRayTrace/blob/master/animation/animation.avi)
 - Acknowledgement: \\
    Thanks to Wang Cunguang, who provided the 3D model of our school gate.
