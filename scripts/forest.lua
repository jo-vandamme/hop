
options = {
    frame_width = 1024,
    frame_height = 800,
    tile_width = 64,
    tile_height = 64,
    spp = 1,
    preview_spp = 1,
    preview = true
}

renderer = nil

function init()

    print("Skull scene")

    shape = load_obj("/home/jo/dev/tracing/scenes/tree_v9/Tree_V9_OBJ/Tree_V9_Final.obj")

    world = World.new()
    --world:add_shape(shape)

    for i = -40,80,20 do
        for j = -80,40,20 do
            xfm = make_translation(i, 0, j)
            inst = make_instance(shape, xfm)
            world:add_shape(inst)
        end
    end

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(13, 5, 25),
        target = Vec3.new(0, 10, 0),
        up = Vec3.new(0, 1, 0),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 90,
        lens_radius = 0.1,
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
