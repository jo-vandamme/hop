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
        eye = Vec3.new(-420, -145, 190),
        target = Vec3.new(-2, -16, -37),
        up = Vec3.new(-0.44, 0.84, -0.33),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 60,
        lens_radius = 3.0,
        focal_distance = 190
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
