

bool check_wav_header(string data) {
	return string_starts_with(data, STR("RIFF"));
}
bool check_ogg_header(string data) {
	return string_starts_with(data, STR("OggS"));
}

// Supporting more than s16 and f32
// If it's a real thing that there's audio devices which support neither then I will be surprised
// The only format I might consider adding is S32 if it turns out people want VERY detailed audio
typedef enum Audio_Format_Bits {
	AUDIO_BITS_16, // this will be s16
	AUDIO_BITS_32, // this will be f32
} Audio_Format_Bits;
u64 get_audio_bit_width_byte_size(Audio_Format_Bits b) {
    switch (b) {
        case AUDIO_BITS_32: return 4; break;
        case AUDIO_BITS_16: return 2; break;
    }
    panic("");
}
typedef struct Audio_Format {
	Audio_Format_Bits bit_width;
	int channels;
	int sample_rate;
} Audio_Format;

// I don't see a big reason for you to use anything else than WAV and OGG.
// If you use mp3 that's just not very smart.
// Ogg has better quality AND better compression AND you don't need any licensing (which you need for mp3)
// https://convertio.co/mp3-ogg/
// I will probably add mp3 support at some point for compatibility reasonavg.
// - Charlie 2024-07-11
typedef enum Audio_Decoder_Kind {
	AUDIO_DECODER_WAV,
	AUDIO_DECODER_OGG
} Audio_Decoder_Kind;

typedef enum Audio_Source_Kind {
	AUDIO_SOURCE_FILE_STREAM,
	AUDIO_SOURCE_MEMORY, // Raw pcm frames
} Audio_Source_Kind;

typedef struct Audio_Source {

	Audio_Source_Kind kind;
	Audio_Format format;
	u64 number_of_frames;
	Allocator allocator;
	string compressed_data;

	// For file stream
	Audio_Decoder_Kind decoder;
	union {
		drwav wav;
		stb_vorbis *ogg;
	};
	
	// For memory source
	void *pcm_frames;
	
} Audio_Source;

int
_audio_file_stream_sample_frames(Audio_Source *src, u64 first_frame_index, 
					             u64 number_of_frames, void *output_buffer);

bool 
audio_source_init_file_stream(Audio_Source *src, string path, Audio_Format_Bits bit_width, 
							  Allocator allocator) {
	*src = ZERO(Audio_Source);
	
	src->allocator = allocator;
	src->kind = AUDIO_SOURCE_FILE_STREAM;
	
	string data;
	bool read_ok = os_read_entire_file(path, &data, allocator);
	src->compressed_data = data;
	
	third_party_allocator = allocator;
	
	if (!read_ok)  {
			third_party_allocator = ZERO(Allocator);
			return false;
		}
	
	if (check_wav_header(data)) {
		drwav_bool32 init_ok = drwav_init_memory(&src->wav, data.data, data.count, null);
		if (!init_ok)  {
			third_party_allocator = ZERO(Allocator);
			return false;
		}
		src->decoder = AUDIO_DECODER_WAV;
		
		src->format.channels = src->wav.fmt.channels;
		src->format.sample_rate = src->wav.fmt.sampleRate;
		src->number_of_frames = src->wav.totalPCMFrameCount;
	} else if (check_ogg_header(data)) {
		int err;
		src->ogg = stb_vorbis_open_memory(data.data, data.count, &err, null);
		if (!src->ogg) {
			third_party_allocator = ZERO(Allocator);
			return false;
		}
		src->decoder = AUDIO_DECODER_OGG;
		
		stb_vorbis_info info = stb_vorbis_get_info(src->ogg);
		src->format.channels = info.channels;
		src->format.sample_rate = info.sample_rate;
		src->number_of_frames = stb_vorbis_stream_length_in_samples(src->ogg);
	} else {
		log_error("Error in init_audio_source_file_stream(): Unrecognized audio format in file '%s'. We currently support WAV and OGG (Vorbis).", path);
		third_party_allocator = ZERO(Allocator);
		return false;
	}
	
	src->format.bit_width = bit_width;
	
	third_party_allocator = ZERO(Allocator);
	
	return true;
}

bool 
audio_source_init_file_decode(Audio_Source *src, string path, Audio_Format_Bits bit_width, 
							  Allocator allocator) {
	if (!audio_source_init_file_stream(src, path, bit_width, allocator)) return false;
	src->kind = AUDIO_SOURCE_MEMORY;
	
	u64 comp_size = get_audio_bit_width_byte_size(src->format.bit_width);
	u64 total_size = src->number_of_frames * src->format.channels * comp_size;
	src->pcm_frames = alloc(allocator, total_size);
	
	int num_retrieved = _audio_file_stream_sample_frames(src, 0, src->number_of_frames, src->pcm_frames);
	assert(num_retrieved == src->number_of_frames, "decoder failed failed");

	return true;
}

void 
audio_source_destroy(Audio_Source *src) {
	switch (src->kind) {
		case AUDIO_SOURCE_FILE_STREAM: {
			if (src->pcm_frames) dealloc(src->allocator, src->pcm_frames);
			break;
		}
		case AUDIO_SOURCE_MEMORY: {
			dealloc(src->allocator, src->pcm_frames);
			break;
		}
	}
	
	
	third_party_allocator = src->allocator;
	switch (src->decoder) {
		case AUDIO_DECODER_WAV: {
			drwav_uninit(&src->wav);
			break;
		}
		case AUDIO_DECODER_OGG: {
			stb_vorbis_close(src->ogg);
			break;
		}
	}
	third_party_allocator = ZERO(Allocator);
	
	dealloc_string(src->allocator, src->compressed_data);
}

int
_audio_file_stream_sample_frames(Audio_Source *src, u64 first_frame_index, 
					             u64 number_of_frames, void *output_buffer) {
	third_party_allocator = src->allocator;
	int retrieved = 0;
	switch (src->decoder) {
	case AUDIO_DECODER_WAV: 
		bool seek_ok = drwav_seek_to_pcm_frame(&src->wav, first_frame_index);
		assert(seek_ok);
		switch(src->format.bit_width) {
		case AUDIO_BITS_32: {
			retrieved = drwav_read_pcm_frames_f32(
				&src->wav, 
				number_of_frames,
				(f32*)output_buffer
			); 
			break;
		}
		case AUDIO_BITS_16: {
			retrieved = drwav_read_pcm_frames_s16(
				&src->wav, 
				number_of_frames,
				(s16*)output_buffer 
			); 
			break;
		}
		default: panic("Invalid bits value");
	} break; // case AUDIO_DECODER_WAV:
	case AUDIO_DECODER_OGG: 
		seek_ok = stb_vorbis_seek(src->ogg, first_frame_index);
		assert(seek_ok);
		switch(src->format.bit_width) {
		case AUDIO_BITS_32: {
			retrieved = stb_vorbis_get_samples_float_interleaved(
				src->ogg, 
				src->format.channels, 
				(f32*)output_buffer, 
				number_of_frames * src->format.channels
			);
			break;
		}
		case AUDIO_BITS_16: {
			retrieved = stb_vorbis_get_samples_short_interleaved(
				src->ogg, 
				src->format.channels, 
				(s16*)output_buffer, 
				number_of_frames * src->format.channels
			); 
			break;
		}
		default: panic("Invalid bits value");
	} break; // case AUDIO_DECODER_OGG:
	default: panic("Invalid decoder value");
	}
	
	third_party_allocator = ZERO(Allocator);
	return retrieved;
}

u64 // New frame index 
audio_source_sample_frames(Audio_Source *src, u64 first_frame_index, u64 number_of_frames, 
						   void *output_buffer, bool looping) {
	
	u64 comp_size = get_audio_bit_width_byte_size(src->format.bit_width);
    u64 frame_size = comp_size * src->format.channels;
    u64 output_size = number_of_frames * frame_size;
    
    if (first_frame_index == src->number_of_frames) {
    	return first_frame_index;
    }
    
	assert(first_frame_index < src->number_of_frames, "Invalid first_frame_index");
	
    u64 new_index = first_frame_index;
    
    
    
    int num_retrieved;
	switch (src->kind) {
	case AUDIO_SOURCE_FILE_STREAM: {
	
		num_retrieved = _audio_file_stream_sample_frames(
			src, 
			first_frame_index, 
			number_of_frames, 
			output_buffer
		);
		new_index += num_retrieved;
		
		assert(num_retrieved <= number_of_frames);
		
		if (num_retrieved < number_of_frames) {
			void *dst_remain = ((u8*)output_buffer) + num_retrieved*frame_size;
			if (looping) {
				num_retrieved = _audio_file_stream_sample_frames(
					src, 
					0, 
					number_of_frames-num_retrieved, 
					dst_remain
				);
				new_index = number_of_frames-num_retrieved;
			} else {
				memset(dst_remain, 0, frame_size * (number_of_frames - num_retrieved));
			}	
		}
		
		break; // case AUDIO_SOURCE_FILE_STREAM
	}
	case AUDIO_SOURCE_MEMORY: {
		s64 first_number_of_frames = min(number_of_frames, src->number_of_frames-first_frame_index);
		void *src_pcm_start = (u8*)src->pcm_frames + first_frame_index*frame_size;
		
		memcpy(output_buffer, src_pcm_start, first_number_of_frames*frame_size);
		new_index += first_number_of_frames;
		
		s64 remainder = number_of_frames-first_number_of_frames;
		if (remainder > 0) {
			void *dst_remain = (u8*)output_buffer + first_number_of_frames*frame_size;
			
			if (looping) {
				memcpy(dst_remain, src->pcm_frames, frame_size*remainder);
				new_index = remainder;
			} else {
				memset(dst_remain, 0, frame_size*remainder);
			}
		}
		
		break;
	}
	}
	
	return new_index;
}

#define U8_MAX  255
#define S16_MIN -32768
#define S16_MAX 32767
#define S24_MIN -8388608
#define S24_MAX 8388607
#define S32_MIN -2147483648
#define S32_MAX 2147483647

void 
mix_frames(void *dst, void *src, u64 frame_count, Audio_Format format) {
    u64 comp_size = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;
    u64 output_size = frame_count * frame_size;
    
    // #Speed #Simd #Incomplete
    // Quality:
    // - Dithering
    // - Clipping. Dynamic Range Compression?
    
    for (u64 frame = 0; frame < frame_count; frame++) {
        
        for (u64 c = 0; c < format.channels; c++) {

            void *src_sample = (u8*)src + frame*frame_size + c*comp_size;
            void *dst_sample = (u8*)dst + frame*frame_size + c*comp_size;

            switch (format.bit_width) {
                case AUDIO_BITS_32: {
                	*((f32*)dst_sample) += *((f32*)src_sample);
            	}
                case AUDIO_BITS_16: {
                    s16 dst_int = *((s16*)dst_sample);
                    s16 src_int = *((s16*)src_sample);
                    *((s16*)dst_sample) = (s16)clamp((s64)(dst_int + src_int), S16_MIN, S16_MAX);
                    break;
                }
            }
        }
    }
}

void
convert_one_component(void *dst, Audio_Format_Bits dst_bits, 
                  void *src, Audio_Format_Bits src_bits) {
	switch (dst_bits) {
		case AUDIO_BITS_32: {
			switch (src_bits) {
			case AUDIO_BITS_32: 
				memcpy(dst, src, get_audio_bit_width_byte_size(dst_bits)); break;
			case AUDIO_BITS_16: 
				// #Simd
				*(f32*)dst = (f64)((f32)*((s16*)src) * ((f64)1.0 / (f64)32768.0));
				break;
			default: panic("Unhandled bits");
			}
			break;
		}
		case AUDIO_BITS_16: {
			switch (src_bits) {
			case AUDIO_BITS_32:
				// #Simd
				*(s16*)dst = (s16)(*((f32*)src) * 32768.0f);
				break;
			case AUDIO_BITS_16:
				memcpy(dst, src, get_audio_bit_width_byte_size(dst_bits)); 
				break;
			default: panic("Unhandled bits");
			}
			break;
		}
		default: panic("Unhandled bits");
	}
}

// Assume dst buffer is large enough
// in-place conversion is OK
void
resample_frames(void *dst, Audio_Format dst_format, 
                void *src, Audio_Format src_format, u64 src_frame_count) {
    assert(dst_format.channels == src_format.channels, "Channel count must be the same for sample rate conversion");
    assert(dst_format.bit_width == src_format.bit_width, "Types must be the same for sample rate conversion");

    f32 src_ratio = (f32)src_format.sample_rate / (f32)dst_format.sample_rate;
    u64 dst_frame_count = (u64)round(src_frame_count / src_ratio);
    u64 dst_comp_size = get_audio_bit_width_byte_size(dst_format.bit_width);
    u64 dst_frame_size = dst_comp_size * dst_format.channels;
    u64 src_comp_size = get_audio_bit_width_byte_size(src_format.bit_width);
    u64 src_frame_size = src_comp_size * src_format.channels;

	// Reverse in case dst == src (so we can do in-place conversion)
    for (s64 dst_frame_index = dst_frame_count - 1; dst_frame_index >= 1; dst_frame_index--) {
        f32 src_frame_index_f = dst_frame_index * src_ratio;
        u64 src_frame_index_1 = (u64)src_frame_index_f;
        u64 src_frame_index_2 = src_frame_index_1 + 1;
        if (src_frame_index_2 >= src_frame_count) src_frame_index_2 = src_frame_count - 1;

        f32 lerp_factor = src_frame_index_f - (f32)src_frame_index_1;

        void *src_frame_1 = (u8*)src + src_frame_index_1 * src_frame_size;
        void *src_frame_2 = (u8*)src + src_frame_index_2 * src_frame_size;
        void *dst_frame = (u8*)dst + dst_frame_index * dst_frame_size;

        for (int c = 0; c < src_format.channels; c++) {
            union {
                s16 s16_sample;
                f32 f32_sample;
                u8 data[4];
            } sample_dst;

            void *src_comp_1 = (u8*)src_frame_1 + c * src_comp_size;
            void *src_comp_2 = (u8*)src_frame_2 + c * src_comp_size;
            void *dst_comp = (u8*)dst_frame + c * dst_comp_size;

            if (src_format.bit_width == AUDIO_BITS_32) {
                float sample_1 = *((f32*)src_comp_1);
                float sample_2 = *((f32*)src_comp_2);
                sample_dst.f32_sample = sample_1 + lerp_factor * (sample_2 - sample_1);
            } else if (src_format.bit_width == AUDIO_BITS_16) {
                s16 sample_1 = *((s16*)src_comp_1);
                s16 sample_2 = *((s16*)src_comp_2);
                sample_dst.s16_sample = (s16)((f32)sample_1 + lerp_factor * ((f32)sample_2 - (f32)sample_1));
            } else {
                panic("Unhandled bit width");
            }

            memcpy(dst_comp, sample_dst.data, dst_comp_size);
        }
    }
    
    // Correct padding if we downscaled (since we coverted in reverse)
    if (src == dst && dst_format.sample_rate < src_format.sample_rate) {
    	void *dst_after_pad = (u8*)dst + (src_frame_count - dst_frame_count) * dst_frame_size;
    	u64 padding = (u64)dst_after_pad - (u64)dst;
    	memcpy(
    		dst, 
    		dst_after_pad, 
    		dst_frame_count * dst_frame_size
		);
		memset((u8*)dst+dst_frame_count * dst_frame_size, 0, padding);
	}
	
}

// Assumes dst buffer is large enough
void 
convert_frames(void *dst, Audio_Format dst_format, 
               void *src, Audio_Format src_format, u64 src_frame_count) {

	u64 dst_comp_size = get_audio_bit_width_byte_size(dst_format.bit_width);
    u64 dst_frame_size = dst_comp_size * dst_format.channels;
	u64 src_comp_size = get_audio_bit_width_byte_size(src_format.bit_width);
    u64 src_frame_size = src_comp_size * src_format.channels;
	   
    if (dst_format.sample_rate != src_format.sample_rate) {
    	f32 ratio = (f32)src_format.sample_rate/(f32)dst_format.sample_rate;
    	src_frame_count = (u64)round((f32)src_frame_count*ratio);
    }

	if (bytes_match(&dst_format, &src_format, sizeof(Audio_Format))) {
		memcpy(dst, src, src_frame_count*src_frame_size);
		return;
	}
	
	u64 output_frame_count = src_frame_count;
	
	// #Speed #Simd
	if (dst_format.channels != src_format.channels || dst_format.bit_width != src_format.bit_width) {
		for (u64 src_frame_index = 0; src_frame_index < src_frame_count; src_frame_index++) {
	        void *src_frame = ((u8*)src) + src_frame_index*src_frame_size;
	        void *dst_frame = ((u8*)dst) + src_frame_index*dst_frame_size;
	        
	        // For getting average src sample
	    	union {
	    		s16 s16_sample;
	    		f32 f32_sample;
	    		u8 data[4];
	    	} avg;
	        if (src_format.channels != dst_format.channels) {
	        	// This is where we get the average src sample
	        	f32 sum = 0;
	        	for (int c = 0; c < src_format.channels; c++) {
	        		avg.s16_sample = 0;
	        		void *src_comp = (u8*)src_frame + c * src_comp_size;
	        		convert_one_component(
	        			avg.data, dst_format.bit_width, 
	        			src_comp, src_format.bit_width
	    			);
	    			if (dst_format.bit_width == AUDIO_BITS_32)      sum += avg.f32_sample;
	    			else if (dst_format.bit_width == AUDIO_BITS_16) sum += (f32)avg.s16_sample;
	    			else panic("Unhandled bit width");
	        	}
	        	if (dst_format.bit_width == AUDIO_BITS_32) {
	        		avg.f32_sample = sum/(f32)src_format.channels;
				} else if (dst_format.bit_width == AUDIO_BITS_16) {
					avg.s16_sample = (s16)round(sum/(f32)src_format.channels);
				} else panic("Unhandled bit width");
	        }
	        
	        if (src_format.channels > dst_format.channels) {
	        
	        	// #Limitation #Audioquality
	        	// Here we are down-scaling the channel count.
	        	// So what we do is we get the average sample for all channels in src and then 
	        	// set all channels in dst to that. This is fine for mono to stereo, but will
	        	// be a loss for example for surround to mono. But I'm not sure we will ever
	        	// care about non-stereo/mono audio.
	        	
				for (int c = 0; c < dst_format.channels; c++) {
					void *dst_comp = (u8*)dst_frame + c * dst_comp_size;
					memcpy(dst_comp, avg.data, dst_comp_size);
				}
				
	        } else if (dst_format.channels > src_format.channels) {
	        	
	        	// Here, we are upscaling to a higher channel count.
	        	// I'm not sure what the best way to do this is, but for now I will try to just
	        	// get the average in src and set that to the extra channels in dst.
	        	// This is obviously fine for mono -> stereo but might be a problem for surround.
	        	// Again, I'm not sure if surround will ever be on our list of worries.
	        	
	        	for (int c = 0; c < dst_format.channels; c++) {
	        		void *dst_comp = (u8*)dst_frame + c * dst_comp_size;
	        		void *src_comp = (u8*)src_frame + c * src_comp_size;
	        		
	        		if (c < src_format.channels) 
	        			convert_one_component(dst_comp, dst_format.bit_width,
	        								  src_comp, src_format.bit_width);
	        		else 
	        			memcpy(dst_comp, avg.data, dst_comp_size);
	        	}
	        	
	        } else {
	        	// Same channel count, just copy components over
	        	for (int c = 0; c < dst_format.channels; c++) {
                    void *dst_comp = (u8*)dst_frame + c * dst_comp_size;
                    void *src_comp = (u8*)src_frame + c * src_comp_size;
                    convert_one_component(dst_comp, dst_format.bit_width, src_comp, src_format.bit_width);
                }
	        }
	    }
    }
    if (dst_format.sample_rate != src_format.sample_rate) {
    	resample_frames(
    		dst, 
    		(Audio_Format){dst_format.bit_width, dst_format.channels, dst_format.sample_rate},
			dst, 
			(Audio_Format){dst_format.bit_width, dst_format.channels, src_format.sample_rate},
			src_frame_count
		);
		
    }
}







// #Temporary this is jsut for testing
Audio_Source *current_source = 0;
u64 current_index = 0;

// This is supposed to be called by OS layer audio thread whenever it wants more audio samples
void do_program_audio_sample(u64 number_of_output_frames, Audio_Format out_format, 
							 void *output) {
	u64 out_comp_size = get_audio_bit_width_byte_size(out_format.bit_width);
    u64 out_frame_size = out_comp_size * out_format.channels;
    u64 output_size = number_of_output_frames * out_frame_size;
    
	memset(output, 0, output_size);
	if (current_source) {

		bool need_convert = !bytes_match(&out_format, &current_source->format, sizeof(Audio_Format));
		
		u64 in_comp_size = get_audio_bit_width_byte_size(current_source->format.bit_width);
		u64 in_frame_size = in_comp_size * current_source->format.channels;
		u64 input_size = number_of_output_frames * in_frame_size;
		
		void *target_buffer = output;
		u64 number_of_sample_frames = number_of_output_frames;
		
		thread_local local_persist void *convert_buffer = 0;
		thread_local local_persist u64 convert_buffer_size;
		if (need_convert) {
			if (current_source->format.sample_rate != out_format.sample_rate) {
				f32 src_ratio 
					= (f32)current_source->format.sample_rate / (f32)out_format.sample_rate;
				number_of_sample_frames = round(number_of_output_frames * src_ratio);
				input_size = number_of_sample_frames * in_frame_size;
			}
			
			u64 biggest_size = max(input_size, output_size);
			
			if (!convert_buffer || convert_buffer_size < biggest_size) {
				// #Speed
				if (convert_buffer) dealloc(get_heap_allocator(), convert_buffer);
				convert_buffer = alloc(get_heap_allocator(), biggest_size);
				convert_buffer_size = biggest_size;
			}
			target_buffer = convert_buffer;
			memset(convert_buffer, 0, biggest_size);
			
		}

		current_index = audio_source_sample_frames(
			current_source, 
			current_index, 
			number_of_sample_frames,
			target_buffer,
			true
		);
		if (need_convert) {
			convert_frames(
				output, 
				out_format, 
				convert_buffer, 
				current_source->format,
				number_of_output_frames
			);
		}
	}
}