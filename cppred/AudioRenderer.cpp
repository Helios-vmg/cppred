#include "stdafx.h"
#include "AudioRenderer.h"
#include "AudioDevice.h"

AudioRenderer::AudioRenderer(AbstractAudioDevice &device): device(&device){
	this->device->set_renderer(*this);
}

AudioRenderer::~AudioRenderer(){
}

void AudioRenderer::write_data_to_device(Uint8 *stream, int len, StereoSampleFinal *spillover_buffer, size_t &spillover_buffer_size){
	{
		LOCK_MUTEX(this->mutex);
		while (len){
			auto frame = this->get_current_frame_with_object();
			if (!frame.first)
				break;
			if (frame.first->frame_no < this->expected_frame){
				this->return_used_frame(frame);
				continue;
			}

			this->expected_frame = frame.first->frame_no + 1;
			const auto size2 = (int)sizeof(frame.first->buffer);

			if (len >= size2){
				memcpy(stream, frame.first->buffer, size2);
				stream += size2;
				len -= size2;
			}else{
				memcpy(stream, frame.first->buffer, len);

				const auto s = sizeof(StereoSampleFinal);
				const auto m = (len + (s - 1)) / s * s;
				assert(m <= size2);
				if (m < size2){
					memcpy(spillover_buffer, (byte_t *)frame.first->buffer + m, size2 - m);
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

TwoWayMixer::TwoWayMixer(AudioDevice &device): AudioRenderer(device){}

TwoWayMixer::~TwoWayMixer(){
	this->device->clear_renderer();
}

void TwoWayMixer::set_renderers(std::unique_ptr<GbAudioRenderer> &&low_priority_renderer, std::unique_ptr<GbAudioRenderer> &&high_priority_renderer){
	this->low_priority_renderer = std::move(low_priority_renderer);
	this->high_priority_renderer = std::move(high_priority_renderer);
}

void TwoWayMixer::start(){
	this->low_priority_renderer->set_NR52(0xFF);
	this->low_priority_renderer->set_NR50(0x77);
	this->high_priority_renderer->set_NR52(0xFF);
	this->high_priority_renderer->set_NR50(0x77);
}

void TwoWayMixer::update(double now){
	this->low_priority_renderer->update(now);
	this->high_priority_renderer->update(now);
	LOCK_MUTEX(this->mutex);
	while (true){
		auto low_frame = this->low_priority_renderer->get_current_frame_with_object();
		auto high_frame = this->high_priority_renderer->get_current_frame_with_object();
		assert(!low_frame.first == !high_frame.first);
		if (!low_frame.first)
			break;
		assert(low_frame.first->frame_no == high_frame.first->frame_no);
		bool active = high_frame.first->active;
		if (active){
			int divisor = this->volume_divisor * 3;
			for (size_t i = 0; i < AudioFrame::length; i++){
				low_frame.first->buffer[i] = high_frame.first->buffer[i] / 2;
			}
		}else{
			int divisor = this->volume_divisor * 3;
			for (size_t i = 0; i < AudioFrame::length; i++){
				low_frame.first->buffer[i] /= divisor;
				low_frame.first->buffer[i] += high_frame.first->buffer[i] / 2;
			}
		}
		AudioRenderer::return_used_frame(high_frame);
		if (!this->queue.try_enqueue(low_frame))
			AudioRenderer::return_used_frame(low_frame);
	}
}

AudioRenderer::frame_t TwoWayMixer::get_current_frame_with_object(){
	frame_t ret(nullptr, nullptr);
	this->queue.try_dequeue(ret);
	return ret;
}

AudioFrame *TwoWayMixer::get_current_frame(){
	assert(false);
	return nullptr;
}

void TwoWayMixer::return_used_frame(AudioFrame *frame){
	assert(false);
}
