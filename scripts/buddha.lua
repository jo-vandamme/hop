require("settings")
require("materials")

function init()

    print("Loading Buddha scene")

    shape = load_obj(get_path() .. "models/buddha.obj")

    world = World.new()
    world:add_shape(shape)

    bbox = shape:get_bbox()
    centroid = bbox:get_centroid()

    target = centroid
    d = (bbox:max() - bbox:min()):length()
    eye = target - Vec3.new(0, 0, d * 1.0)

    world:preprocess()

    camera_desc = {
        eye = eye,
        target = target,
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
        focal_distance = (eye - target):length()
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
