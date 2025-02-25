#include "image.h"
#include <cstring>
#include <csetjmp>
extern "C"
{
#include "libs/jpeg/jpeglib.h"
}

#define INVERT_ENDIAN_16(x) ((x >> 8) | (x << 8))

namespace image
{
    struct jpeg_error_struct_l
    {
        struct jpeg_error_mgr pub;
        jmp_buf setjmp_buffer;
    };

    static void libjpeg_error_func_l(j_common_ptr cinfo)
    {
        longjmp(((jpeg_error_struct_l *)cinfo->err)->setjmp_buffer, 1);
    }

    template <typename T>
    void Image<T>::load_jpeg(std::string file)
    {
        FILE *fp = fopen(file.c_str(), "rb");
        if (!fp)
            abort();

        // Huge thanks to https://gist.github.com/PhirePhly/3080633
        unsigned char *jpeg_decomp = NULL;
        jpeg_error_struct_l jerr;
        jpeg_decompress_struct cinfo;

        // Init
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = libjpeg_error_func_l;

        if (setjmp(jerr.setjmp_buffer))
        {
            // Free memory
            delete[] jpeg_decomp;
            return;
        }

        jpeg_create_decompress(&cinfo);

        // Parse and start decompressing
        jpeg_stdio_src(&cinfo, fp);
        jpeg_read_header(&cinfo, FALSE);
        jpeg_start_decompress(&cinfo);

        // Init output buffer
        jpeg_decomp = new unsigned char[cinfo.image_width * cinfo.image_height * cinfo.num_components];

        // Decompress
        while (cinfo.output_scanline < cinfo.output_height)
        {
            unsigned char *buffer_array[1];
            buffer_array[0] = jpeg_decomp + (cinfo.output_scanline) * cinfo.image_width * cinfo.num_components;
            jpeg_read_scanlines(&cinfo, buffer_array, 1);
        }

        // Cleanup
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        // Init CImg image
        init(cinfo.image_width, cinfo.image_height, cinfo.num_components);

        // Copy over
        if (d_depth == 8)
        {
            for (int i = 0; i < (int)cinfo.image_width * (int)cinfo.image_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    channel(c)[i] = jpeg_decomp[i * cinfo.num_components + c];
        }
        else if (d_depth == 16)
        {
            for (int i = 0; i < (int)cinfo.image_width * (int)cinfo.image_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    channel(c)[i] = jpeg_decomp[i * cinfo.num_components + c] << 8; // Scale up to 16 if required
        }

        // Free memory
        delete[] jpeg_decomp;
        fclose(fp);
    }

    template <typename T>
    void Image<T>::load_jpeg(uint8_t *buffer, int size)
    {
        // Huge thanks to https://gist.github.com/PhirePhly/3080633
        unsigned char *jpeg_decomp = NULL;
        jpeg_error_struct_l jerr;
        jpeg_decompress_struct cinfo;

        // Init
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = libjpeg_error_func_l;

        if (setjmp(jerr.setjmp_buffer))
        {
            // Free memory
            delete[] jpeg_decomp;
            return;
        }

        jpeg_create_decompress(&cinfo);

        // Parse and start decompressing
        jpeg_mem__src(&cinfo, buffer, size);
        jpeg_read_header(&cinfo, FALSE);
        jpeg_start_decompress(&cinfo);

        // Init output buffer
        jpeg_decomp = new unsigned char[cinfo.image_width * cinfo.image_height * cinfo.num_components];

        // Decompress
        while (cinfo.output_scanline < cinfo.output_height)
        {
            unsigned char *buffer_array[1];
            buffer_array[0] = jpeg_decomp + (cinfo.output_scanline) * cinfo.image_width * cinfo.num_components;
            jpeg_read_scanlines(&cinfo, buffer_array, 1);
        }

        // Cleanup
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        // Init CImg image
        init(cinfo.image_width, cinfo.image_height, cinfo.num_components);

        // Copy over
        if (d_depth == 8)
        {
            for (int i = 0; i < (int)cinfo.image_width * (int)cinfo.image_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    channel(c)[i] = jpeg_decomp[i * cinfo.num_components + c];
        }
        else if (d_depth == 16)
        {
            for (int i = 0; i < (int)cinfo.image_width * (int)cinfo.image_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    channel(c)[i] = jpeg_decomp[i * cinfo.num_components + c] << 8; // Scale up to 16 if required
        }

        // Free memory
        delete[] jpeg_decomp;
    }

    template <typename T>
    void Image<T>::save_jpeg(std::string file)
    {
        FILE *fp = fopen(file.c_str(), "wb");
        if (!fp)
            abort();

        unsigned char *jpeg_decomp = NULL;
        jpeg_error_struct_l jerr;
        jpeg_compress_struct cinfo;

        // Init
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = libjpeg_error_func_l;

        if (setjmp(jerr.setjmp_buffer))
        {
            // Free memory
            delete[] jpeg_decomp;
            return;
        }

        jpeg_create_compress(&cinfo);

        // Parse and start decompressing
        jpeg_stdio_dest(&cinfo, fp);
        cinfo.image_width = d_width;
        cinfo.image_height = d_height;
        cinfo.input_components = d_channels;
        cinfo.in_color_space = d_channels == 3 ? JCS_RGB : JCS_GRAYSCALE;
        jpeg_set_defaults(&cinfo);
        jpeg_start_compress(&cinfo, true);

        // Init output buffer
        jpeg_decomp = new unsigned char[cinfo.image_width * cinfo.image_height * cinfo.num_components];

        // Copy over
        if (d_depth == 8)
        {
            for (int i = 0; i < (int)d_width * (int)d_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    jpeg_decomp[i * cinfo.num_components + c] = channel(c)[i];
        }
        else if (d_depth == 16)
        {
            for (int i = 0; i < (int)d_width * (int)d_height; i++)
                for (int c = 0; c < cinfo.num_components; c++)
                    jpeg_decomp[i * cinfo.num_components + c] = channel(c)[i] >> 8; // Scale down to 8 if required
        }

        // Compress
        while (cinfo.next_scanline < cinfo.image_height)
        {
            unsigned char *buffer_array[1];
            buffer_array[0] = jpeg_decomp + (cinfo.next_scanline) * cinfo.image_width * cinfo.num_components;
            jpeg_write_scanlines(&cinfo, buffer_array, 1);
        }

        jpeg_finish_compress(&cinfo);

        // Free memory
        fclose(fp);
        delete[] jpeg_decomp;
    }

    // Generate Images for uint16_t and uint8_t
    template class Image<uint8_t>;
    template class Image<uint16_t>;
}