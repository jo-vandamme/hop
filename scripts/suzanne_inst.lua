require("settings")
require("materials")

function init()

    print("Loading Suzanne instancing scene")

    shape = load_obj(get_path() .. "models/suzanne.obj")

    world = World.new()

    for i = -10,10,2.0 do
        for j = -10,10,1 do
            xfm = make_translation(i, j, 0) *
                  make_scale(0.3, 0.3, 0.3)

            inst = make_instance(shape, xfm, true)
            world:add_shape(inst)
        end
    end

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 0, 15),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
        focal_distance = 10
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)

    renderer:render_interactive()

end
