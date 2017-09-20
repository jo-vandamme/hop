
options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 64,
    tile_height = 64,
    samples_per_pixel = 1
}

renderer = nil

function init()

    print("Skull scene")

    shape = load_obj("/home/jo/dev/tracing/scenes/Maybach_DS8_Zeppelin_OBJ.obj")

    world = World.new()
    world:add_shape(make_instance(shape, make_scale(0.2, 0.2, 0.2)))
    world:preprocess()

    camera_desc = {
        transform = make_lookat(Vec3.new(40, 30, 50), Vec3.new(0, 0, 0), Vec3.new(0, 1, 0)),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.0,
        focal_distance = 40
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
