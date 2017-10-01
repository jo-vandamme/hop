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
    tonemap = "gamma",
    ray_epsilon = 0.0001
}

renderer = nil
last_cursor_pos_x = nil
last_cursor_pos_y = nil
last_key = nil

function key_handler(key, action)
    if action == 1 then
        last_key = key
    elseif action == 0 then
        last_key = nil
    end
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
    -- key D and left mouse button are pressed
    if button == 0 and action == 1 and last_key == 68 then
        dist = renderer:set_focus_point(last_cursor_pos_x, last_cursor_pos_y)
        print("Distance: " .. dist)
    end
end

function cursor_pos_handler(x, y)
    last_cursor_pos_x = x
    last_cursor_pos_y = y
end

function mouse_scroll_handler(x, y)
end
