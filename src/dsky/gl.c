#include <stdbool.h>
#include <stdlib.h>
#include <dsky/gl.h>
#include <dsky/vec.h>
#include <dsky/log.h>
#include <dsky/hope.h>
#include <assert.h>

#define TAG "GL"

static bool is_reasonable_power_of_two(uint32_t n) {
    switch(n) {
    case 1<<0: return true;
    case 1<<1: return true;
    case 1<<2: return true;
    case 1<<3: return true;
    case 1<<4: return true;
    case 1<<5: return true;
    case 1<<6: return true;
    case 1<<7: return true;
    case 1<<8: return true;
    case 1<<9: return true;
    case 1<<10: return true;
    case 1<<11: return true;
    case 1<<12: logw("The texture takes a bit too much space\n"); return true;
    case 1<<13: logw("The texture's gonna eat all the VRAM\n"); return true;
    case 1<<14: logw("The texture's gonna eat all the VRAM\n"); return true;
    }
    return false;
}

GLuint GLTexture_from_power_of_two_rgba32(const Rgba32 *data, Extent2u extent) {
    assert(data);
    assert(is_reasonable_power_of_two(extent.w));
    assert(is_reasonable_power_of_two(extent.h));
    assert(extent.w == extent.h);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    hope(tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, extent.w, extent.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

char* GLShader_get_info_log(GLuint shader) {
    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *info_log = malloc(info_log_length+1);
    GLsizei dummy_out_length;
    glGetShaderInfoLog(shader, info_log_length, &dummy_out_length, info_log);
    info_log[info_log_length] = '\0';
    return info_log;
}

char* GLProgram_get_info_log(GLuint program) {
    GLint info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar *info_log = malloc(info_log_length+1);
    GLsizei dummy_out_length;
    glGetProgramInfoLog(program, info_log_length, &dummy_out_length, info_log);
    info_log[info_log_length] = '\0';
    return info_log;
}

void GLShader_log_info_log(GLuint shader, const char *header) {
    char* info_log = GLShader_get_info_log(shader);
    logi("%s\n%s\n", header, info_log);
    free(info_log);
}
void GLProgram_log_info_log(GLuint program, const char *header) {
    char* info_log = GLProgram_get_info_log(program);
    logi("%s\n%s\n", header, info_log);
    free(info_log);
}

GLuint GLProgram_quick_load(const char *vert, const char *frag) {
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    hope(vshader);
    hope(fshader);
    glShaderSource(vshader, 1, &vert, NULL);
    glShaderSource(fshader, 1, &frag, NULL);

    glCompileShader(vshader);
    glCompileShader(fshader);
    GLint vstatus, fstatus;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &vstatus);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &fstatus);
    if(!vstatus) {
        GLShader_log_info_log(vshader, "---- Vertex shader info log:");
        logi("---- For information, the source is:\n\n%s\n", vert);
        glDeleteShader(vshader);
    }
    if(!fstatus) {
        GLShader_log_info_log(fshader, "---- Fragment shader info log:");
        logi("---- For information, the source is:\n\n%s\n", frag);
        glDeleteShader(fshader);
    }
    if(!vstatus || !fstatus) // Wanna see both logs before returning
        return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glDetachShader(program, vshader);
    glDetachShader(program, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    GLint pstatus;
    glGetProgramiv(program, GL_LINK_STATUS, &pstatus);
    if(!pstatus) {
        GLProgram_log_info_log(program, "---- Program link status:");
        glDeleteProgram(program);
        return 0;
    }

    return program;
}
