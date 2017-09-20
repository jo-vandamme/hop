
options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 64,
    tile_height = 64,
    spp = 1,
    preview_spp = 1,
    preview = false
}

renderer = nil

function init()

    print("Skull scene")

    shape = load_obj("/home/jo/dev/tracing/hop/models/skull.obj")

    world = World.new()

    d = 50
    step = 5

    for x = -d,d,step do
        for y = -d,d,step do
            for z = -d,d,step do
                xfm = make_translation(x, y, z)
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
    eye = target + Vec3.new(d * 0.5, d * 0.5, d * 0.7)

    world:preprocess()

    camera_desc = {
        transform = make_lookat(eye, target, Vec3.new(0, 1, 0)),
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

function key_handler(key, action)
    print("Key handler: " .. key .. " " .. action)
    if string.byte(" ", 1) == key and action == 1 then
        renderer:reset()
    end
end

function mouse_button_handler(button, action, mods)
end

function cursor_pos_handler(x, y)
end
