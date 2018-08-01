# vandalizer-vr


## Building for Web

Clone and build emscripten from this particular branch: https://github.com/joshshadik/emscripten/tree/stable


Then create a build directory and run cmake

```
cmake .. -DWASM_BUILD=1 -DCMAKE_TOOLCHAIN_FILE=PATH_TO_INSTALLED_EMSDK_GOES_HERE/cmake/Modules/Platform/Emscripten.cmake -DEMSCRIPTEN_ROOT_PATH=PATH_TO_INSTALLED_EMSDK_GOES_HERE
```

where *PATH_TO_INSTALLED_EMSDK_GOES_HERE* is dependant on where you cloned emscripten

and then run make
```
make
```


if you want to use the same background model you can purchase it here : https://sketchfab.com/models/acc162ade37c4b07b2d74a59fc40d5af
download as gltf and place into  /resources/gallery/
then add ```-DPROPRIETARY_ASSETS=1``` when running cmake.
