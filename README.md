# Exterior Mapping
This application provides a implementation to an algorithm proposed in Master's thesis with the name of Exterior Mapping. The Exterior Mapping algorithm aims to render large three-dimensional computer scenes using the Image Based Rendering and theory of light fields. For further information please refer to the thesis as it describes the implementation of the proposed algorithm in depth.

The application provides a renderer written using C++ and Vulkan API and was loosely implemented based on the [Vulkan tutorial](https://vulkan-tutorial.com/) from Alexander Overvoorde, [simple renderer](https://github.com/Taardal/vulkan-tutorial) by Torbjørn Årdal and some functionality was implemented with the usage of Sascha Willems' repository of [Vulkan C++ examples and demos](https://github.com/SaschaWillems/Vulkan).

# Contents of this repository
- `eval/` - contains Python script, that generates data presented in the chapter concerning evaluation
- `external/` - contains non-downloaded external libraries
- `include/` - contains `*.h` files
- `res/` - contains resources needed to run the application, namely models, shaders and configuration files
- `src` - contains `*.cpp` files

## Building the application
The application uses CMake build system, so please make sure, that CMake is installed on the machine. As mentioned previously, the application also uses the Vulkan API, so latest version of Vulkan needs to be available to the application as well. Once both of these have been successfully installed. The following commands can be ran in the root of this project:

```
mkdiir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
(Build the code in Release, as in Debug, the model parsing done by assimp takes a significant amount of time.)

As the application uses the CMake `ExternalProject` module, all of the libraries needed by the application are downloaded and built into the `build/downloaded/` folder. The shader files are also compiled during the build, these are saved into `build/compiled_shaders/` folder.

## Running the application
Before running the application, please make sure that the necessary models are downloaded from [this link](https://drive.google.com/file/d/1AQd8o1OTtUKqS0fa-NVg6DNq4-RjNuLP/view?usp=drive_link) and placed into the `res/models/` folder. **Alternatively, for the thesis submission, they are already placed inside the correct folder.** Once that has been done, the application can be run as:

```
./ExteriorMapping { --recover | --config CONFIG_FILE } [ -w W H ] [ -n W H ] [ -v W H ]
```
where:
- `--recover` - runs the application with the configuration saved at the end of the last execution of the application
- `--config CONFIG_FILE` - runs the application using the configuration specified in the `CONFIG_FILE`, where `CONFIG_FILE` is a path to a config file **WITHIN** the `res/configs/` folder (e.g. `./ExteriorMapping --config by_step/config.json`)
- `-w`, `-n`, `-v` - represent the window, novel view and view grid image resolution, respectively
- `W`, `H` - represent the width and height in pixels

As mentioned, there are also scripts, that run evaluation presented in the last chapter of the thesis. These are located in the `eval/` folder. The needed packages can be downloaded by running the following commands in the `eval/` folder:

```
python -m pip install -r requirements.txt
```

Once that is done, the evaluation can be ran as:

```
python graphs.py
```

The graphs will be generated into the `eval/graphs/` folder.

## Documentation
The documentation to this application has been created using Doxygen. The documentaion can be generated using the `BUILD_DOC` flag passed to CMake when building.

## The models
The downloadable models have not been created by the author of this repository and thesis. The famous Crytek Sponza model by Marco Dabrovic has been downloaded from this [page](https://casual-effects.com/data/). The model of the Porsche Cayman GTS has been downloaded from a cgtrader website and was created by the user under this [link](https://www.cgtrader.com/designers/raddysa71arxi).