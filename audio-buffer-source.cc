#include "onlinedecoder/audio-buffer-source.h"

namespace kaldi {

// put a buffer in the queue
void AudioBufferSource::EnqueueBuffer(AudioBuffer* pBuffer)
{
  // lock the mutex to guard the buffer queue for writting
  KALDI_LOG << "12";
  std::lock_guard<std::mutex> mtx_locker(buffer_mtx_);
  KALDI_LOG << "13";
  data_buffer_queue_.push(pBuffer);
  KALDI_LOG << "14";
  buffer_cond_.notify_one();
  KALDI_LOG << "15";
}

// get g buffer from the queue, if the queue is empty and is not ended, wait until a buffer is available
AudioBuffer* AudioBufferSource::DequeueBuffer()
{
  // lock the mutex to guard the buffer queue for reading
  std::unique_lock<std::mutex> mtx_locker(buffer_mtx_);
  if (data_buffer_queue_.empty() && ended_ == true)
	  return NULL;
  // wait until there is a buffer available or the queue is ended
  buffer_cond_.wait(mtx_locker, [this] {return (!this->data_buffer_queue_.empty() || this->ended_); });
  if (data_buffer_queue_.empty() && ended_ == true)
	  return NULL;
  else
  {
	  AudioBuffer* pBuffer = data_buffer_queue_.front();
	  data_buffer_queue_.pop();
	  return pBuffer;
  }
}

AudioState AudioBufferSource::ReadData(Vector<BaseFloat>* data, std::string& spk){
  std::string current_spkr = "";
  if (cur_buffer_ != NULL)
  {
	  current_spkr = cur_buffer_->spkr_;
  }
  if (cur_buffer_ == NULL || pos_in_current_buf_ == cur_buffer_->size_) {
	  if (cur_buffer_ != NULL)
	  {
		  delete cur_buffer_->pData_;
		  delete cur_buffer_;
	  }
	cur_buffer_ = this->DequeueBuffer();
	if (cur_buffer_ == NULL)
	{
		// if the returned buffer is empty, which means the audio buffer is ended
		KALDI_ASSERT(ended_ == true);
		data->Resize(0);
		spk = "";
		return AudioState::AudioEnd;
	}
	else
	{
	  pos_in_current_buf_ = 0;
	  // if the spkr id of the new buffer and last buffer are different, 
	  // it means the end of last speaker is reached
	  if (current_spkr != "" && current_spkr != cur_buffer_->spkr_)
	  {
		  data->Resize(0);
		  spk = current_spkr;
		  return AudioState::SpkrEnd;
	  }
	}
  }

  // set the current spkr id
  if (current_spkr == "")
	  current_spkr = cur_buffer_->spkr_;

  // get the chunk_length of the required data
  int32 chunk_length = data->Dim();
  for(int32 i = 0; i < chunk_length; i++) {
    (*data)(i) = static_cast<BaseFloat>(cur_buffer_->pData_[pos_in_current_buf_]);
    pos_in_current_buf_++;
    if (pos_in_current_buf_ >= cur_buffer_->size_) {
	  delete cur_buffer_->pData_;
	  delete cur_buffer_;
      cur_buffer_ = NULL;
	  cur_buffer_ = this->DequeueBuffer();
	  if (cur_buffer_ == NULL)
	  {
		  KALDI_ASSERT(ended_ == true);
		  data->Resize(i, kCopyData);
		  spk = current_spkr;
		  return AudioState::AudioEnd;
	  }
	  else
	  {
		  pos_in_current_buf_ = 0;
		  if (current_spkr != cur_buffer_->spkr_)
		  {
			  data->Resize(i, kCopyData);
			  spk = current_spkr;
			  return AudioState::SpkrEnd;
		  }
	  }
    }
  }

  spk = current_spkr;
  return AudioState::SpkrContinue;
}

// External Interface: put the data buffer in the queue is it is not ENDED!
void AudioBufferSource::ReceiveData(AudioBuffer* pBuffer){
  KALDI_LOG << "11";
  if (ended_ == false)
	  this->EnqueueBuffer(pBuffer);
}

void AudioBufferSource::SetEnded(bool ended) {
	std::lock_guard<std::mutex> mtx_locker(buffer_mtx_);
	if (ended_ == false && ended == true)
	{
		// notify the buffer_cond_, let the blocked DequeueBuffer call to exit
		ended_ = ended;
		buffer_cond_.notify_one();
		return;
    }
	ended_ = ended;
}

// TODO: write some protection code to prevent possible crash 
//       when the buffer is not empty and there is still data reading in other thread
AudioBufferSource::~AudioBufferSource(){
  if (ended_ == false)
	  SetEnded(true);
  // if the queue is not empty, give 10 seconds to free
  // if after 10 seconds the queue is still nonempty, there might be an error occured
  if (!data_buffer_queue_.empty())
	  sleep(10);
  if (!cur_buffer_) {
	  delete cur_buffer_->pData_;
	  delete cur_buffer_;
      cur_buffer_ = NULL;
  }

  while(data_buffer_queue_.size() > 0)
  {
	  cur_buffer_ = data_buffer_queue_.back();
	  delete cur_buffer_->pData_;
	  delete cur_buffer_;
	  cur_buffer_ = NULL;
  }
}

}
