#include "lc3.h"
#include "decoder.h"
#include <stdlib.h>

static bool _initialized = false;

struct Decoder{
    uint8_t bit_depth;
    uint8_t num_channels;
    uint32_t sample_rate;
    uint16_t frame_us;
    uint32_t bit_rate;
    lc3_decoder_t lc3_dec;
} decoder;

/*
  * Initialize the decoder with the given parameters.
  * This function allocates some resources on the heap.
  * decoder_destroy() should be called to free these resources and avoid memory leaks.
  *
  * bit_depth: The bit depth of the input audio.
  *     Accepted values are 16, 24
  * num_channels: The number of channels in the input audio. Currently only supports 1 channel.
  *     If num_channels is not 1, LC3_DECODER_INVALID_PARAMS will be returned.
  * sample_rate: The sample rate of the input audio.
  *     Accepted values are 8000, 16000, 24000, 32000, 48000
  * frame_us: The frame duration in microseconds.
  *     Accepted values are 2500, 5000, 7500, 10000
  *
  * Returns LC3_DECODER_OK on success, otherwise an error code.
*/
lc3_decoder_err_t decoder_init(
        uint8_t bit_depth,
        uint8_t num_channels,
        uint32_t sample_rate,
        uint16_t frame_us,
        uint32_t bit_rate){
    if(_initialized){
        return LC3_DECODER_OK;
    }
    if(bit_depth != 16 && bit_depth != 24){
        return LC3_DECODER_INVALID_PARAMS;
    }
    if (num_channels != 1){
        return LC3_DECODER_INVALID_PARAMS;
    }
    if(sample_rate != 8000 && sample_rate != 16000 && sample_rate != 24000 && sample_rate != 32000 && sample_rate != 48000){
        return LC3_DECODER_INVALID_PARAMS;
    }
    if(frame_us != 2500 && frame_us != 5000 && frame_us != 7500 && frame_us != 10000){
        return LC3_DECODER_INVALID_PARAMS;
    }
    decoder.bit_depth = bit_depth;
    decoder.num_channels = num_channels;
    decoder.sample_rate = sample_rate;
    decoder.frame_us = frame_us;
    decoder.bit_rate = bit_rate;

    void * mem = malloc(lc3_hr_decoder_size(false, frame_us, sample_rate));
    if(!mem) {
        return LC3_DECODER_MEMORY_ERROR;
    }

    decoder.lc3_dec = lc3_hr_setup_decoder(false, frame_us, sample_rate, 0, mem);
    if(!decoder.lc3_dec){
        free(mem);
        return LC3_DECODER_INTERNAL_ERROR;
    }
    _initialized = true;

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
    int block_bytes = decoder_get_block_bytes();
    if(block_bytes < 0 || frame_samples < 0) return LC3_DECODER_INTERNAL_ERROR;

    enum lc3_pcm_format fmt = decoder.bit_depth == 24 ? LC3_PCM_FORMAT_S24_3LE : LC3_PCM_FORMAT_S16;
    int ret = lc3_decode(decoder.lc3_dec, frame_buffer, block_bytes, fmt, decoded_data, decoder.num_channels);
    if(ret < 0) return LC3_DECODER_INTERNAL_ERROR;

    return LC3_DECODER_OK;
}

/*
  * Returns the number of bytes in the frame block.
*/
int decoder_get_block_bytes(){
    return lc3_hr_frame_block_bytes(
        false,
        decoder.frame_us,
        decoder.sample_rate,
        decoder.num_channels,
        decoder.bit_rate
    );
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