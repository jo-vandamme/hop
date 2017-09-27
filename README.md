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
- Adaptive sampling and firefly reduction
- Interactive tiled rendering
- Improved interactivity with adaptative resolution when the render starts
- Trackball camera

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
$ ./hop -s scene.lua
```

## Example scene file, in Lua

```lua
renderer = nil

function init()

    -- The render options
    options = {
        frame_width = 800,
        frame_height = 800,
        tile_width = 64,
        tile_height = 64,
        spp = 1,
        adaptive_spp = 0,
        firefly_spp = 0,
        adaptive_threshold = 0.1,
        adaptive_exponent = 1,
        firefly_threshold = 0.1,
        tonemap = "filmic",
        preview_spp = 1,
        preview = true
    }

    shape = load_obj("tree.obj")

    world = World.new()
    world:add_shape(shape)

    -- Build the acceleration structures, this will take some time
    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 10, 30),
        target = Vec3.new(0, 10, 0),
        up = Vec3.new(0, 1, 0),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.5,
        focal_distance = 40
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end

function key_handler(key, action)
    -- space key pressed
    if key == 32 and action == 1 then
        renderer:reset()
    end
end

function mouse_button_handler(button, action, mods)
    -- do nothing
end

function cursor_pos_handler(x, y)
    -- do nothing
end
```

## Sample Images

Image of a splash showing depth of field, refraction and reflections.
![Splash](doc/images/splash_tonemap_filmic.png?raw=true "Splash")
This model was downloaded on Free3D.

