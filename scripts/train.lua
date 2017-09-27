require("settings")
require("materials")

function init()

    print("Loading train scene")

    shape = load_obj(get_path() .. "models/train.obj")

    world = World.new()
    world:add_shape(make_instance(shape, make_scale(0.1, 0.1, 0.1)))
    world:preprocess()

    camera_desc = {
        eye = Vec3.new(40, 20, 40),
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

