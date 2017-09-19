## Hop

Simple Monte-Carlo path tracer in C++.
This is a work in progress, only done on my spare time.

[![Build status](https://travis-ci.org/jo-va/hop.svg?branch=master)](https://travis-ci.org/jo-va/hop)

## Features
- OBJ model loading
- Instancing
- Data driven configuration via Lua
- Interactive tiled rendering
- Depth of field

Since I am developping on Linux, the code is targeted to Linux platforms for now, but supporting Windows/Mac OS should not be too difficult.

## Compiling and running
Requires cmake and OpenMP for multithreading, Lua is built along with the project.

Download & Compile:
```
$ git clone https://github.com/jo-va/hop.git
$ cd hop
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Run:
```
$ ./hop scene.lua
```

## Example scene file

```lua
renderer = nil

function init()

    options = {
        frame_width = 800,
        frame_height = 800,
        tile_width = 64,
        tile_height = 64,
        samples_per_pixel = 1
    }

    shape = load_obj("tree.obj")

    world = World.new()
    world:add_shape(shape)

    -- Build the BVH tree
    world:preprocess()

    camera_desc = {
        transform = make_lookat(Vec3.new(0, 10, 30), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0)),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
        focal_distance = 40
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end

function key_handler(key)

    if string.byte(" ", 1) == key then
        renderer:reset()
    end

end
```

## Sample Images

Image of a tree (35M triangles), rendered with ambient occlusion.
![Tree](doc/images/tree.png?raw=true "Tree")

The same tree, instanced 50 times (170M triangles) and rendered with depth of field and ambient occlusion.
![Forest DOF](doc/images/forest_dof.png?raw=true "Forest DOF")

Image of a splash, rendered with ambient occlusion and some depth of field.
![Splash AO](doc/images/splash_ao.png?raw=true "Splash AO")
The model was dowloaded on Free3D and is from DOSH Design.

This image shows the ray tracing of 3 mesh instances for a total of 4.5M triangles. It also shows some depth of field.
![Skull](doc/images/skull.png?raw=true "Skull")
I really don't remember where I downloaded this model...

