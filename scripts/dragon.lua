require("settings")
require("materials")

function init()

    print("Loading Dragon scene")

    -- overwrite some of the settings
    options.frame_width = 700
    options.frame_height = 700

    shape = load_obj(get_path() .. "models/dragon.obj")

    world = World.new()
    world:add_shape(make_instance(shape, make_scale(-1, 1, 1)))
    world:preprocess()

    camera_desc = {
        eye = Vec3.new(-295, -50, 195),
        target = Vec3.new(-2, -16, -37),
        up = Vec3.new(-0.4, 0.8, -0.4),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
