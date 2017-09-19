#pragma once

#include "render/window.h"

#include <string>
#include <GL/glew.h>

namespace hop {

class GLWindow : public Window
{
public:
    GLWindow(unsigned int width, unsigned int height, const char* title);

    void* map_framebuffer();
    void unmap_framebuffer();

    void init() override;
    void release() override;
    void swap_buffers() override;

private:
    void init_gl();
    void create_textured_quad();
    void check_gl_error();
    std::string get_gl_log(GLuint object);
    GLuint create_shader(GLenum type, const char* shader_source);

private:
    GLuint m_texture_id;
    GLuint m_pbo_id;
    GLuint m_quad_vao_id;
    GLuint m_quad_vbo_id;
    GLuint m_quad_program_id;
};

} // namespace hop
