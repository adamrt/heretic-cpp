#include <iostream>
#include <stdexcept>

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(std::string filename)
{
    sg_sampler_desc sampler_desc = {};
    sampler_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    sampler_desc.min_filter = SG_FILTER_NEAREST;
    sampler_desc.mag_filter = SG_FILTER_NEAREST;
    sampler = sg_make_sampler(&sampler_desc);

    image = load_png(filename.c_str());
    if (image.id == SG_INVALID_ID) {
        throw std::runtime_error("This is a crash test exception.");
    }
}

Texture::~Texture()
{
    std::cout << "destroying now!" << std::endl;
    sg_destroy_image(image);
    sg_destroy_sampler(sampler);
}

sg_image Texture::load_png(const char* filename)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // Load image as RGBA
    if (!data) {
        return { SG_INVALID_ID };
    }

    sg_image_desc image_desc = {};
    image_desc.width = width;
    image_desc.height = height;
    image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    image_desc.data.subimage[0][0] = { data, static_cast<size_t>(width * height * 4) };

    sg_image _image = sg_make_image(&image_desc);

    stbi_image_free(data); // Free the loaded image data

    return _image;
}
