#ifndef KALDI_AUDIO_BUFFER_SOURCE_H_
#define KALDI_AUDIO_BUFFER_SOURCE_H_

#include <fstream>
#include <iostream>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include "matrix/kaldi-vector.h"

namespace kaldi {

// AudioBufferSource implementation using a queue of Gst Buffers
// Reference: gst_audio_source
class AudioBufferSource {
 public:
  typedef kaldi::int16 SampleType;  // hardcoded 16-bit audio

  // Audio State enum
  enum AudioState {
	  SpkrContinue,
	  SpkrEnd,
	  AudioEnd
  };

  // Buffer definition
  struct AudioBuffer {
	  std::string spkr_;
	  SampleType* pData_;
	  int size_;
  };

  AudioBufferSource(): ended_(false), cur_buffer_(NULL), pos_in_current_buf_(0) {}

  // read data from audiobuffer
  // return: 
  //    spkr_continue: readed data block is nonempty, and there may be more data for the same speaker
  //    spkr_end: this is the end of the current speaker audio
  //    audio_end: the audio buffer is end
  AudioState ReadData(Vector<BaseFloat>* data, string& spk);

  void ReceiveData(AudioBuffer* pBuffer);

  void SetEnded(bool ended);

  ~AudioBufferSource();

 private:

  // put a buffer in the queue
  void EnqueueBuffer(AudioBuffer* pBuffer);

  // get g buffer from the queue, if the queue is empty and is not ended, wait until a buffer is available
  AudioBuffer* DequeueBuffer();

  bool ended_;
  std::mutex buffer_mtx_;
  std::condition_variable buffer_cond_;
  AudioBuffer* cur_buffer_;
  kaldi::int32 pos_in_current_buf_;
  std::queue<AudioBuffer* > data_buffer_queue_;
  KALDI_DISALLOW_COPY_AND_ASSIGN(AudioBufferSource);
};

}
#endif  // KALDI_AUDIO_BUFFER_SOURCE_H_
