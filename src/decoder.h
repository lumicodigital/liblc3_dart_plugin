#ifndef LC3_DECODER_H
#define LC3_DECODER_H

#include <stdint.h>

//#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
//
//#if _WIN32
//#include <windows.h>
//#else
//#include <pthread.h>
//#include <unistd.h>
//#endif
//
//#if _WIN32
//#define FFI_PLUGIN_EXPORT __declspec(dllexport)
//#else
//#define FFI_PLUGIN_EXPORT
//#endif

typedef enum {
  LC3_DECODER_OK = 0,
  LC3_DECODER_NOT_INITIALIZED,
  LC3_DECODER_INVALID_PARAMS,
  LC3_DECODER_MEMORY_ERROR,
  LC3_DECODER_INTERNAL_ERROR
} lc3_decoder_err_t;

/*
  * Initialize the decoder with the given parameters.
  * This function allocates some resources on the heap.
  * decoder_destroy() should be called to free these resources and avoid memory leaks.
  * 
  * bit_depth: The bit depth of the input audio.
  *     Accepted values are 16, 24, 32
  * num_channels: The number of channels in the input audio. Currently only supports 1 channel.
  *     If num_channels is not 1, LC3_DECODER_INVALID_PARAMS will be returned.
  * sample_rate: The sample rate of the input audio.
  *     Accepted values are 8000, 16000, 24000, 32000, 48000
  * frame_us: The frame duration in microseconds.
  *     Accepted values are 2500, 5000, 7500, 10000
  * 
  * Returns LC3_DECODER_OK on success, otherwise an error code.
*/
lc3_decoder_err_t decoder_init(uint8_t bit_depth, uint8_t num_channels, uint32_t sample_rate, uint16_t frame_us, uint32_t bit_rate);


/*
  * Decode the given frame buffer.
  * The function expects the frame buffer to be of the size returned by decoder_get_block_bytes().
  * The decoded data buffer should be of the size returned by decoder_get_frame_samples() * (bitdepth // 8).
  * 
  * frame_buffer: The frame buffer to decode.
  * decoded_data: The decoded data.
  * 
  * Returns LC3_DECODER_OK on success, otherwise an error code.
*/
lc3_decoder_err_t decoder_decode(const uint8_t* frame_buffer, uint8_t* decoded_data);

/*
  * Returns the number of bytes in the frame block.
*/
int decoder_get_block_bytes();

/*
  * Returns the number of samples in a frame.
*/
int decoder_get_frame_samples();

/*
  * Dealocates the resources allocated by decoder_init().
*/
void decoder_destroy();

#endif