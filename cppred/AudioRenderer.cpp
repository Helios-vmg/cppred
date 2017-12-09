#include "AudioRenderer.h"
#include "AudioDevice.h"

AudioRenderer::AudioRenderer(AudioDevice &device): device(&device){
	this->device->set_renderer(*this);
}

AudioRenderer::~AudioRenderer(){
	this->device->clear_renderer();
}

void AudioRenderer::write_data_to_device(Uint8 *stream, int len, StereoSampleFinal *spillover_buffer, size_t &spillover_buffer_size){
	{
		LOCK_MUTEX(this->mutex);
		while (len){
			auto frame = this->get_current_frame();
			if (!frame)
				break;
			if (frame->frame_no < this->expected_frame){
				this->return_used_frame(frame);
				continue;
			}

			this->expected_frame = frame->frame_no + 1;
			const auto size2 = (int)sizeof(frame->buffer);

			if (len >= size2){
				memcpy(stream, frame->buffer, size2);
				stream += size2;
				len -= size2;
			}else{
				memcpy(stream, frame->buffer, len);

				const auto s = sizeof(StereoSampleFinal);
				const auto m = (len + (s - 1)) / s * s;
				assert(m <= size2);
				if (m < size2){
					memcpy(spillover_buffer, (byte_t *)frame->buffer + m, size2 - m);
					spillover_buffer_size = (size2 - m) / s;
				}

				stream += len;
				len = 0;
			}
			this->return_used_frame(frame);
		}
	}
	if (len)
		memset(stream, 0, len);
}
