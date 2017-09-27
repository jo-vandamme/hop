require("settings")
require("materials")

function init()

    print("Loading Splash scene")

    shape = load_obj(get_path() .. "models/splash.obj")

    world = World.new()
    world:add_shape(shape)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 30, 30),
        target = Vec3.new(0, 5, 0),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.3,
        focal_distance = 30
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
