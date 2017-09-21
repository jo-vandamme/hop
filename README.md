## Hop

Simple Monte-Carlo path tracer in C++.
This is a work in progress, only done on my spare time.

[![Build status](https://travis-ci.org/jo-va/hop.svg?branch=master)](https://travis-ci.org/jo-va/hop)

## Features
- Multithreaded rendering and BVH building
- OBJ model loading
- Instancing
- Depth of field
- Data driven scene and render configuration via Lua
- Interactive tiled rendering
- Improved interactivity with adaptative resolution when the render starts

Since I am developping on Linux, the code is targeted to Linux platforms for now, but supporting Windows/Mac OS should not be too difficult.

## Compiling and running
Requires cmake and OpenMP for multithreading, Lua is built along with the project.

Download & Compile hop:
```
$ git clone https://github.com/jo-va/hop.git
$ cd hop
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Run hop:
```
$ ./hop scene.lua
```

## Example scene file, in Lua

```lua
renderer = nil

function init()

    options = {
        frame_width = 800,
        frame_height = 800,
        tile_width = 64,
        tile_height = 64,
        spp = 1,
        preview_spp = 1,
        preview = true
    }

    shape = load_obj("tree.obj")

    world = World.new()
    world:add_shape(shape)
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

Most models shown here were downloaded on Free3D.

Image of a tree (3.5M triangles), rendered with ambient occlusion.
![Tree AO](doc/images/tree_ao.png?raw=true "Tree AO")

The same tree, instanced 50 times (170M triangles) and rendered with depth of field and ambient occlusion.
![Forest AO](doc/images/forest_ao.png?raw=true "Forest AO")

![Train AO](doc/images/train_ao.png?raw=true "Train AO")

![Splash AO](doc/images/splash_ao.png?raw=true "Splash AO")

The black teeth are due to the surface normals being inverted.
![Skull AO](doc/images/skull_ao.png?raw=true "Skull AO")

