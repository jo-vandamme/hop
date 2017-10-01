require("settings")
require("materials")

function init()

    print("Loading test scene")

    options.adaptive_spp = 0
    options.preview_spp = 1
    options.tonemap = "linear"
    options.ray_epsilon = 0.0001

    plane = load_obj(get_path() .. "models/plane.obj")
    monkey = load_obj(get_path() .. "models/suzanne.obj")
    torus = load_obj(get_path() .. "models/torus.obj")
    cone = load_obj(get_path() .. "models/cone.obj")
    text = load_obj(get_path() .. "models/text.obj")

    world = World.new()
    world:add_shape(plane)
    world:add_shape(make_instance(monkey, make_translation(1.5, 1.1, 0)))
    world:add_shape(make_instance(cone, make_translation(-1.5, 1.5, -0.5)))
    world:add_shape(make_instance(torus, make_translation(0, 0.5, 1)))
    world:add_shape(make_instance(text, make_translation(-2.5, 0.3, 3)))

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(3.4, 8, 13.3),
        target = Vec3.new(-0.5, -0.17, 0.27),
        up = Vec3.new(-0.18, 0.86, -0.5),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 60,
        lens_radius = 0.3,
        focal_distance = 14
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)

    renderer:render_interactive()

end
