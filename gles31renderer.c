#include <SDL2/SDL.h>
// Define this before any GL includes to get function prototypes
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL2/SDL_opengles2.h>
#include <assert.h>
#include <string.h>
#include "renderer.h"
#include "atlas.inl"

#define BUFFER_SIZE 16384

// Vertex structure for interleaved data
typedef struct {
    float pos[2];
    float tex[2];
    unsigned char color[4];
} Vertex;

static Vertex vertices[BUFFER_SIZE * 4];
static GLushort indices[BUFFER_SIZE * 6];

static int width = 800;
static int height = 480;
static int buf_idx;

static SDL_Window *window;
static GLuint shader_program;
static GLuint vao, vbo, ebo;
static GLuint texture;
static GLint u_projection;

// Vertex shader
static const char *vertex_shader_src = 
"#version 310 es\n"
"precision mediump float;\n"
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec2 a_tex;\n"
"layout(location = 2) in vec4 a_color;\n"
"uniform mat4 u_projection;\n"
"out vec2 v_tex;\n"
"out vec4 v_color;\n"
"void main() {\n"
"  v_tex = a_tex;\n"
"  v_color = a_color;\n"
"  gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);\n"
"}\n";

// Fragment shader
static const char *fragment_shader_src =
"#version 310 es\n"
"precision mediump float;\n"
"in vec2 v_tex;\n"
"in vec4 v_color;\n"
"uniform sampler2D u_texture;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"  float alpha = texture(u_texture, v_tex).a;\n"
"  fragColor = v_color * alpha;\n"
"}\n";

static GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    assert(success);
    
    return shader;
}

void r_init(void) {
    // Init SDL window
    window = SDL_CreateWindow(
        NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_OPENGL);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_CreateContext(window);

    // Create shaders
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);
    
    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    assert(success);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    u_projection = glGetUniformLocation(shader_program, "u_projection");

    // Create VAO and buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex));
    glEnableVertexAttribArray(1);
    
    // Color attribute
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    // Create texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
                 GL_ALPHA, GL_UNSIGNED_BYTE, atlas_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set GL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);
    
    assert(glGetError() == GL_NO_ERROR);
        // Unbind VAO first (this also unbinds the VBO from VAO state)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void flush(void) {
    if (buf_idx == 0) return;
    glUseProgram(shader_program);
    
    // Set projection matrix (orthographic)
    float proj[16] = {
        2.0f/width, 0, 0, 0,
        0, -2.0f/height, 0, 0,
        0, 0, -1, 0,
        -1, 1, 0, 1
    };
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, proj);

    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf_idx * 4 * sizeof(Vertex), vertices);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, buf_idx * 6 * sizeof(GLushort), indices);
    
    glDrawElements(GL_TRIANGLES, buf_idx * 6, GL_UNSIGNED_SHORT, 0);

    buf_idx = 0;
}

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
    if (buf_idx == BUFFER_SIZE) flush();

    int vi = buf_idx * 4;
    int ii = buf_idx * 6;
    
    // Texture coordinates
    float tx0 = src.x / (float)ATLAS_WIDTH;
    float ty0 = src.y / (float)ATLAS_HEIGHT;
    float tx1 = (src.x + src.w) / (float)ATLAS_WIDTH;
    float ty1 = (src.y + src.h) / (float)ATLAS_HEIGHT;

    // Vertices (counter-clockwise)
    vertices[vi + 0] = (Vertex){{dst.x, dst.y}, {tx0, ty0}, {color.r, color.g, color.b, color.a}};
    vertices[vi + 1] = (Vertex){{dst.x + dst.w, dst.y}, {tx1, ty0}, {color.r, color.g, color.b, color.a}};
    vertices[vi + 2] = (Vertex){{dst.x + dst.w, dst.y + dst.h}, {tx1, ty1}, {color.r, color.g, color.b, color.a}};
    vertices[vi + 3] = (Vertex){{dst.x, dst.y + dst.h}, {tx0, ty1}, {color.r, color.g, color.b, color.a}};

    // Indices (two triangles)
    GLushort base = buf_idx * 4;
    indices[ii + 0] = base + 0;
    indices[ii + 1] = base + 1;
    indices[ii + 2] = base + 2;
    indices[ii + 3] = base + 0;
    indices[ii + 4] = base + 2;
    indices[ii + 5] = base + 3;

    buf_idx++;
}

void r_draw_rect(mu_Rect rect, mu_Color color) {
    push_quad(rect, atlas[ATLAS_WHITE], color);
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = {pos.x, pos.y, 0, 0};
    for (const char *p = text; *p; p++) {
        if ((*p & 0xc0) == 0x80) continue;
        int chr = mu_min((unsigned char)*p, 127);
        mu_Rect src = atlas[ATLAS_FONT + chr];
        dst.w = src.w;
        dst.h = src.h;
        push_quad(dst, src, color);
        dst.x += dst.w;
    }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
    mu_Rect src = atlas[id];
    int x = rect.x + (rect.w - src.w) / 2;
    int y = rect.y + (rect.h - src.h) / 2;
    push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

int r_get_text_width(const char *text, int len) {
    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) continue;
        int chr = mu_min((unsigned char)*p, 127);
        res += atlas[ATLAS_FONT + chr].w;
    }
    return res;
}

int r_get_text_height(void) {
    return 18;
}

void r_set_clip_rect(mu_Rect rect) {
    flush();
    glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}

void r_clear(mu_Color clr) {
    flush();
    glViewport(0, 0, width, height);
    glClearColor(clr.r / 255.0f, clr.g / 255.0f, clr.b / 255.0f, clr.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void r_present(void) {
    flush();
    SDL_GL_SwapWindow(window);
}