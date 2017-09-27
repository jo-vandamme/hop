print("Loading settings")

options = {
    frame_width = 1000,
    frame_height = 800,
    spp = 10,
    adaptive_spp = 100,
    firefly_spp = 0,
    adaptive_threshold = 1.0,
    adaptive_exponent = 2,
    firefly_threshold = 1.0,
    preview_spp = 1,
    preview = true,
    tonemap = "gamma"
}

renderer = nil

function key_handler(key, action)
    -- space pressed
    if key == 32 and action == 1 then
        print("render reset")
        renderer:reset()
    end
    -- Z key pressed
    if key == 90 and action == 1 then
        camera = renderer:get_camera()
        eye = camera:get_eye()
        target = camera:get_target()
        up = camera:get_up()
        print("eye: " .. tostring(eye) .. " target: " .. tostring(target) .. " up: " .. tostring(up))
    end
end

function mouse_button_handler(button, action, mods)
end

function cursor_pos_handler(x, y)
end

function mouse_scroll_handler(x, y)
end
