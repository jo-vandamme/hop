require("settings")
require("materials")

function init()

    print("Loading train scene")

    shape = load_obj(get_path() .. "models/train.obj")

    world = World.new()
    world:add_shape(make_instance(shape, make_scale(0.1, 0.1, 0.1)))
    world:preprocess()

    camera_desc = {
        eye = Vec3.new(34, 26, 40),
        target = Vec3.new(-0.6, -2.6, 1.8),
        up = Vec3.new(-0.32, 0.88, -0.35),
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

