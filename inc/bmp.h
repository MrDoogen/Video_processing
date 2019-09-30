#ifndef BMP_H
#define BMP_H

size_t width_bmp(uint8_t *in_filename);
size_t height_bmp(uint8_t *in_filename);
uint8_t *bmp_to_yuv(uint8_t *bmp_data);

#endif /* BMP_H */
