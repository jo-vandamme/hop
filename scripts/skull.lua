require("settings")
require("materials")

function init()

    print("Loading Skull scene")

    shape = load_obj(get_path() .. "models/skull.obj")

    world = World.new()
    world:add_shape(shape)

    xfm1 = make_translation(2, 0, -3)
    inst1 = make_instance(shape, xfm1)
    world:add_shape(inst1)

    xfm2 = make_translation(-1, 0, 3)
    inst2 = make_instance(shape, xfm2)
    world:add_shape(inst2)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(3, 1, 5),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.1,
        focal_distance = 5
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
