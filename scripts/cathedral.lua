

options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 16,
    tile_height = 16,
    spp = 20,
    preview_spp = 1,
    preview = true
}

renderer = nil

function init()

    -- model from patrix on sketchfab
    -- https://sketchfab.com/models/faed84a829114e378be255414a7826ca#
    shape = load_obj("/home/jo/dev/tracing/scenes/cathedral/combined02.obj")

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
        up = Vec3.new(0, 1, 0),
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
    -- space pressed
    if key == 32 and action == 1 then
        renderer:reset()
    end
end

function mouse_button_handler(button, action, mods)
end

function cursor_pos_handler(x, y)
end
