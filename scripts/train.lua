

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

    shape = load_obj("/home/jo/dev/tracing/scenes/train.obj")

    inst = make_instance(shape, make_scale(0.1, 0.1, 0.1))

    world = World.new()
    world:add_shape(inst)

    world:preprocess()

    camera_desc = {
        transform = make_lookat(Vec3.new(50, 30, 50), Vec3.new(0, 0, 0), Vec3.new(0, 1, 0)),
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

function key_handler(key)
    print("Key handler: " .. key)
    if string.byte(" ", 1) == key then
        renderer:reset()
    end
end
