#include "AudioRenderer.h"
#include "AudioDevice.h"

AudioRenderer2::AudioRenderer2(AudioDevice &device): device(&device){
	this->device->set_renderer(*this);
}

AudioRenderer2::~AudioRenderer2(){
	this->device->clear_renderer();
}

void AudioRenderer2::write_data_to_device(Uint8 *stream, int len){
	{
		LOCK_MUTEX(this->mutex);
		auto frame = this->get_current_frame();
		if (frame){
			if (frame->frame_no < this->expected_frame){
				this->return_used_frame(frame);
			}else{
				this->expected_frame = frame->frame_no + 1;
				auto n = std::min<size_t>(len, sizeof(frame->buffer));
				memcpy(stream, frame->buffer, n);
				if (len - n)
					memset(stream + n, 0, len - n);
				this->return_used_frame(frame);
				return;
			}
		}
	}
	memset(stream, 0, len);
}
