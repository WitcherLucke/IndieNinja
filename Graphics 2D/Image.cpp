#include "Image.h"
#include <fstream>
#include <vector>
//#include <gli.hpp>


Image::Image()
    : width(0), height(0), textureID(0), loadState(LoadState::None) {}

Image::~Image() {
    Free();
}

void Image::Free() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    loadState = LoadState::None;
}

#pragma pack(push, 1)
struct DDS_HEADER {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    uint32_t ddspf_size;
    uint32_t ddspf_flags;
    uint32_t ddspf_fourCC;
    uint32_t ddspf_RGBBitCount;
    uint32_t ddspf_RBitMask;
    uint32_t ddspf_GBitMask;
    uint32_t ddspf_BBitMask;
    uint32_t ddspf_ABitMask;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

struct DDS_HEADER_DXT10 {
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t miscFlags2;
};
#pragma pack(pop)

static GLuint LoadBC7DDS(const char* filepath) {
    // Open the file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return 0; // Failed to open file
    }

    // Verify DDS magic number
    char magic[4];
    file.read(magic, 4);
    if (strncmp(magic, "DDS ", 4) != 0) {
        file.close();
        return 0; // Not a DDS file
    }

    // Read DDS header
    DDS_HEADER header;
    file.read(reinterpret_cast<char*>(&header), sizeof(DDS_HEADER));

    // Check for DX10 extension
    bool hasDX10Header = (header.ddspf_fourCC == '01XD'); // 'DX10' in little-endian

    DDS_HEADER_DXT10 dx10Header;
    if (hasDX10Header) {
        file.read(reinterpret_cast<char*>(&dx10Header), sizeof(DDS_HEADER_DXT10));
    }

    // Verify BC7 format
    const bool isBC7 = hasDX10Header ?
        (dx10Header.dxgiFormat == 98 || // DXGI_FORMAT_BC7_UNORM
            dx10Header.dxgiFormat == 99)  // DXGI_FORMAT_BC7_UNORM_SRGB
        : (header.ddspf_fourCC == '7CBD'); // 'BC7' in little-endian (uncommon)

    if (!isBC7) {
        file.close();
        return 0; // Not a BC7 texture
    }

    // Determine OpenGL format
    GLenum internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
    if (hasDX10Header && dx10Header.dxgiFormat == 99) {
        internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
    }

    // Calculate buffer size for main texture
    const uint32_t blockSize = 16; // BC7 always uses 16-byte blocks
    const uint32_t width = header.dwWidth;
    const uint32_t height = header.dwHeight;
    const uint32_t mipMapCount = (header.dwMipMapCount) ? header.dwMipMapCount : 1;

    // Create OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load each mip level
    uint32_t currentWidth = width;
    uint32_t currentHeight = height;
    for (uint32_t level = 0; level < mipMapCount && (currentWidth || currentHeight); ++level) {
        uint32_t size = ((currentWidth + 3) / 4) * ((currentHeight + 3) / 4) * blockSize;
        std::vector<unsigned char> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);

        glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat,
            currentWidth, currentHeight, 0, size, buffer.data());

        // Half dimensions for next mip level
        currentWidth = std::max<uint32_t>(1, currentWidth / 2);
        currentHeight = std::max<uint32_t>(1, currentHeight / 2);
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        mipMapCount > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    file.close();
    return textureID;
}

bool Image::LoadFromDiskBC7(const std::string& path) {
    Free();

    textureID = LoadBC7DDS(path.c_str());

    loadState = LoadState::BC7;
    return true;
}

bool Image::LoadFromDiskStandard(const std::string& path) {
    Free(); // Clear previous texture

    int nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        loadState = LoadState::None;
        return false;
    }

    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 4) format = GL_RGBA;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    loadState = LoadState::Standard;
    return true;
}

/*bool Image::IsBC7Format(const std::string& path) {
    // Simple stub based on file extension
    auto extPos = path.find_last_of('.');
    if (extPos == std::string::npos) return false;

    std::string ext = path.substr(extPos + 1);
    for (char& c : ext) c = tolower(c);

    return ext == "ktx" || ext == "dds";
}*/

/*bool Image::LoadFromDisk(const std::string& path) {
    if (IsBC7Format(path)) {
        return LoadFromDiskBC7(path);
    }
    else {
        return LoadFromDiskStandard(path);
    }
}*/

bool Image::IsLoaded() const {
    return loadState != LoadState::None;
}

/*bool Image::IsBC7() const {
    return loadState == LoadState::BC7;
}*/

bool Image::IsStandard() const {
    return loadState == LoadState::Standard;
}

int Image::GetWidth() const { return width; }
int Image::GetHeight() const { return height; }
GLuint Image::GetTextureID() const { return textureID; }