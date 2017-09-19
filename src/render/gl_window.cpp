#include "render/gl_window.h"
#include "except.h"

#include <string>
#include <sstream>
#include <GL/glew.h>

namespace hop {

GLWindow::GLWindow(unsigned int width, unsigned int height, const char* title)
    : Window(width, height, title)
    , m_texture_id(0), m_pbo_id(0)
    , m_quad_vao_id(0), m_quad_vbo_id(0), m_quad_program_id(0)
{
}

void* GLWindow::map_framebuffer()
{
    // Bind PBO to update pixel values
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * m_window_width * m_window_height, 0, GL_STREAM_DRAW);
    return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
}

void GLWindow::unmap_framebuffer()
{
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Transfer the data
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_window_width, m_window_height, GL_RGB, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void GLWindow::swap_buffers()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Draw texture to fullscreen quad
    glUseProgram(m_quad_program_id);
    glBindVertexArray(m_quad_vao_id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    Window::swap_buffers();
}

void GLWindow::init()
{
    Window::init();

    init_gl();
    create_textured_quad();
}

void GLWindow::release()
{
    if (m_pbo_id) glDeleteBuffers(1, &m_pbo_id);
    if (m_texture_id) glDeleteTextures(1, &m_texture_id);
    if (m_quad_vao_id) glDeleteVertexArrays(1, &m_quad_vao_id);
    if (m_quad_vbo_id) glDeleteBuffers(1, &m_quad_vbo_id);
    if (m_quad_program_id) glDeleteProgram(m_quad_program_id);

    m_pbo_id = 0;
    m_texture_id = 0;
    m_quad_vao_id = 0;
    m_quad_vbo_id = 0;
    m_quad_program_id = 0;

    Window::release();
}

void GLWindow::init_gl()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw Error("GLWindow: failed to initialize GLEW");

    // Streaming texture upload using PBOs enabling DMA texture transfers to the GPU
    // http://www.songho.ca/opengl/gl_pbo.html

    // Create the framebuffer texture and reserve space for it
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_window_width, m_window_height, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_gl_error();

    glGenBuffers(1, &m_pbo_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * m_window_width * m_window_height, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    check_gl_error();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glViewport(0, 0, m_window_width, m_window_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    check_gl_error();
}

void GLWindow::create_textured_quad()
{
    const char* quad_vs_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec2 texcoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "    TexCoord = texcoord;\n"
        "}";

    const char* quad_fs_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    FragColor = texture(tex, TexCoord);\n"
        "}";

    const float vertex_data[] = {
        -1.0f,-1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,-1.0f, 0.0f, 0.0f,
         1.0f,-1.0f, 1.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quad_vao_id);
    glGenBuffers(1, &m_quad_vbo_id);

    glBindVertexArray(m_quad_vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_quad_program_id = glCreateProgram();
    GLuint vshader = create_shader(GL_VERTEX_SHADER, quad_vs_shader_source);
    GLuint fshader = create_shader(GL_FRAGMENT_SHADER, quad_fs_shader_source);
    glAttachShader(m_quad_program_id, vshader);
    glAttachShader(m_quad_program_id, fshader);
    glLinkProgram(m_quad_program_id);

    GLint linked_ok = GL_FALSE;
    glGetProgramiv(m_quad_program_id, GL_LINK_STATUS, &linked_ok);
    if (linked_ok == GL_FALSE)
        throw Error("GLWindow: Quad program error: " + get_gl_log(m_quad_program_id));

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glUseProgram(m_quad_program_id);
    glUniform1i(glGetUniformLocation(m_quad_program_id, "tex"), 0);
    glUseProgram(0);

    check_gl_error();
}

void GLWindow::check_gl_error()
{
    GLenum err;
    std::ostringstream oss;
    while ((err = glGetError()) != GL_NO_ERROR)
        oss << "(" << err << "): " << glewGetErrorString(err);
    if (!oss.str().empty())
        throw Error("GLWindow: OpenGL error: " + oss.str());
}

std::string GLWindow::get_gl_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else
        return "get_log: Not a shader or program";
    char log[256];

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, &log[0]);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, &log[0]);

    return std::string(log);
}

GLuint GLWindow::create_shader(GLenum type, const char* shader_source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint compiled_ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled_ok);
    if (compiled_ok == GL_FALSE)
        throw Error("GLWindow: create_shader error: " + get_gl_log(shader));
    return shader;
}

} // namespace hop
