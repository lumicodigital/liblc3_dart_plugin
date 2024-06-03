#ifndef LC3_DECODER_H
#define LC3_DECODER_H

#include <stdint.h>

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
  * num_channels: The number of channels in the input audio.
  * sample_rate: The sample rate of the input audio.
  * frame_us: The frame duration in microseconds.
  * 
  * Returns LC3_DECODER_OK on success, otherwise an error code.
*/
lc3_decoder_err_t decoder_init(uint8_t bit_depth, uint8_t num_channels, uint32_t sample_rate, uint16_t frame_us);


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
lc3_decoder_err_t decoder_decode(const uint8_t* frame_buffer, int16_t* decoded_data);

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