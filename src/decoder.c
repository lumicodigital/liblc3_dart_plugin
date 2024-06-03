#include "lc3.h"
#include "decoder.h"
#include <stdlib.h>

static bool _initialized = false;

struct Decoder{
  uint8_t bit_depth;
  uint8_t num_channels;
  uint32_t sample_rate;
  uint16_t frame_us;
  lc3_decoder_t lc3_dec;
} decoder;

/*
  * Initialize the decoder with the given parameters.
  * This function allocates some resources on the heap.
  * decoder_destroy() should be called to free these resources and avoid memory leaks.
  * Calling the function more than once, before calling decoder_destroy(), will have no effect.
  * 
  * bit_depth: The bit depth of the input audio.
  * num_channels: The number of channels in the input audio.
  * sample_rate: The sample rate of the input audio.
  * frame_us: The frame duration in microseconds.
  * 
  * Returns LC3_DECODER_OK on success, otherwise an error code.
*/
lc3_decoder_err_t decoder_init(uint8_t bit_depth, uint8_t num_channels, uint32_t sample_rate, uint16_t frame_us){
  if(_initialized){
    return LC3_DECODER_OK;
  }
  _initialized = true;
  decoder.bit_depth = bit_depth;
  decoder.num_channels = num_channels;
  decoder.sample_rate = sample_rate;
  decoder.frame_us = frame_us;
  
  void * mem = malloc(lc3_hr_decoder_size(false, frame_us, sample_rate));
  if(!mem) return LC3_DECODER_MEMORY_ERROR;

  decoder.lc3_dec = lc3_hr_setup_decoder(false, frame_us, sample_rate, 0, mem);
  if(!decoder.lc3_dec){
    free(mem);
    return LC3_DECODER_MEMORY_ERROR;
  }

  return LC3_DECODER_OK;
}


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
lc3_decoder_err_t decoder_decode(const uint8_t* frame_buffer, int16_t* decoded_data){
  if(!_initialized) return LC3_DECODER_NOT_INITIALIZED;
  if(!frame_buffer || !decoded_data) return LC3_DECODER_INVALID_PARAMS;

  int frame_samples = lc3_hr_frame_samples(false, decoder.frame_us, decoder.sample_rate);
  int block_bytes = lc3_hr_frame_block_bytes(false, decoder.frame_us, decoder.sample_rate, 1, decoder.bit_depth);
  if(block_bytes < 0 || frame_samples < 0) return LC3_DECODER_INTERNAL_ERROR;

  enum lc3_pcm_format fmt = decoder.bit_depth == 24 ? LC3_PCM_FORMAT_S16 : LC3_PCM_FORMAT_S24_3LE;
  int ret = lc3_decode(decoder.lc3_dec, frame_buffer, block_bytes, fmt, decoded_data, 1);
  if(ret < 0) return LC3_DECODER_INTERNAL_ERROR;

  return LC3_DECODER_OK;
}

/*
  * Returns the number of bytes in the frame block.
*/
int decoder_get_block_bytes(){
  return lc3_hr_frame_block_bytes(false, decoder.frame_us, decoder.sample_rate, 1, decoder.bit_depth);
}

/*
  * Returns the number of samples in a frame.
*/
int decoder_get_frame_samples(){
  return lc3_hr_frame_samples(false, decoder.frame_us, decoder.sample_rate);
}

/*
  * Dealocates the resources allocated by decoder_init().
*/
void decoder_destroy(){
  if(!_initialized) return;
  _initialized = false;
  free(decoder.lc3_dec);
}