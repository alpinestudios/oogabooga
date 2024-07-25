
/*

		Loading audio:
		
	bool audio_open_source_stream(Audio_Source *src, string path, Allocator allocator);
	bool audio_open_source_load(Audio_Source *src, string path, Allocator allocator);
	void audio_source_destroy(Audio_Source *src);

		Playing audio (the simple way):
		
	void play_one_audio_clip_source(Audio_Source source);
	void play_one_audio_clip(string path);
	void play_one_audio_clip_source_at_position(Audio_Source source, Vector3 pos);
	void play_one_audio_clip_at_position(string path, Vector3 pos);
	
		Playing audio (with players):
	
	Audio_Player * audio_player_get_one();
	void           audio_player_release(Audio_Player *p);
	
	void    audio_player_set_state(Audio_Player *p, Audio_Player_State state);
	void    audio_player_set_time_stamp(Audio_Player *p, float64 time_in_seconds);
	void    audio_player_set_progression_factor(Audio_Player *p, float64 factor);
	float64 audio_player_get_time_stamp(Audio_Player *p);
	float64 audio_player_get_current_progression_factor(Audio_Player *p);
	void    audio_player_set_source(Audio_Player *p, Audio_Source src, bool retain_progression_factor);
	void    audio_player_clear_source(Audio_Player *p);
	void    audio_player_set_looping(Audio_Player *p, bool looping);
	
*/



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

// #Global
// Implemented per OS
ogb_instance Audio_Format audio_output_format; 
ogb_instance Mutex audio_init_mutex;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Audio_Format audio_output_format; 
Mutex audio_init_mutex;
#endif

// I don't see a big reason for you to use anything else than WAV and OGG.
// If you use mp3 that's just not very smart.
// Ogg has better quality AND better compression AND you don't need any licensing (which you need for mp3)
// https://convertio.co/mp3-ogg/
// I will probably add mp3 support at some point for compatibility reasons.
// - Charlie 2024-07-11
typedef enum Audio_Decoder_Kind {
	AUDIO_DECODER_WAV,
	AUDIO_DECODER_OGG
} Audio_Decoder_Kind;

typedef enum Audio_Source_Kind {
	AUDIO_SOURCE_FILE_STREAM,
	AUDIO_SOURCE_MEMORY, // Raw pcm frames
} Audio_Source_Kind;

typedef struct {
    u32 data1;
    u16 data2;
    u16 data3;
    u8  data4[8];
} Wav_Subformat_Guid;
inline bool is_equal_wav_guid(const Wav_Subformat_Guid* a, const Wav_Subformat_Guid* b) {
    return (
        a->data1 == b->data1 &&
        a->data2 == b->data2 &&
        a->data3 == b->data3 &&
        memcmp(a->data4, b->data4, 8) == 0
    );
}

const Wav_Subformat_Guid WAV_SUBTYPE_PCM = (Wav_Subformat_Guid){ 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
const Wav_Subformat_Guid WAV_SUBTYPE_IEEE_FLOAT = (Wav_Subformat_Guid){ 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

typedef struct Wav_Stream {
	File file;
	int channels;
	int sample_rate;
	int format;
	int bits_per_sample;
	u64 number_of_frames;
	u64 pcm_start;
	u64 valid_bits_per_sample;
	Wav_Subformat_Guid sub_format;
} Wav_Stream;

typedef struct Audio_Source {

	Audio_Source_Kind kind;
	Audio_Format format;
	u64 number_of_frames;
	Allocator allocator;

	// For file stream
	Audio_Decoder_Kind decoder;
	union {
		Wav_Stream wav;
		stb_vorbis *ogg;
	};
	
	// #Memory #Incomplete #StbVorbisFileStream
	// I tried replacing the stdio stuff in stb_vorbis with oogabooga file api, but now
	// stb_vorbis is shitting itself.
	// So, for now, we're just reading the file into memory and then streaming from that
	// memory.
	// Luckily, ogg compression is pretty good so it's not going to completely butcher
	// memory usage, but it's definitely suboptimal.
	string ogg_raw;
	
	// For memory source
	void *pcm_frames;
	
	Mutex mutex_for_destroy; // This should ONLY be used so a source isnt sampled on audio thread while it's being destroyed
	
} Audio_Source;

int 
convert_frames(void *dst, Audio_Format dst_format, 
               void *src, Audio_Format src_format, u64 src_frame_count);

bool 
check_wav_header(string data) {
	return string_starts_with(data, STR("RIFF"));
}
bool 
check_ogg_header(string data) {
	return string_starts_with(data, STR("OggS"));
}

bool 
wav_open_file(string path, Wav_Stream *wav, u64 sample_rate, u64 *number_of_frames) {

	// https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
	// Seriously, why is everything Microsoft makes so stinky when it should be so simple?
	// The above specification is pretty straight forward and to the point, but then I
	// start finding nonsense in random WAV files because some people thought it was a
	// good idea to try and add "extensions" to it and now there is LITERALLY junk in
	// wave files that should just be so simple.
	// Apparently, some wave files are just... mpeg audio... with a wave header...
	// I JUST WANT SOME PCM FRAMES

    wav->file = os_file_open(path, O_READ);
    
    if (wav->file == OS_INVALID_FILE) return false;
    
    string header = talloc_string(12);
    
    u64 read;
    bool ok = os_file_read(wav->file, header.data, 12, &read);
    if (!ok || read != 12) {
        os_file_close(wav->file);
        return false;
    }
    

	if (!strings_match(string_view(header, 0, 4), STR("RIFF"))) return false;
	if (!strings_match(string_view(header, 8, 4), STR("WAVE"))) {
		log_error("Invalid header in wave file @ %s", path);
		os_file_close(wav->file);
		return false;
	}
	
	u32 number_of_sub_chunk_bytes = *(u32*)(header.data+4);
	number_of_sub_chunk_bytes -= 4; // ckID
	
	u32 avg_bytes_per_sec;
	u16 block_align;
	
	string chunk_header = talloc_string(8);
	const u64 NON_DATA_CHUNK_MAX_SIZE = 40;
    string chunk = talloc_string(NON_DATA_CHUNK_MAX_SIZE);
    
	for (u64 byte_pos = 4; byte_pos < number_of_sub_chunk_bytes;) {
		ok = os_file_read(wav->file, chunk_header.data, 8, &read);
	    if (!ok || read != 8) {
	        os_file_close(wav->file);
	        return false;
	    }
	    byte_pos += 8;
	    
	    string chunk_id = string_view(chunk_header, 0, 4);
	    u32 chunk_size = *(u32*)(chunk_header.data+4);
	    
	    byte_pos += chunk_size;
	    
	    // Ignored chunks
	    // THIS IS WHY WE CAN'T HAVE NICE THINGS
	    // (yes, some files actually has a chunk with id "junk")
	    if (strings_match(chunk_id, STR("bext"))
	     || strings_match(chunk_id, STR("fact"))
	     || strings_match(chunk_id, STR("junk"))) {
	     	u64 pos = os_file_get_pos(wav->file);
	     	os_file_set_pos(wav->file, pos+chunk_size);
	    	continue;
	    }
	    
	    if (!strings_match(chunk_id, STR("data")) && chunk_size <= NON_DATA_CHUNK_MAX_SIZE) {
	    	ok = os_file_read(wav->file, chunk.data, chunk_size, &read);
		    if (!ok || read != chunk_size) {
		        os_file_close(wav->file);
		        return false;
		    }
	    }
	    
	    if (strings_match(chunk_id, STR("fmt "))) {
	    	
	    	if (chunk_size != 16 && chunk_size != 18 && chunk_size != 40) {
	    		log_error("Invalid wav fmt chunk, bad size %d", chunk_size);
	    		os_file_close(wav->file);
	    		return false;
	    	}
	    	
	    	if (chunk_size >= 16) {
	    		wav->format           = *(u16*)(chunk.data+0);
	    		wav->channels         = *(u16*)(chunk.data+2);
	    		wav->sample_rate      = *(u32*)(chunk.data+4);
	    		avg_bytes_per_sec     = *(u32*)(chunk.data+8);
	    		block_align           = *(u16*)(chunk.data+12);
	    		wav->bits_per_sample  = *(u16*)(chunk.data+14);
	    		
	    		wav->valid_bits_per_sample = wav->bits_per_sample;
	    	}
	    	if (chunk_size == 40) {
	    		/* cbSize @+16 */
	    		wav->valid_bits_per_sample =                *(u16*)(chunk.data+18);
	    		// u32 channel_mask        =                *(u16*)(chunk.data+20);
	    		wav->sub_format            = *(Wav_Subformat_Guid*)(chunk.data+24);
	    	}
	    	
	    } else if (strings_match(chunk_id, STR("data"))) {
	    	
	    	u64 number_of_bytes = chunk_size;
	    	
	    	if (number_of_bytes % 2 != 0) {
	    		// Consume pad byte
	    		u8 pad;
	    		ok = os_file_read(wav->file, &pad, 1, &read);
			    if (!ok || read != 1) {
			        os_file_close(wav->file);
			        return false;
			    }
	    	}
	    	
	    	wav->pcm_start = byte_pos - chunk_size;
	    	
	    	u64 number_of_samples
	    		= number_of_bytes / (wav->bits_per_sample / 8);
	    		
    		wav->number_of_frames = number_of_samples / wav->channels;
	    	*number_of_frames = wav->number_of_frames; // If same sample rates...
	    } else {
	    	log_warning("Unhandled chunk id '%s' in wave file @ %s", chunk_id, path);
	    	
	    	if (chunk_size > NON_DATA_CHUNK_MAX_SIZE) {
	    		u64 pos = os_file_get_pos(wav->file);
	     		os_file_set_pos(wav->file, pos+chunk_size);
	    	}
	    }
	}
    
    if (wav->format == 0xFFFE) {
        if (is_equal_wav_guid(&wav->sub_format, &WAV_SUBTYPE_PCM)) {
            wav->format = 0x0001;
        } else if (is_equal_wav_guid(&wav->sub_format, &WAV_SUBTYPE_IEEE_FLOAT)) {
            wav->format = 0x0003;
        } else {
            os_file_close(wav->file);
            return false;
        }
    }
    
    if ((wav->format != 0x0001 && wav->format != 0x0003
    	 || wav->format == 0x0003 && wav->valid_bits_per_sample != 32)) {
    	log_error("Wav file @ '%s' format 0x%x (%d bits) is not supported.", path, wav->format, wav->valid_bits_per_sample);
    	return false;
    }
    
    if (wav->valid_bits_per_sample == 24) {
    	log_warning("The current support for 24-bit wave audio is hit-or-miss. If the audio sounds weird, you should convert it to another bit-width (or vorbis).");
    }
    
    if (wav->sample_rate != sample_rate) {
    	f32 ratio = (f32)sample_rate/(f32)wav->sample_rate;
    	*number_of_frames = (u64)round((f32)wav->number_of_frames*ratio);
    }

    // Set the file position to the beginning of the PCM data
    ok = os_file_set_pos(wav->file, wav->pcm_start);
    if (!ok) {
        os_file_close(wav->file);
        return false;
    }
    
    return true;
}
void 
wav_close(Wav_Stream *wav) {
	os_file_close(wav->file);
}
bool 
wav_set_frame_pos(Wav_Stream *wav, u64 output_sample_rate, u64 frame_index) {

	f32 ratio = (f32)wav->sample_rate/(f32)output_sample_rate;
	frame_index = (u64)round(ratio*(f32)frame_index);
	
	u64 frame_size = wav->channels*(wav->bits_per_sample/8);
	return os_file_set_pos(wav->file, wav->pcm_start + frame_index*frame_size);
}
u64 
wav_read_frames(Wav_Stream *wav, Audio_Format format, void *frames, 
				    u64 number_of_frames) {
	s64 pos = os_file_get_pos(wav->file);
	if (pos < wav->pcm_start) return false;
	
	u64 comp_size = wav->bits_per_sample/8;
	u64 frame_size = wav->channels*comp_size;
	
	u64 out_comp_size = get_audio_bit_width_byte_size(format.bit_width);
	u64 out_frame_size = format.channels*out_comp_size;

	// We first convert bit width, and then channels & sample rate
	u64 convert_frame_size = out_comp_size * wav->channels;
	
	u64 end = wav->pcm_start + frame_size*wav->number_of_frames;
	
	u64 remaining_frames = (end-pos)/frame_size;
	
	f32 ratio = (f32)wav->sample_rate / (f32)format.sample_rate;

	u64 frames_to_output = min(round(remaining_frames/ratio), number_of_frames);
	u64 frames_to_read   = frames_to_output;
	
	if (wav->sample_rate != format.sample_rate) {
		
		frames_to_read = (u64)round(ratio*frames_to_output);
	}
	
	u64 required_size 
		= number_of_frames*max(format.channels,wav->channels)*4;
	
	// #Cleanup #Memory refactor intermediate buffers
	thread_local local_persist void *raw_buffer = 0;
	thread_local local_persist u64  raw_buffer_size = 0;
	thread_local local_persist void *convert_buffer = 0;
	thread_local local_persist u64  convert_buffer_size = 0;
	if (!raw_buffer || required_size > raw_buffer_size) {
		if (raw_buffer) dealloc(get_heap_allocator(), raw_buffer);
		
		u64 new_size = get_next_power_of_two(required_size);
		
		raw_buffer = alloc(get_heap_allocator(), new_size);
		memset(raw_buffer, 0, new_size);
		raw_buffer_size = new_size;
	}
	if (!convert_buffer || required_size > convert_buffer_size) {
		if (convert_buffer) dealloc(get_heap_allocator(), convert_buffer);
		
		u64 new_size = get_next_power_of_two(required_size);
		
		convert_buffer = alloc(get_heap_allocator(), new_size);
		memset(convert_buffer, 0, new_size);
		convert_buffer_size = new_size;
	}
	
	u64 frames_read;
	bool ok = os_file_read(wav->file, raw_buffer, frames_to_read*frame_size, &frames_read);
	if (!ok) return 0;
	if (frames_read != frames_to_read*frame_size) {
		os_file_set_pos(wav->file, pos);
		return 0;
	}
	
	bool raw_is_float32 = wav->format == 0x0003;
	bool raw_is_f32 = raw_is_float32 && wav->valid_bits_per_sample == 32;
	bool raw_is_int = wav->format == 0x0001;
	bool raw_is_s16 = raw_is_int && wav->valid_bits_per_sample == 16;

	switch (format.bit_width) {
	case AUDIO_BITS_32: {
		if (raw_is_f32) {
			memcpy(convert_buffer, raw_buffer, frames_to_read*frame_size);
		} else {
			assert(raw_is_int);
			
			// Convert any integer to f32
			for (u64 f = 0; f < frames_to_read; f++) {
			    void *raw_frame = (u8*)raw_buffer + f * frame_size;
			    void *dst_frame = (u8*)convert_buffer + f * convert_frame_size;
			    for (u64 c = 0; c < wav->channels; c++) {
			        void *raw_comp = (u8*)raw_frame + c * comp_size;
			        void *dst_comp = (u8*)dst_frame + c * out_comp_size;
			
			        s64 i = 0;
			        if (wav->valid_bits_per_sample == 32) {
			            i = *(s32*)raw_comp;
			        } else if (wav->valid_bits_per_sample == 16) {
			            i = *(s16*)raw_comp;
			        } else if (wav->valid_bits_per_sample == 24) {
			            u8 bytes[3];
			            memcpy(bytes, raw_comp, 3);
			            
			            u16 test = 0x1;
			            bool is_little_endian = *((u8*)&test) == 0x1;
			            
			            if (is_little_endian) {
			                i = (s32)((bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16));
			            } else {
			                i = (s32)((bytes[2] << 0) | (bytes[1] << 8) | (bytes[0] << 16));
			            }
			            if (i & 0x800000) {
			                i |= ~0xFFFFFF; // Sign extend the 24-bit value
			            }
			        }
			        s64 max = (1LL << (wav->valid_bits_per_sample - 1LL)) - 1LL;
			
			        *(f32*)dst_comp = (f64)i / (f64)max;
			    }
			}
		}
		break;
	}
	case AUDIO_BITS_16: {
		if (raw_is_s16) {
			memcpy(convert_buffer, raw_buffer, frames_to_read*frame_size);
		} else if (raw_is_f32) {
			// Convert f32 to s16
			for (u64 f = 0; f < frames_to_read; f++) {
				void *raw_frame = (u8*)raw_buffer + f*frame_size;
				void *dst_frame = (u8*)convert_buffer + f*convert_frame_size;
				for (u64 c = 0; c < wav->channels; c++) {
					void *raw_comp = (u8*)raw_frame + c*comp_size;
					void *dst_comp = (u8*)dst_frame + c*out_comp_size;
					
					*(s16*)dst_comp = (s16)(*((f32*)raw_comp) * 32768.0f);
				}
			}
		} else {
			assert(raw_is_int);
			// Convert any integer to s16
			for (u64 f = 0; f < frames_to_read; f++) {
				void *raw_frame = (u8*)raw_buffer + f*frame_size;
				void *dst_frame = (u8*)convert_buffer + f*convert_frame_size;
				for (u64 c = 0; c < wav->channels; c++) {
					void *raw_comp = (u8*)raw_frame + c*comp_size;
					void *dst_comp = (u8*)dst_frame + c*out_comp_size;
					
					s64 i = 0;
			        if (wav->valid_bits_per_sample == 32) {
			            i = *(s32*)raw_comp;
			        } else if (wav->valid_bits_per_sample == 16) {
			            i = *(s16*)raw_comp;
			        } else if (wav->valid_bits_per_sample == 24) {
			            u8 bytes[3];
			            memcpy(bytes, raw_comp, 3);
			            
			            u16 test = 0x1;
			            bool is_little_endian = *((u8*)&test) == 0x1;
			            
			            if (is_little_endian) {
			                i = (s32)((bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16));
			            } else {
			                i = (s32)((bytes[2] << 0) | (bytes[1] << 8) | (bytes[0] << 16));
			            }
			            if (i & 0x800000) {
			                i |= ~0xFFFFFF; // Sign extend the 24-bit value
			            }
			        }
			        s64 max = (1LL << (wav->valid_bits_per_sample - 1LL)) - 1LL;
					
					f32 factor = (f64)i * (1.0/(f64)max);
					
					*(s16*)dst_comp = (s16)(factor*32768.0f);
				}
			}
		}
		break;
	}
	}
	
	int converted = convert_frames(
		frames, 
		format,
		convert_buffer, 
		(Audio_Format){ format.bit_width, wav->channels, wav->sample_rate},
		frames_to_read
	);
	assert(converted == frames_to_output);
	
	return frames_to_output;
}
bool 
wav_load_file(string path, void **frames, Audio_Format format, u64 *number_of_frames,
			  Allocator allocator) {
	Wav_Stream wav;
	if (!wav_open_file(path, &wav, format.sample_rate, number_of_frames)) return false;
	
	u64 comp_size = get_audio_bit_width_byte_size(format.bit_width);
	u64 frame_size = comp_size*format.channels;
	
	*frames = alloc(allocator, *number_of_frames*frame_size);
	
	u64 read = wav_read_frames(&wav, format, *frames, *number_of_frames);
	if (read != *number_of_frames) {
		if (read > 0) {
			assert(*frames, "What's goin on here");
		}
		return false;
	}
	
	
	return true;
}


int
audio_source_get_frames(Audio_Source *src, u64 first_frame_index, 
					             u64 number_of_frames, void *output_buffer);


bool
audio_open_source_stream_format(Audio_Source *src, string path, Audio_Format format, 
							    Allocator allocator) {
	*src = ZERO(Audio_Source);
	
	mutex_init(&src->mutex_for_destroy);
	
	src->allocator = allocator;
	src->kind = AUDIO_SOURCE_FILE_STREAM;
	
	src->format = format;
	
	File file = os_file_open(path, O_READ);
	if (file == OS_INVALID_FILE) return false;
	string header = talloc_string(4);
	memset(header.data, 0, 4);
	u64 read;
	bool ok = os_file_read(file, header.data, 4, &read);
	if (read != 4) return false;
	os_file_close(file);
	
	if (check_wav_header(header)) {
		src->decoder = AUDIO_DECODER_WAV;
		ok = wav_open_file(path, &src->wav, src->format.sample_rate, &src->number_of_frames);
		if (!ok) return false;
	} else if (check_ogg_header(header)) {
		src->decoder = AUDIO_DECODER_OGG;
		
		ok = os_read_entire_file(path, &src->ogg_raw, src->allocator);
		if (!ok) return false;
		
		third_party_allocator = src->allocator;
		int err = 0;
		src->ogg = stb_vorbis_open_memory(src->ogg_raw.data, src->ogg_raw.count, &err, 0);
		third_party_allocator = ZERO(Allocator);
		
		if (err != 0 || src->ogg == 0) return false;
		
		third_party_allocator = src->allocator;
		src->number_of_frames = stb_vorbis_stream_length_in_samples(src->ogg);
		third_party_allocator = ZERO(Allocator);
	} else {
		log_error("Error in audio_open_source_stream(): Unrecognized audio format in file '%s'. We currently support WAV and OGG (Vorbis).", path);
		return false;
	}
	
	return true;
}
bool
audio_open_source_stream(Audio_Source *src, string path, Allocator allocator) {
	mutex_acquire_or_wait(&audio_init_mutex);
	Audio_Format format = audio_output_format;
	mutex_release(&audio_init_mutex);
	return audio_open_source_stream_format(src, path, format, allocator);
}
bool
audio_open_source_load_format(Audio_Source *src, string path, Audio_Format format, 
							  Allocator allocator) {
	*src = ZERO(Audio_Source);
	
	mutex_init(&src->mutex_for_destroy);
	
	src->allocator = allocator;
	src->kind = AUDIO_SOURCE_MEMORY;
	src->format = format;
	
	File file = os_file_open(path, O_READ);
	if (file == OS_INVALID_FILE) return false;
	string header = talloc_string(4);
	memset(header.data, 0, 4);
	u64 read;
	bool ok = os_file_read(file, header.data, 4, &read);
	if (read != 4) return false;
	os_file_close(file);
	u64 frame_size 
		= src->format.channels*get_audio_bit_width_byte_size(src->format.bit_width);
	
	if (check_wav_header(header)) {
		src->decoder = AUDIO_DECODER_WAV;
		ok = wav_load_file(path, &src->pcm_frames, src->format, &src->number_of_frames, src->allocator);
		if (!ok) return false;
	} else if (check_ogg_header(header)) {
		src->decoder = AUDIO_DECODER_OGG;
		
		ok = os_read_entire_file(path, &src->ogg_raw, src->allocator);
		if (!ok) return false;
		
		third_party_allocator = src->allocator;
		int err = 0;
		src->ogg = stb_vorbis_open_memory(src->ogg_raw.data, src->ogg_raw.count, &err, 0);
		third_party_allocator = ZERO(Allocator);
		
		if (err != 0 || src->ogg == 0) return false;
		
		third_party_allocator = src->allocator;
		src->number_of_frames = stb_vorbis_stream_length_in_samples(src->ogg);
		third_party_allocator = ZERO(Allocator);
		
		src->pcm_frames = alloc(src->allocator, src->number_of_frames*frame_size);
		int retrieved = audio_source_get_frames(
			src, 
			0, 
			src->number_of_frames, 
			src->pcm_frames
		);
		
		
		third_party_allocator = src->allocator;
		stb_vorbis_close(src->ogg);
		third_party_allocator = ZERO(Allocator);
		
		if (retrieved != src->number_of_frames) {
			dealloc(src->allocator, src->pcm_frames);
			return false;
		}
	} else {
		log_error("Error in audio_open_source_load(): Unrecognized audio format in file '%s'. We currently support WAV and OGG (Vorbis).", path);
		return false;
	}
	
	return true;
}
bool
audio_open_source_load(Audio_Source *src, string path, Allocator allocator) {
	mutex_acquire_or_wait(&audio_init_mutex);
	Audio_Format format = audio_output_format;
	mutex_release(&audio_init_mutex);
	return audio_open_source_load_format(src, path, format, allocator);
}

void 
audio_source_destroy(Audio_Source *src) {

	mutex_acquire_or_wait(&src->mutex_for_destroy);

	switch (src->kind) {
		case AUDIO_SOURCE_FILE_STREAM: {
			third_party_allocator = src->allocator;
			switch (src->decoder) {
				case AUDIO_DECODER_WAV: {
					wav_close(&src->wav);
					break;
				}
				case AUDIO_DECODER_OGG: {
					stb_vorbis_close(src->ogg);
					dealloc_string(src->allocator, src->ogg_raw);
					break;
				}
			}
			third_party_allocator = ZERO(Allocator);
			break;
		}
		case AUDIO_SOURCE_MEMORY: {
			dealloc(src->allocator, src->pcm_frames);
			break;
		}
	}
	
	mutex_release(&src->mutex_for_destroy);
}

int
audio_source_get_frames(Audio_Source *src, u64 first_frame_index, 
					    u64 number_of_frames, void *output_buffer) {
	int retrieved = 0;
	switch (src->decoder) {
	case AUDIO_DECODER_WAV: {
		bool seek_ok = wav_set_frame_pos(&src->wav, src->format.sample_rate, first_frame_index);
		assert(seek_ok);
		
		retrieved = wav_read_frames(
			&src->wav, 
			src->format,
			output_buffer,
			number_of_frames
		); 
		
	} break; // case AUDIO_DECODER_WAV:
	case AUDIO_DECODER_OGG:  {
		f32 ratio = (f32)src->ogg->sample_rate/(f32)src->format.sample_rate;
		
		third_party_allocator = src->allocator;
		bool seek_ok = stb_vorbis_seek(src->ogg, round(first_frame_index*ratio));
		third_party_allocator = ZERO(Allocator);
		assert(seek_ok);
		
		// We need to convert sample rate & channels for vorbis
		
		u64 comp_size = get_audio_bit_width_byte_size(src->format.bit_width);
		u64 frame_size = src->format.channels*comp_size;
		
		u64 convert_frame_size = max(src->format.channels, src->ogg->channels)*comp_size;
		u64 required_size = convert_frame_size*number_of_frames;
		
		// #Cleanup #Memory refactor intermediate buffers
		thread_local local_persist void *convert_buffer = 0;
		thread_local local_persist u64  convert_buffer_size = 0;
		if (!convert_buffer || required_size > convert_buffer_size) {
			if (convert_buffer) dealloc(get_heap_allocator(), convert_buffer);
			
			u64 new_size = get_next_power_of_two(required_size);
			
			convert_buffer = alloc(get_heap_allocator(), new_size);
			memset(convert_buffer, 0, new_size);
			convert_buffer_size = new_size;
		}
		
		u64 number_of_frames_to_sample = number_of_frames;
		void *target_buffer = output_buffer;
		
		if (src->ogg->sample_rate != src->format.sample_rate 
		    || src->ogg->channels != src->format.channels) {
			number_of_frames_to_sample = (u64)round(ratio * (f32)number_of_frames);
			target_buffer = convert_buffer;
		}
		
		third_party_allocator = src->allocator;
		
		// Unfortunately, vorbis only converts o a different channel count if that differing
		// channel count is 2. So we might as well just deal with it ourselves.
		
		switch(src->format.bit_width) {
		case AUDIO_BITS_32: {
			retrieved = stb_vorbis_get_samples_float_interleaved(
				src->ogg, 
				src->ogg->channels, 
				(f32*)target_buffer, 
				number_of_frames_to_sample * src->ogg->channels
			);
			break;
		}
		case AUDIO_BITS_16: {
			retrieved = stb_vorbis_get_samples_short_interleaved(
				src->ogg, 
				src->ogg->channels, 
				(s16*)target_buffer, 
				number_of_frames_to_sample * src->ogg->channels
			); 
			break;
		}
		default: panic("Invalid bits value");
		}
		third_party_allocator = ZERO(Allocator);
		
		if (src->ogg->sample_rate != src->format.sample_rate 
		    || src->ogg->channels != src->format.channels) {
		    
			retrieved = convert_frames(
				output_buffer, 
				src->format,
				convert_buffer, 
				(Audio_Format){src->format.bit_width, src->ogg->channels, src->ogg->sample_rate},
				number_of_frames_to_sample
			);
		}
		
	} break; // case AUDIO_DECODER_OGG:
	default: panic("Invalid decoder value");
	}
	
	return retrieved;
}

u64 // New frame index 
audio_source_sample_next_frames(Audio_Source *src, u64 first_frame_index, u64 number_of_frames, 
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
	
		num_retrieved = audio_source_get_frames(
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
				num_retrieved = audio_source_get_frames(
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

    for (s64 dst_frame_index = dst_frame_count - 1; dst_frame_index >= 0; dst_frame_index--) {
        f32 src_frame_index_f = dst_frame_index * src_ratio;
        u64 src_frame_index_1 = (u64)src_frame_index_f;
        u64 src_frame_index_2 = src_frame_index_1 + 1;
        if (src_frame_index_2 >= src_frame_count) src_frame_index_2 = src_frame_count - 1;

        f32 lerp_factor = src_frame_index_f - (f32)src_frame_index_1;

        void *src_frame_1 = (u8*)src + src_frame_index_1 * src_frame_size;
        void *src_frame_2 = (u8*)src + src_frame_index_2 * src_frame_size;
        void *dst_frame = (u8*)dst + dst_frame_index * dst_frame_size;

        for (int c = 0; c < src_format.channels; c++) {
            void *src_comp_1 = (u8*)src_frame_1 + c * src_comp_size;
            void *src_comp_2 = (u8*)src_frame_2 + c * src_comp_size;
            void *dst_comp = (u8*)dst_frame + c * dst_comp_size;

            if (src_format.bit_width == AUDIO_BITS_32) {
                float32 sample_1 = *((f32*)src_comp_1);
                float32 sample_2 = *((f32*)src_comp_2);
                f32 s = sample_1 + lerp_factor * (sample_2 - sample_1);
                memcpy(dst_comp, &s, sizeof(f32));
            } else if (src_format.bit_width == AUDIO_BITS_16) {
                s16 sample_1 = *((s16*)src_comp_1);
                s16 sample_2 = *((s16*)src_comp_2);
                s16 s = (s16)(sample_1 + lerp_factor * (sample_2 - sample_1));
                memcpy(dst_comp, &s, sizeof(s16));
            } else {
                panic("Unhandled bit width");
            }
        }
    }

    // Correct padding on downsampling since we downsample backwards
    if (dst_format.sample_rate < src_format.sample_rate) {
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
int // Returns outputted number of frames
convert_frames(void *dst, Audio_Format dst_format, 
               void *src, Audio_Format src_format, u64 src_frame_count) {

	u64 dst_comp_size = get_audio_bit_width_byte_size(dst_format.bit_width);
    u64 dst_frame_size = dst_comp_size * dst_format.channels;
	u64 src_comp_size = get_audio_bit_width_byte_size(src_format.bit_width);
    u64 src_frame_size = src_comp_size * src_format.channels;
	   
	u64 output_frame_count = src_frame_count;
	
    if (dst_format.sample_rate != src_format.sample_rate) {
    	f32 ratio = (f32)src_format.sample_rate/(f32)dst_format.sample_rate;
    	output_frame_count = (u64)round((f32)src_frame_count/ratio);
    }

	if (bytes_match(&dst_format, &src_format, sizeof(Audio_Format))) {
		memcpy(dst, src, src_frame_count*src_frame_size);
		return src_frame_count;
	}
	
	bool need_sample_conversion 
		= dst_format.channels != src_format.channels 
	   || dst_format.bit_width != src_format.bit_width;
	// #Speed #Simd
	if (need_sample_conversion) {
		for (u64 src_frame_index = 0; src_frame_index < src_frame_count; src_frame_index++) {
	        void *src_frame = ((u8*)src) + src_frame_index*src_frame_size;
	        void *dst_frame = ((u8*)dst) + src_frame_index*dst_frame_size;
	        
	    	union {
	    		s16 s16_sample;
	    		f32 f32_sample;
	    		u8 data[4];
	    	} avg;
	    	
	    	// We only need the average if downscaling or src format is more than 1 when
	    	// upscaling. If source format is mono, the upscaling is just copying
	    	// that one channel to all the channels in dst.
	    	
	        if (src_format.channels != dst_format.channels &&
	        	 src_format.channels > dst_format.channels ||
	        	 src_format.channels > 1) {
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
					avg.s16_sample = (s16)round(sum/(f32)src_format.channels) * 32768.0;
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
				
	        } else if (src_format.channels == 1) {
	        	for (int c = 0; c < dst_format.channels; c++) {
					void *dst_comp = (u8*)dst_frame + c * dst_comp_size;
					convert_one_component(dst_comp,  dst_format.bit_width,
									      src_frame, src_format.bit_width);
				}
	        } else if (dst_format.channels > src_format.channels) {
	        	
	        	// Here, we are upscaling to a higher channel count from a src channel count
	        	// more than 1.
	        	// I'm not sure what the best way to do this is, but for now I will try to
	        	// just get the average in src and set that to the extra channels in dst.
	        	
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
    		dst_format,
			need_sample_conversion ? dst : src, 
			need_sample_conversion ? 
				(Audio_Format){dst_format.bit_width, dst_format.channels, src_format.sample_rate}
				: src_format,
			src_frame_count
		);

    }
    
    return output_frame_count;
}



#define AUDIO_STATE_FADE_TIME_MS 40

typedef enum Audio_Player_State {
	AUDIO_PLAYER_STATE_PAUSED,
	AUDIO_PLAYER_STATE_PLAYING
} Audio_Player_State;
typedef struct Audio_Player {
	// You shouldn't set these directly.
	// Configure players with the player_xxxxx procedures
	Audio_Source source;
	bool has_source;
	bool allocated;
	bool marked_for_release; // We release on audio thread
	Audio_Player_State state;
	u64 frame_index;
	bool looping;
	u64 fade_frames;
	u64 fade_frames_total;
	bool release_when_done;
	// I think we only need to sync when audio thread samples the source, which should be
	// very quick and low contention, hence a spinlock.
	Spinlock sample_lock; 
	
	// These can be set safely
	Vector3 position; // ndc space -1 to 1
	bool disable_spacialization;
	float32 volume;
	
} Audio_Player;
#define AUDIO_PLAYERS_PER_BLOCK 128
typedef struct Audio_Player_Block {
	Audio_Player players[AUDIO_PLAYERS_PER_BLOCK]; // Players need to be persistent in memory
	
	struct Audio_Player_Block *next;
} Audio_Player_Block;

// #Global
ogb_instance Audio_Player_Block audio_player_block;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Audio_Player_Block audio_player_block = {0};
#endif

Audio_Player *
audio_player_get_one() {

	Audio_Player_Block *block = &audio_player_block;
	Audio_Player_Block *last = 0;
	
	while (block) {
		
		for (u64 i = 0; i < AUDIO_PLAYERS_PER_BLOCK; i++) {
			if (!block->players[i].allocated) {
			
				memset(&block->players[i], 0, sizeof(block->players[i]));
				block->players[i].allocated = true;
				block->players[i].volume = 1.0;
				
				return &block->players[i];
			}
		}
		
		last = block;
		block = block->next;
	}
	
	// No free player found, make another block
	// #Volatile can't assign to last->next before this is zero initialized
	Audio_Player_Block *new_block = alloc(get_heap_allocator(), sizeof(Audio_Player_Block));
	
#if !DO_ZERO_INITIALIATION
	memset(new_block, 0, sizeof(*new_block));
#endif

	last->next = new_block;

	new_block->players[0].allocated = true;
	new_block->players[0].volume = 1.0;
	return &new_block->players[0];
}

void 
audio_player_release(Audio_Player *p) {
	p->marked_for_release = true;
}
void
audio_player_set_state(Audio_Player *p, Audio_Player_State state) {

	if (p->state == state) return;

	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	p->state = state;
	
	float64 full_duration 
		= (float64)p->source.number_of_frames/(float64)p->source.format.sample_rate;
	float64 progression = (float64)p->frame_index / (float64)p->source.number_of_frames;
	float64 remaining = (1.0-progression)*full_duration;
	
	float64 fade_seconds = min(AUDIO_STATE_FADE_TIME_MS/1000.0, remaining);
	
	float64 fade_factor = fade_seconds/full_duration;
	
	p->fade_frames = (u64)round(fade_factor*(float64)p->source.number_of_frames);
	p->fade_frames_total = p->fade_frames;
	
	spinlock_release(&p->sample_lock);
}
void
audio_player_set_time_stamp(Audio_Player *p, float64 time_in_seconds) {
	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	
	float64 full_duration 
		= (float64)p->source.number_of_frames/(float64)p->source.format.sample_rate;
	time_in_seconds = clamp(time_in_seconds, 0, full_duration);
	float64 progression = time_in_seconds/full_duration;
	
	p->frame_index = (u64)round((float64)p->source.number_of_frames*progression);
	
	spinlock_release(&p->sample_lock);
}
void // 0 - 1
audio_player_set_progression_factor(Audio_Player *p, float64 factor) {
	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	
	p->frame_index = (u64)round((float64)p->source.number_of_frames*factor);
	
	spinlock_release(&p->sample_lock);
}
float64 // seconds
audio_player_get_time_stamp(Audio_Player *p) {
	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	
	float64 full_duration 
		= (float64)p->source.number_of_frames/(float64)p->source.format.sample_rate;
	float64 progression = (float64)p->frame_index / (float64)p->source.number_of_frames;
	
	spinlock_release(&p->sample_lock);
	
	return progression*full_duration;
}
float64
audio_player_get_current_progression_factor(Audio_Player *p) {
	if (!p->has_source) return 0;
	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	
	float64 progression = (float64)p->frame_index / (float64)p->source.number_of_frames;
	
	spinlock_release(&p->sample_lock);
	
	return progression;
}
void 
audio_player_set_source(Audio_Player *p, Audio_Source src, bool retain_progression_factor) {

	float64 last_progression = audio_player_get_current_progression_factor(p);
	
	spinlock_acquire_or_wait(&p->sample_lock);

	p->source = src;
	p->has_source = true;
	
	if (retain_progression_factor) {
		p->frame_index = (u64)round((float64)p->source.number_of_frames*last_progression);
	} else {
		p->frame_index = 0;
	}
	
	spinlock_release(&p->sample_lock);
}
void 
audio_player_clear_source(Audio_Player *p) {
	spinlock_acquire_or_wait(&p->sample_lock);
	assert(p->frame_index <= p->source.number_of_frames);
	
	p->has_source = false;
	p->state = AUDIO_PLAYER_STATE_PAUSED;
	p->source = ZERO(Audio_Source);
	
	spinlock_release(&p->sample_lock);
}
void
audio_player_set_looping(Audio_Player *p, bool looping) {
	spinlock_acquire_or_wait(&p->sample_lock);
	
	if (p->has_source && looping && !p->looping && p->frame_index == p->source.number_of_frames) {
		p->frame_index = 0;
	}
	
	p->looping = looping;
	
	spinlock_release(&p->sample_lock);
}

// #Global
ogb_instance Hash_Table just_audio_clips;
ogb_instance bool just_audio_clips_initted;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Hash_Table just_audio_clips;
bool just_audio_clips_initted = false;
#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE

void
play_one_audio_clip_source_at_position(Audio_Source source, Vector3 pos) {
	Audio_Player *p = audio_player_get_one();
	audio_player_set_source(p, source, false);
	audio_player_set_state(p, AUDIO_PLAYER_STATE_PLAYING);
	p->position = pos;
	p->release_when_done = true;
}

void inline 
play_one_audio_clip_source(Audio_Source source) {
	play_one_audio_clip_source_at_position(source, v3(0, 0, 0));
}
void
play_one_audio_clip_at_position(string path, Vector3 pos) {
	if (!just_audio_clips_initted) {
		just_audio_clips_initted = true;
		just_audio_clips = make_hash_table(string, Audio_Source, get_heap_allocator());
	}
	
	Audio_Source *src_ptr = hash_table_find(&just_audio_clips, path);
	if (src_ptr) {
		play_one_audio_clip_source_at_position(*src_ptr, pos);
	} else {
		Audio_Source new_src;
		bool ok = audio_open_source_load(&new_src, path, get_heap_allocator());
		if (!ok) {
			log_error("Could not load audio to play from %s", path);
			return;
		}
		hash_table_add(&just_audio_clips, path, new_src);
		play_one_audio_clip_source_at_position(new_src, pos);
	}
	
}
void inline
play_one_audio_clip(string path) {
	play_one_audio_clip_at_position(path, v3(0, 0, 0));
}

void
audio_apply_fade_in(void *frames, u64 number_of_frames, Audio_Format format, 
					float64 fade_from, float64 fade_to) {
	u64 comp_size  = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;
    
    for (u64 f = 0; f < number_of_frames; f++) {
    	f32 frame_t = (f32)f/(f32)number_of_frames;
    	f32 log_scale = log10(1.0 + 9.0 * frame_t) / log10(10.0);
    	for (u64 c = 0; c < format.channels; c++) {
    		void *p = ((u8*)frames)+frame_size*f+c*comp_size;
	    	switch (format.bit_width) {
	    		case AUDIO_BITS_32: {
	    			f32 s = (*(f32*)p);
	    			*(f32*)p = smerpf(s * fade_from, s * fade_to, log_scale);
	    			break;
	    		}
	    		case AUDIO_BITS_16: {
	    			s16 s = (*(s16*)p);
	    			*(s16*)p = smerpi((f32)s * fade_from, (f32)s * fade_to, log_scale);
	    			break;
	    		}
	    	}
	    }
    }
}
void
audio_apply_fade_out(void *frames, u64 number_of_frames, Audio_Format format, 
					 float64 fade_from, float64 fade_to) {
	u64 comp_size  = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;
    
    for (u64 f = 0; f < number_of_frames; f++) {
    	f64 frame_t = 1.0-(f32)f/(f32)number_of_frames;
    	f64 log_scale = log10(1.0 + 9.0 * frame_t) / log10(10.0);
    	for (u64 c = 0; c < format.channels; c++) {
    		void *p = ((u8*)frames)+frame_size*f+c*comp_size;
	    	switch (format.bit_width) {
	    		case AUDIO_BITS_32: {
	    			f32 s = (*(f32*)p);
	    			*(f32*)p = smerpf(s * fade_from, s * fade_to, log_scale);
	    			break;
	    		}
	    		case AUDIO_BITS_16: {
	    			s16 s = (*(s16*)p);
	    			*(s16*)p = smerpi((f64)s * fade_from, (f64)s * fade_to, log_scale);
	    			break;
	    		}
	    	}
	    }
    }
}

void apply_audio_spacialization_mono(void* frames, Audio_Format format, u64 number_of_frames, Vector3 pos) {
	// No idea if this actually gives the perception of audio being positioned.
	// I also don't have a mono audio device to test it.
    float* audio_data = (float*)frames;

    float32 distance = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    float32 attenuation = 1.0f / (1.0f + distance); 

    float32 alpha = 0.1f;
    float32 prev_sample = 0.0f;
    
    u64 comp_size  = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;

    for (u64 i = 0; i < number_of_frames; ++i) {
        float32 sample = audio_data[i];
        convert_one_component(
        	&sample, 
        	AUDIO_BITS_32, 
        	(u8*)frames+i*frame_size,
        	format.bit_width
    	);

        sample *= attenuation;

        sample = alpha * sample + (1.0f - alpha) * prev_sample;
        prev_sample = sample;

        convert_one_component(
	    	(u8*)frames+i*frame_size,
	    	format.bit_width,
	    	&sample, 
	    	AUDIO_BITS_32
		);
    }
}
void apply_audio_spacialization(void* frames, Audio_Format format, u64 number_of_frames, Vector3 pos) {

	if (format.channels == 1) {
		apply_audio_spacialization_mono(frames, format, number_of_frames, pos);
	}

    float32 distance = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    float32 attenuation = 1.0f / (1.0f + distance);

    float32 left_right_pan = (pos.x + 1.0f) * 0.5f;
    float32 up_down_pan = (pos.y + 1.0f) * 0.5f;   
    float32 front_back_pan = (pos.z + 1.0f) * 0.5f;

	u64 comp_size  = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;
	
	float32 high_pass_coeff = 0.8f + 0.2f * up_down_pan;
    float32 low_pass_coeff = 1.0f - high_pass_coeff;
	
    // Apply gains to each frame
    for (u64 i = 0; i < number_of_frames; ++i) {
        for (u64 c = 0; c < format.channels; ++c) {
        	// Convert whatever to float32 -1 to 1
        	float32 sample;
            convert_one_component(
            	&sample, 
            	AUDIO_BITS_32, 
            	(u8*)frames+i*frame_size+c*comp_size, 
            	format.bit_width
        	);
            float32 gain = 1.0f / format.channels;

            if (format.channels == 2) {
            
            	// time delay and phase shift for vertical position
			    float32 phase_shift = (up_down_pan - 0.5f) * 0.5f; // 0.5 radians phase shift range
            
                // Stereo
                if (c == 0) {
                    gain = (1.0f - left_right_pan) * attenuation;
                    sample = sample * cos(phase_shift) - sample * sin(phase_shift);
                } else if (c == 1) {
                    gain = left_right_pan * attenuation;
                    sample = sample * cos(phase_shift) + sample * sin(phase_shift);
                }
            } else if (format.channels == 4) {
                // Quadraphonic sound (left-right, front-back)
                if (c == 0) {
                    gain = (1.0f - left_right_pan) * (1.0f - front_back_pan) * attenuation;
                } else if (c == 1) {
                    gain = left_right_pan * (1.0f - front_back_pan) * attenuation;
                } else if (c == 2) {
                    gain = (1.0f - left_right_pan) * front_back_pan * attenuation;
                } else if (c == 3) {
                    gain = left_right_pan * front_back_pan * attenuation;
                }
            } else if (format.channels == 6) {
                // 5.1 surround sound (left, right, center, LFE, rear left, rear right)
                if (c == 0) {
                    gain = (1.0f - left_right_pan) * attenuation;
                } else if (c == 1) {
                    gain = left_right_pan * attenuation;
                } else if (c == 2) {
                    gain = (1.0f - front_back_pan) * attenuation;
                } else if (c == 3) {
                    gain = 0.5f * attenuation; // LFE (subwoofer) channel
                } else if (c == 4) {
                    gain = (1.0f - left_right_pan) * front_back_pan * attenuation;
                } else if (c == 5) {
                    gain = left_right_pan * front_back_pan * attenuation;
                }
            } else {
            	// No idea what device this is, just distribute equally
                gain = attenuation / format.channels;
            }

			sample *= gain;
			// Convert back to whatever
			convert_one_component(
            	(u8*)frames+i*frame_size+c*comp_size, 
            	format.bit_width,
            	&sample, 
            	AUDIO_BITS_32
        	);
        }
    }
}

void apply_audio_volume(void* frames, Audio_Format format, u64 number_of_frames, float32 vol) {
	
	// #Speed
	// This is lazy, also it can be combined with other passes.

	u64 comp_size  = get_audio_bit_width_byte_size(format.bit_width);
    u64 frame_size = comp_size * format.channels;
	if (vol <= 0.0) {
		memset(frames, 0, frame_size*number_of_frames);
	}
	
	for (u64 i = 0; i < number_of_frames; ++i) {
        for (u64 c = 0; c < format.channels; ++c) {
        	float32 sample;
            convert_one_component(
            	&sample, 
            	AUDIO_BITS_32, 
            	(u8*)frames+i*frame_size+c*comp_size, 
            	format.bit_width
        	);
        	
        	sample *= vol;
        	
			convert_one_component(
            	(u8*)frames+i*frame_size+c*comp_size, 
            	format.bit_width,
            	&sample, 
            	AUDIO_BITS_32
        	);
        }
    }
}

// This is supposed to be called by OS layer audio thread whenever it wants more audio samples
void 
do_program_audio_sample(u64 number_of_output_frames, Audio_Format out_format, 
							 void *output) {
							 
	reset_temporary_storage();
							 
	u64 out_comp_size  = get_audio_bit_width_byte_size(out_format.bit_width);
    u64 out_frame_size = out_comp_size * out_format.channels;
    u64 output_size    = number_of_output_frames * out_frame_size;
    
	memset(output, 0, output_size);
	
	Audio_Player_Block *block = &audio_player_block;
	
	// #Cleanup #Memory refactor intermediate buffers
	thread_local local_persist void *mix_buffer = 0;
	thread_local local_persist u64 mix_buffer_size;
	thread_local local_persist void *convert_buffer = 0;
	thread_local local_persist u64 convert_buffer_size;
	
	memset(mix_buffer, 0, mix_buffer_size);
	
	
	while (block) {
		
		for (u64 i = 0; i < AUDIO_PLAYERS_PER_BLOCK; i++) {
			Audio_Player *p = &block->players[i];
			if (p->release_when_done && (p->frame_index >= p->source.number_of_frames
										  || !p->has_source)) {
				p->allocated = false;
			}
			if (!p->allocated) {
				continue;
			}
			
			if (p->marked_for_release) {
				p->marked_for_release = false;
				p->allocated = false;
				continue;
			}
			
			if (p->state != AUDIO_PLAYER_STATE_PLAYING) {
				if (p->fade_frames == 0) continue;
			}
			
			spinlock_acquire_or_wait(&p->sample_lock);
			
			Audio_Source src = p->source;
			
			mutex_acquire_or_wait(&src.mutex_for_destroy);
			
			bool need_convert = !bytes_match(
				&out_format, 
				&src.format, 
				sizeof(Audio_Format)
			);
			
			u64 in_comp_size 
				= get_audio_bit_width_byte_size(src.format.bit_width);
			
			u64 in_frame_size = in_comp_size * src.format.channels;
			u64 input_size = number_of_output_frames * in_frame_size;
			
			u64 biggest_size = max(input_size, output_size);
	
			if (!mix_buffer || mix_buffer_size < biggest_size) {
				u64 new_size = get_next_power_of_two(biggest_size);
				if (mix_buffer) dealloc(get_heap_allocator(), mix_buffer);
				mix_buffer = alloc(get_heap_allocator(), new_size);
				mix_buffer_size = new_size;
				memset(mix_buffer, 0, new_size);
			}
			
			void *target_buffer = mix_buffer;
			u64 number_of_sample_frames = number_of_output_frames;
			
			if (need_convert) {
				if (src.format.sample_rate != out_format.sample_rate) {
					f32 src_ratio 
						= (f32)src.format.sample_rate 
						  / (f32)out_format.sample_rate;
						
					number_of_sample_frames = round(number_of_output_frames * src_ratio);
					input_size = number_of_sample_frames * in_frame_size;
				}
				
				u64 biggest_size = max(input_size, output_size);
				
				if (!convert_buffer || convert_buffer_size < biggest_size) {
					u64 new_size = get_next_power_of_two(biggest_size);
					if (convert_buffer) dealloc(get_heap_allocator(), convert_buffer);
					convert_buffer = alloc(get_heap_allocator(), new_size);
					convert_buffer_size = new_size;
					memset(convert_buffer, 0, new_size);
				}
				target_buffer = convert_buffer;
				
			}
	
			p->frame_index = audio_source_sample_next_frames(
				&src,
				p->frame_index, 
				number_of_sample_frames,
				target_buffer,
				p->looping
			);
			
			if (p->fade_frames > 0) {
				u64 frames_to_fade = min(p->fade_frames, number_of_sample_frames);
				
				u64 frames_faded_so_far = (p->fade_frames_total-p->fade_frames);
				
				switch (p->state) {
					case AUDIO_PLAYER_STATE_PLAYING: {
						// We need to fade in
						float64 fade_from 
							= (f64)frames_faded_so_far / (f64)p->fade_frames_total;
							
						float64 fade_to 
							= (f64)(frames_faded_so_far + frames_to_fade) / (f64)p->fade_frames_total;
						audio_apply_fade_in(
							target_buffer, 
							frames_to_fade, 
							p->source.format, 
							fade_from,
							fade_to
						);
						break;
					}
					case AUDIO_PLAYER_STATE_PAUSED: {
						// We need to fade out
						// #Bug #Incomplete
						// I can't get this to fade out without noise.
						// I tried dithering but that didn't help.
						float64 fade_from 
							= 1.0 - (f64)frames_faded_so_far / (f64)p->fade_frames_total;
							
						float64 fade_to 
							= 1.0 - (f64)(frames_faded_so_far + frames_to_fade) / (f64)p->fade_frames_total;
						audio_apply_fade_out(
							target_buffer, 
							frames_to_fade, 
							p->source.format, 
							fade_from,
							fade_to
						);
						break;
					}
				}
				
				p->fade_frames -= frames_to_fade;
				
				if (frames_to_fade < number_of_sample_frames) {
					memset(
						(u8*)target_buffer+frames_to_fade, 
						0, 
						number_of_sample_frames-frames_to_fade
					);
				}
			}
			
			spinlock_release(&p->sample_lock);
						
			if (need_convert) {
				int converted = convert_frames(
					mix_buffer, 
					out_format, 
					convert_buffer, 
					src.format,
					number_of_sample_frames
				);
				assert(converted == number_of_output_frames);
			}

			if (!p->disable_spacialization) {
				apply_audio_spacialization(mix_buffer, out_format, number_of_output_frames, p->position);
			}
			if (p->volume != 0.0) {
				apply_audio_volume(mix_buffer, out_format, number_of_output_frames, p->volume);
			}
			
			mix_frames(output, mix_buffer, number_of_output_frames, out_format);
			
			mutex_release(&src.mutex_for_destroy);
		}
		
		block = block->next;
	}
}