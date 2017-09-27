require("settings")
require("materials")

function init()

    print("Loading Cathedral scene")

    -- model from patrix on sketchfab
    -- https://sketchfab.com/models/faed84a829114e378be255414a7826ca#
    shape = load_obj(get_path() .. "models/cathedral.obj")

    world = World.new()
    world:add_shape(shape)

    bbox = shape:get_bbox()
    centroid = bbox:get_centroid()

    target = centroid
    d = (bbox:max() - bbox:min()):length()
    eye = target + Vec3.new(-d * 0.9, d * 0.4, -d * 0.8)

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
