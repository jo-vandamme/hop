
options = {
    frame_width = 800,
    frame_height = 800,
    tile_width = 32,
    tile_height = 32,
    spp = 100,
    preview_spp = 10,
    preview = true
}

renderer = nil

function init()

    print("Tree scene")

    shape = load_obj("/home/jo/dev/tracing/scenes/tree_v9/Tree_V9_OBJ/Tree_V9_Final.obj")

    world = World.new()
    world:add_shape(shape)

    world:preprocess()

    camera_desc = {
        eye = Vec3.new(0, 10, 30),
        target = Vec3.new(0, 10, 0),
        up = Vec3.new(0, 1, 0),
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
    if key == 32 and action == 1 then
        renderer:reset()
    end
end

function mouse_button_handler(button, action, mods)
end

function cursor_pos_handler(x, y)
end
