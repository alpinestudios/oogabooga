
/*
bool check_wav_header(string data) {
	return string_starts_with(data, STR("RIFFWAVE"));
}
bool check_ogg_header(string data) {
	return string_starts_with(data, STR("oggs"));
}

typedef enum Audio_Format_Type {
	AUDIO_FORMAT_TYPE_UNKNOWN, 
	AUDIO_FORMAT_TYPE_U8, 
	AUDIO_FORMAT_TYPE_S16, 
	AUDIO_FORMAT_TYPE_S24, 
	AUDIO_FORMAT_TYPE_S32, 
	AUDIO_FORMAT_TYPE_F32
} Audio_Format_Type;
typedef struct Audio_Format {
	Audio_Format_Type type;
	int channels;
	int sample_rate;
} Audio_Format;

u64 get_audio_format_component_byte_size(Audio_Format_Type format) {
    switch (format) {
        case AUDIO_FORMAT_TYPE_F32: return 4; break;
        case AUDIO_FORMAT_TYPE_S32: return 4; break;
        case AUDIO_FORMAT_TYPE_S24: return 3; break;
        case AUDIO_FORMAT_TYPE_S16: return 2; break;
        case AUDIO_FORMAT_TYPE_U8: return 1; break;
        case AUDIO_FORMAT_TYPE_UNKNOWN: return 0; break;
    }
    panic("");
}
#define U8_MAX  255
#define S16_MIN -32768
#define S16_MAX 32767
#define S24_MIN -8388608
#define S24_MAX 8388607
#define S32_MIN -2147483648
#define S32_MAX 2147483647

void mix_frames(void *dst, void *src, u64 frame_count, Audio_Format format) {
    u64 comp_size = get_audio_format_component_byte_size(format.type);
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

            switch (format.type) {
                case AUDIO_FORMAT_TYPE_F32: {
                	*((f32*)dst_sample) += *((f32*)src_sample);
            	}
                case AUDIO_FORMAT_TYPE_S32: {
                    s32 dst_int = *((s32*)dst_sample);
                    s32 src_int = *((s32*)src_sample);
                    *((s32*)dst_sample) = (s32)clamp((s64)(dst_int + src_int), S32_MIN, S32_MAX);
                    break;
                }
                case AUDIO_FORMAT_TYPE_S24: {
                    s64 src_int = 0;
                    memcpy(&src_int, src_sample, 3);

                    src_int <<= 40;
                    src_int >>= 40;

                    s64 dst_int;
                    memcpy(&dst_int, dst_sample, 3); 

                    dst_int <<= 40;
                    dst_int >>= 40;

                    s64 sum = clamp(src_int + dst_int, S24_MIN, S24_MAX);
                    memcpy(dst_sample, &sum, 3);
                    
                    break;
                }
                    
                case AUDIO_FORMAT_TYPE_S16: {
                    s16 dst_int = *((s16*)dst_sample);
                    s16 src_int = *((s16*)src_sample);
                    *((s16*)dst_sample) = (s16)clamp((s64)(dst_int + src_int), S16_MIN, S16_MAX);
                    break;
                }
                    
                case AUDIO_FORMAT_TYPE_U8: {
                    u8 dst_int = *((u8*)dst_sample);
                    u8 src_int = *((u8*)src_sample);
                    *((u8*)dst_sample) = (u8)clamp((s64(dst_int + src_int), 0, U8_MAX);
                    
                    break;
                }
                    
                case AUDIO_FORMAT_TYPE_UNKNOWN: break;
            }
        }
    }
}

*/