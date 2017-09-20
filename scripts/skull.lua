----------------------------
-- Hop example scene file --
----------------------------

options = {
    frame_width = 1024,
    frame_height = 800,
    tile_width = 64,
    tile_height = 64,
    samples_per_pixel = 1
}

renderer = nil

function init()

    print("Skull scene")

    shape = load_obj("/home/jo/dev/tracing/hop/models/skull.obj")

    world = World.new()
    world:add_shape(shape)

    xfm1 = make_translation(2, 0, -3)
    inst1 = make_instance(shape, xfm1)
    world:add_shape(inst1)

    xfm2 = make_translation(-1, 0, 3)
    inst2 = make_instance(shape, xfm2)
    world:add_shape(inst2)

    world:preprocess()

    camera_desc = {
        transform = make_lookat(Vec3.new(3, 1, 5), Vec3.new(0, 0, 0), Vec3.new(0, 1, 0)),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.1,
        focal_distance = 5
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
