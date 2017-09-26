
options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 16,
    tile_height = 16,
    spp = 10,
    adaptive_spp = 0,
    firefly_spp = 0,
    adaptive_threshold = 0.1,
    adaptive_exponent = 1,
    firefly_threshold = 0.1,
    tonemap = "filmic",
    preview_spp = 1,
    preview = true
}

renderer = nil

function init()

    print("Skull scene")

    shape = load_obj("/home/jo/dev/tracing/hop/models/splash.obj")

    world = World.new()
    world:add_shape(shape)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 30, 30),
        target = Vec3.new(0, 5, 0),
        up = Vec3.new(0, 1, 0),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.3,
        focal_distance = 30
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
