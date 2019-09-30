#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <fstream>

#include "bmp.h"
#include "read_file.h"
#include "fsize.h"


static inline void set_frame(int width, int height, uint8_t *stream_data, 
                uint8_t *picture_data, int width_picture, int height_picture) {
    int offset_x = 10;
    int offset_y = 10;

    int count = 0;
    for (int i = offset_x; i < height_picture + offset_x; i++) {
        for (int j = offset_y; j < width_picture + offset_y; j++) {
            stream_data[(width * i) + j] = picture_data[count];
            count++;
        }
    }
    
    int stream_frame = width * height;
    int index_x1 = stream_frame + offset_x;
    int index_y1 = stream_frame + offset_y;

    int width_result = (width_picture/2) + offset_y;

    for (int i = index_x1; i < index_x1 + (height_picture/2); i++) {
        for (int j = index_y1; j < width_result; j++) {  
            stream_data[(width * i) + j] = picture_data[count];
            count++;
        }
    }
    
    int small_stream_frame = stream_frame + (stream_frame/4);
    int index_x2 = small_stream_frame + offset_x;
    int index_y2 = small_stream_frame + offset_y;

    for (int i = index_x2; i < index_x2 + (height_picture/2); i++) {
        for (int j = index_y2; j < width_result; j++) {        
            stream_data[(width * i) + j] = picture_data[count];
            count++;
        }
    }
}


int add_pic_to_stream(char *in_filename, int width, int height,
                                    char *out_filename, char *in_picname) {

    /* Prepare video file */
    size_t video_file_size = fsize(in_filename);
    if (!video_file_size) {
        return 1;
    }
    size_t frame_size = width * height * 1.5;
    size_t n_frames = video_file_size / frame_size;

    uint8_t *video_yuv_data = read_file(in_filename);
    if (!video_yuv_data) {
        return 2;
    }

    /* Prepare picture file */
    uint8_t *picture_bmp_data = read_file(in_picname);
    if (!picture_bmp_data) {
        return 3;
    }

    size_t width_picture = width_bmp(picture_bmp_data);
    if (!width_picture) {
        return 4;
    }

    size_t height_picture = height_bmp(picture_bmp_data);
    if (!height_picture) {
        return 5;
    }

    uint8_t *picture_yuv_data = bmp_to_yuv(picture_bmp_data);
    if (!picture_yuv_data) {
        return 6;
    }

    uint8_t *ptr = video_yuv_data;
    for (size_t i = 0; i < n_frames; i++) {
        set_frame(width, height, ptr, picture_yuv_data, width_picture, height_picture);
        ptr += frame_size;
    }

    std::ofstream outfile;
    try {
        outfile.open(out_filename, std::ios::out|std::ios::binary);
    }
    catch (const std::ios_base::failure &) {
        return 7;
    }

    outfile.write(reinterpret_cast<char*>(video_yuv_data),
                                            video_file_size * sizeof(char));
    outfile.close();
    delete[] video_yuv_data;
    delete[] picture_yuv_data;
    delete[] picture_bmp_data;
    return 0;
}


int main (int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Failed: Failure input parameters.\n");
    }
    /* 
     * argv[1] is name of input video file
     * argv[2] is width of input video file
     * argv[3] is height of input video file
     * argv[4] is output video file name
     * argv[5] is name of input picture file
     */
    if (add_pic_to_stream(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4],
                                                                 argv[5]) != 0) {
        fprintf(stdout, "Failed: System error.\n");
        return 1;
    }

    fprintf(stdout, "OK!\n");
    return 0;
}
