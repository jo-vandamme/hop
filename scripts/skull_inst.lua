require("settings")
require("materials")

function init()

    print("Loading Skull instancing scene")

    shape = load_obj(get_path() .. "models/skull.obj")

    world = World.new()

    d = 30
    step = 4

    for x = -d,d,step do
        for y = -d,d,step do
            for z = -d,d,step do
                xfm = make_translation(x + math.random(-1, 1), y + math.random(-1, 1), z + math.random(-1, 1)) *
                      make_rotation(
                          Vec3.new(math.random(), math.random(), math.random()),
                          math.random(-90, 90))
                inst = make_instance(shape, xfm)
                world:add_shape(inst)
            end
        end
    end

    world:preprocess()

    bbox = world:get_bbox()
    centroid = bbox:get_centroid()

    target = centroid
    d = (bbox:max() - bbox:min()):length()
    eye = target + Vec3.new(d * 0.0, d * 0.0, d * 0.5)

    world:preprocess()

    camera_desc = {
        eye = eye,
        target = target,
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
        focal_distance = d * 0.25
    }
    camera = Camera.make_perspective(camera_desc)

    renderer = Renderer.new(world, camera, options)
    renderer:render_interactive()

end
