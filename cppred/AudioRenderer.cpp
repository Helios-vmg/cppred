#include "AudioRenderer.h"

void AudioRenderer::initialize_new_frame(){
	auto frame = this->publishing_frames.get_private_resource();
	frame->frame_no = this->frame_no++;
	auto &buffer = frame->buffer;
	memset(buffer, 0, sizeof(buffer));
}

void AudioRenderer::write_sample(StereoSampleFinal *&buffer, const StereoSampleFinal &sample){
	buffer[this->current_frame_position++] = sample;
	if (this->current_frame_position >= AudioFrame::length){
#ifdef OUTPUT_AUDIO_TO_FILE
		if (this->output_file)
			this->output_file->write((const char *)buffer, AudioFrame::length * sizeof(StereoSampleFinal));
		for (int i = 4; i--;){
			auto &buffer2 = this->output_buffers_by_channel[i]->buffer;
			if (this->output_files_by_channel[i])
				this->output_files_by_channel[i]->write((const char *)buffer2, sizeof(buffer2));
		}
#endif
		this->current_frame_position = 0;
		this->publishing_frames.publish();
		this->initialize_new_frame();
		buffer = this->publishing_frames.get_private_resource()->buffer;
	}
}

AudioFrame *AudioRenderer::get_current_frame(){
	return this->publishing_frames.get_public_resource();
}

void AudioRenderer::return_used_frame(AudioFrame *frame){
	this->publishing_frames.return_resource(frame);
}
