
options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 64,
    tile_height = 64,
    spp = 1,
    preview_spp = 1,
    preview = true
}

renderer = nil

function init()

    print("Enter OBJ file to load:")
    obj = io.read()
    shape = load_obj(obj)

    world = World.new()
    world:add_shape(shape)

    bbox = shape:get_bbox()
    centroid = bbox:get_centroid()

    target = centroid
    d = (bbox:max() - bbox:min()):length()
    eye = target + Vec3.new(0, 0, d * 1.5)

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
    print("Key: " .. key .. " action: " .. action)

    -- space pressed
    if key == 32 and action == 1 then
        renderer:reset()
    end
end

function mouse_button_handler(button, action, mods)
end

function cursor_pos_handler(x, y)
end
