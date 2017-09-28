require("settings")
require("materials")

function init()

    print("Loading Cornell scene")

    options.frame_width = 800
    options.frame_height = 600
    options.adaptive_spp = 0

    left_wall = load_obj(get_path() .. "models/cbox/leftwall.obj")
    right_wall = load_obj(get_path() .. "models/cbox/rightwall.obj")
    walls = load_obj(get_path() .. "models/cbox/walls.obj")
    sphere1 = load_obj(get_path() .. "models/cbox/sphere1.obj")
    sphere2 = load_obj(get_path() .. "models/cbox/sphere2.obj")
    light = load_obj(get_path() .. "models/cbox/light.obj")

    world = World.new()
    world:add_shape(left_wall)
    world:add_shape(right_wall)
    world:add_shape(walls)
    world:add_shape(sphere1)
    world:add_shape(sphere2)
    world:add_shape(light)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 0.919769, 5.41159),
        target = Vec3.new(0, 0.893051, 4.41198),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 27.7856 * 2,
        lens_radius = 0.0
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)

    renderer:render_interactive()

end
