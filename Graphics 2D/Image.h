#pragma once

#include <stdio.h>
#include <string>
#include <GL/glew.h>    
#include <glfw3.h>
#include "stb_image.h"
#include <iostream>
#include <glm.hpp>
#include <GLI/gli.hpp>

//  Declares the gli namespace, which is where the GLI library's types and functions reside.

/* In GLI library context
gli::texture is the main texture object you load data into.

gli::gl::format helps convert gli texture formats to OpenGL enums.

gli::gl::PROFILE_GL33 is a tag used to specify OpenGL 3.3 profile in format translation functions */


/*namespace gli {
    class texture;
    namespace gl {
        struct format;
        struct PROFILE_GL33 {};
    }
}*/

class Image {
public:
    Image();
    ~Image();

    // Loaders
    bool LoadFromDiskStandard(const std::string& path); // uses stb_image
    bool LoadFromDiskBC7(const std::string& path);      // uses gli

    // Convenience
    bool IsLoaded() const;
    //bool IsBC7() const;
    bool IsStandard() const;

    // Getters
    int GetWidth() const;
    int GetHeight() const;
    GLuint GetTextureID() const;

/* Your image can be in one of a few mutually exclusive loading states:

Not loaded (None)

Loaded as a standard image (PNG, JPG, etc.)

Loaded as a BC7 compressed texture

Using an enum models this explicitly instead of using ambiguous flags or integers.  */

private:
    enum class LoadState {
        None,
        Standard,
        BC7,
    };

    int width;
    int height;
    GLuint textureID;
    LoadState loadState;

    void Free();
};