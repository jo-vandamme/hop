
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

    print("Hop example.lua script file init() function.")

    shape = load_obj("/home/jo/dev/tracing/scenes/suzanne2.obj")

    world = World.new()

    for i = -10,10,2.0 do
        for j = -10,10,1 do
            xfm = make_translation(i, j, 0) *
                  make_scale(0.3, 0.3, 0.3)

            inst = make_instance(shape, xfm)
            world:add_shape(inst)
        end
    end

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 0, 10),
        target = Vec3.new(0, 0, 0),
        up = Vec3.new(0, 1, 0),
        frame_width = options.frame_width,
        frame_height = options.frame_height,
        fov = 130,
        lens_radius = 0.0,
        focal_distance = 10
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
