require("settings")
require("materials")

function init()

    print("Loading Suzanne scene")

    options.preview_spp = 3

    shape = load_obj(get_path() .. "models/suzanne.obj")

    world = World.new()
    world:add_shape(shape)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 1, 6),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 60,
        lens_radius = 0.0,
        focal_distance = 10
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)

    renderer:render_interactive()

end
