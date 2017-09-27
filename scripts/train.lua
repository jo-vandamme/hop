
options = {
    frame_width = 800,
    frame_height = 600,
    spp = 10,
    adaptive_spp = 100,
    adaptive_threshold = 1.0,
    adaptive_exponent = 2,
    preview_spp = 2,
}

renderer = nil

function init()

    print("Train scene")

    shape = load_obj("/home/jo/dev/tracing/scenes/train.obj")

    world = World.new()
    world:add_shape(make_instance(shape, make_scale(0.1, 0.1, 0.1)))
    world:preprocess()

    camera_desc = {
        eye = Vec3.new(50, 30, 50),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
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

function mouse_scroll_handler(x, y)
end
