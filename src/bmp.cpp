#include <fstream>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <cstring> /* memset */
#include <emmintrin.h> /* SSE2 */

#include <read_file.h>
#include <fsize.h>

#if defined (__WIN32__) || defined (__WIN64__)
#include <windows.h>
#elif defined (__gnu_linux__) || defined (__FreeBSD__)
#include <unistd.h>
#endif


static int get_ncpus(void) {
    int nprocs;
#if defined (__WIN32__) || defined (__WIN64__)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    nprocs = info.dwNumberOfProcessors;
#elif defined (__gnu_linux__) || defined (__FreeBSD__)
#ifdef _SC_NPROCESSORS_ONLN
    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
#else
#error "Can't define number of CPUs"
#endif
#endif
    if (nprocs < 1) {
        nprocs = 1;
    }
    return nprocs;
}


size_t width_bmp(uint8_t *in_bmp_data) {
    int width = *reinterpret_cast<int*>(&in_bmp_data[18]);
    return width;
}


size_t height_bmp(uint8_t *in_bmp_data) {
    int height = *reinterpret_cast<int*>(&in_bmp_data[22]);
    return height;
}


/*
static inline uint8_t normalize_byte_value(uint8_t in_byte) {
    if (in_byte < 0) {
        return 0;
    }
    else if (in_byte > 255) {
        return 255;
    }
    else {
        return in_byte;
    }
}
*/


static void thread_convert(uint8_t *in_data,
                          uint8_t *out_data,
                         size_t first_index,
                           size_t end_index,
                          size_t frame_size) {

    size_t size = end_index - first_index;

    uint8_t r[size];
    uint8_t g[size];
    uint8_t b[size];

    int ypos;
    int vpos;
    int upos;

    for (size_t i = 0, j = first_index; j < end_index; i++, j++) {
        r[i] = in_data[j * 3 + 2];
        g[i] = in_data[j * 3 + 1];
        b[i] = in_data[j * 3 + 0];
    }

    alignas(16) double index0[] = {0.257, 0.257};
    alignas(16) double index1[] = {0.504, 0.504};
    alignas(16) double index2[] = {0.098, 0.098};
    alignas(16) double index3[] = {0.439, 0.439};
    alignas(16) double index4[] = {0.368, 0.368};
    alignas(16) double index5[] = {0.071, 0.071};
    alignas(16) double index6[] = {-0.148, -0.148};
    alignas(16) double index7[] = {0.291, 0.291};
    alignas(16) double index8[] = {0.439, 0.439};

    alignas(16) double number16[] = {16, 16};
    alignas(16) double number128[] = {128, 128};

    __m128d *index0_simd = reinterpret_cast<__m128d*>(index0);
    __m128d *index1_simd = reinterpret_cast<__m128d*>(index1);
    __m128d *index2_simd = reinterpret_cast<__m128d*>(index2);
    __m128d *index3_simd = reinterpret_cast<__m128d*>(index3);
    __m128d *index4_simd = reinterpret_cast<__m128d*>(index4);
    __m128d *index5_simd = reinterpret_cast<__m128d*>(index5);
    __m128d *index6_simd = reinterpret_cast<__m128d*>(index6);
    __m128d *index7_simd = reinterpret_cast<__m128d*>(index7);
    __m128d *index8_simd = reinterpret_cast<__m128d*>(index8);

    __m128d *number16_simd = reinterpret_cast<__m128d*>(number16);
    __m128d *number128_simd = reinterpret_cast<__m128d*>(number128);

    alignas(16) double buf[2];
    __m128d *buf_simd;

    __m128d temp0;
    __m128d temp1;
    __m128d temp2;
    __m128d add_temp;

    for (size_t i = 0, j = first_index; j < end_index; i += 2, j += 2) {
/*
        y = normalize_byte_value((0.257*r[i]) + (0.504*g[i]) + (0.098*b[i]) + 16);
        v = normalize_byte_value((0.439*r[i]) - (0.368*g[i]) - (0.071*b[i]) + 128);
        u = normalize_byte_value(-(0.148*r[i]) - (0.291*g[i]) + (0.439*b[i]) + 128);
*/
        buf[0] = r[i + 0];
        buf[1] = r[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);
        temp0 = _mm_mul_pd(*index0_simd, *buf_simd);

        buf[0] = g[i + 0];
        buf[1] = g[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);        
        temp1 = _mm_mul_pd(*index1_simd, *buf_simd);

        buf[0] = b[i + 0];
        buf[1] = b[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);  
        temp2 = _mm_mul_pd(*index2_simd, *buf_simd);

        add_temp = _mm_add_pd(temp0, temp1);
        add_temp = _mm_add_pd(add_temp, temp2);
        add_temp = _mm_add_pd(add_temp, *number16_simd);

        _mm_store_pd(buf, add_temp);

        ypos = j;

        out_data[ypos + 0] = buf[0];
        out_data[ypos + 1] = buf[1];

        ////////////////////////////////////////////////////////

        buf[0] = r[i + 0];
        buf[1] = r[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);
        temp0 = _mm_mul_pd(*index3_simd, *buf_simd);

        buf[0] = g[i + 0];
        buf[1] = g[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);        
        temp1 = _mm_mul_pd(*index4_simd, *buf_simd);

        buf[0] = b[i + 0];
        buf[1] = b[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);  
        temp2 = _mm_mul_pd(*index5_simd, *buf_simd);

        add_temp = _mm_sub_pd(temp0, temp1);
        add_temp = _mm_sub_pd(add_temp, temp2);
        add_temp = _mm_add_pd(add_temp, *number128_simd);

        _mm_store_pd(buf, add_temp);

        vpos = frame_size + j;

        out_data[vpos + 0] = buf[0];
        out_data[vpos + 1] = buf[1];

        ////////////////////////////////////////////////////////

        buf[0] = r[i + 0];
        buf[1] = r[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);
        temp0 = _mm_mul_pd(*index6_simd, *buf_simd);
        
        buf[0] = g[i + 0];
        buf[1] = g[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);        
        temp1 = _mm_mul_pd(*index7_simd, *buf_simd);

        buf[0] = b[i + 0];
        buf[1] = b[i + 1];
        buf_simd = reinterpret_cast<__m128d*>(buf);  
        temp2 = _mm_mul_pd(*index8_simd, *buf_simd);

        add_temp = _mm_sub_pd(temp0, temp1);
        add_temp = _mm_add_pd(add_temp, temp2);
        add_temp = _mm_add_pd(add_temp, *number128_simd);

        _mm_store_pd(buf, add_temp);

        upos = frame_size/4 + 1;

        out_data[upos + 0] = buf[0];
        out_data[upos + 1] = buf[1];
    }
}


uint8_t *bmp_to_yuv(uint8_t *in_bmp_data) {
    int width = width_bmp(in_bmp_data);
    int height = height_bmp(in_bmp_data);
    size_t frame_size = width * height;
    size_t yuv_size = frame_size * 1.5;

    uint8_t *yuv_data;
    try {
        yuv_data = new uint8_t [frame_size * 2];
    }
    catch (const std::bad_alloc &) {
        return nullptr;
    }

    memset(yuv_data, 0, yuv_size * sizeof(char));

    in_bmp_data = &in_bmp_data[54];

    /* Set load */
    int nprocs = get_ncpus();
    int interval = frame_size/nprocs;
    int first_index;
    int end_index;

    std::thread thread_pool[nprocs];

    for (int i = 0; i < nprocs; i++) {
        first_index = i * interval;
        end_index = first_index + interval;
        if ((i + 1) == nprocs) {
            end_index = frame_size;
        }
        thread_pool[i] = std::thread(thread_convert,
            in_bmp_data, yuv_data, first_index, end_index, frame_size);
    }
    for (int i = 0; i < nprocs; i++) {
        if (thread_pool[i].joinable()) {
            thread_pool[i].join();
        }
    }

    return yuv_data;
}
