#ifndef KALDI_ONLINE_DECODER_H_
#define KALDI_ONLINE_DECODER_H_

#include "speech-recognition-engine.h"
#include "online2/online-nnet3-decoding.h"
#include "online2/online-nnet2-feature-pipeline.h"
#include "online2/online-nnet2-decoding-threaded.h"
#include "online2/online-nnet2-decoding.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"
#include "util/kaldi-thread.h"
#include "nnet3/nnet-utils.h"
#include "lm/const-arpa-lm.h"
#include "lat/word-align-lattice.h"
#include "hmm/hmm-utils.h"
#include "lat/sausages.h"
#include "onlinedecoder/audio-buffer-source.h"

#include <mutex>
#include <condition_variable>

#define DEFAULT_MODEL           ""
#define DEFAULT_FST             ""
#define DEFAULT_WORD_SYMS       ""
#define DEFAULT_PHONE_SYMS      ""
#define DEFAULT_WORD_BOUNDARY_FILE ""
#define DEFAULT_LMWT_SCALE	1.0
#define DEFAULT_CHUNK_LENGTH_IN_SECS  0.05
#define DEFAULT_TRACEBACK_PERIOD_IN_SECS  0.5
#define DEFAULT_USE_THREADED_DECODER false
#define DEFAULT_NUM_NBEST 1
#define DEFAULT_NUM_PHONE_ALIGNMENT 1
#define DEFAULT_MIN_WORDS_FOR_IVECTOR 2

namespace kaldi {

/**
 * Some structs used for storing recognition results
 */
typedef struct _WordInHypothesis WordInHypothesis;
typedef struct _PhoneAlignmentInfo PhoneAlignmentInfo;
typedef struct _WordAlignmentInfo WordAlignmentInfo;
typedef struct _NBestResult NBestResult;
typedef struct _FullFinalResult FullFinalResult;

struct _WordInHypothesis {
  int32 word_id;
};

struct _WordAlignmentInfo {
  int32 word_id;
  int32 start_frame;
  int32 length_in_frames;
  double confidence;
};

struct _PhoneAlignmentInfo {
  int32 phone_id;
  int32 start_frame;
  int32 length_in_frames;
  double confidence;
};

struct _NBestResult {
  int32 num_frames;
  double likelihood;
  std::vector<WordInHypothesis> words;
  std::vector<PhoneAlignmentInfo> phone_alignment;
  std::vector<WordAlignmentInfo> word_alignment;
};

struct _FullFinalResult {
  std::string spkr;
  std::vector<NBestResult> nbest_results;
  std::string phone_alignment;
};

/// OnlineDecoderOptions contains basic options for online decoder.
struct OnlineDecoderOptions {

  bool silent_;
	bool do_endpointing_;
	bool inverse_scale_;
	bool do_phone_alignment_;
	bool use_threaded_decoder_;
	
	BaseFloat lmwt_scale_;
	BaseFloat chunk_length_in_secs_;
	BaseFloat traceback_period_in_secs_;

  int32 num_nbest_;
  int32 num_phone_alignment_;
	int32 min_words_for_ivector_;
  
	std::string model_rspecifier_;
	std::string fst_rspecifier_;
	std::string lm_fst_rspecifier_;
	std::string word_syms_filename_;
	std::string phone_syms_filename_;
	std::string word_boundary_info_filename_;
	std::string adaptation_state_str_;


  
  OnlineDecoderOptions() : silent_(false),
                 do_endpointing_(false),
                 inverse_scale_(false),
                 do_phone_alignment_(false),
                 use_threaded_decoder_(DEFAULT_USE_THREADED_DECODER),
                 lmwt_scale_(DEFAULT_LMWT_SCALE),
                 chunk_length_in_secs_(DEFAULT_CHUNK_LENGTH_IN_SECS),
                 traceback_period_in_secs_(DEFAULT_TRACEBACK_PERIOD_IN_SECS),
                 num_nbest_(DEFAULT_NUM_NBEST),
                 num_phone_alignment_(DEFAULT_NUM_PHONE_ALIGNMENT),
                 min_words_for_ivector_(DEFAULT_MIN_WORDS_FOR_IVECTOR),
                 model_rspecifier_(DEFAULT_MODEL),
                 fst_rspecifier_(DEFAULT_FST),
                 word_syms_filename_(DEFAULT_WORD_SYMS),
                 phone_syms_filename_(DEFAULT_PHONE_SYMS),
                 word_boundary_info_filename_(DEFAULT_WORD_BOUNDARY_FILE),
                 adaptation_state_str_("") {}
  
  void Register(OptionsItf *opts) {
    
    opts->Register("model", &model_rspecifier_, "Filename of the acoustic model.");  
              
    opts->Register("fst", &fst_rspecifier_, "Filename of the HCLG FST");
    
    opts->Register("word-syms", &word_syms_filename_, "Name of word symbols "
        "file (typically words.txt)");
        
    opts->Register("phone-syms", &phone_syms_filename_, "Name of phoneme symbols "
        "file (typically phones.txt)");
        
    opts->Register("do-phone-alignment", &do_phone_alignment_, "If true, output "
        "phoneme-level alignment, default false.");
        
    opts->Register("do-endpointing", &do_endpointing_, "If true, apply endpoint detection, "
        "and split the audio at endpoints, default false.");
        
    opts->Register("adaptation-state", &adaptation_state_str_, "Current adaptation state, "
        "in stringified form, set to empty string to reset");

    opts->Register("inverse-scale", &inverse_scale_, "If true, inverse the acoustic scaling "
        "of the output lattice, default false.");

    opts->Register("lmwt-scale", &lmwt_scale_, "LM scaling for the output lattice, "
        "usually in conjunction with inverse-scaling=true");
        
    opts->Register("chunk-length-in-secs", &chunk_length_in_secs_, 
        "Length of a audio chunk that is processed at a time."
        "Smaller values decrease latency, bigger values (e.g. 0.2) "
        "improve speed if multithreaded BLAS/MKL is used");
        
    opts->Register("traceback-period-in-secs", &traceback_period_in_secs_, 
        "Time period after which new interim recognition result is sent");
        
    opts->Register("lm-fst", &lm_fst_rspecifier_, "Language language model FST (G.fst), "
        "only needed when rescoring with the constant ARPA LM");
        
    opts->Register("word-boundary-file", &word_boundary_info_filename_, 
        "Word-boundary file. Setting this property triggers generating word "
        "alignments in full results");

    opts->Register("use-threaded-decoder", &use_threaded_decoder_, 
        "Use a decoder that does feature calculation and decoding in separate threads "
        "(NB! must be set before other properties)");
                
    opts->Register("num-nbest", &num_nbest_, "number of hypotheses in the full final results");
    
    opts->Register("num-phone-alignment", &num_phone_alignment_, "number of hypotheses "
        "where alignment should be done");
    
    opts->Register("min-words-for-ivector", &min_words_for_ivector_,
        "threshold for updating ivector (adaptation state). "
        "Minimal number of words in the first transcription for triggering "
        "update of the adaptation state");
  }
};

// Decoder class
class OnlineDecoder {
public:
	typedef kaldi::int32 int32;
    typedef kaldi::int64 int64;

	enum DecoderState {
		InitDecoding,
		OnDecoding,
		SuspendDecoding,
		StopDecoding,
		EndDecoding
	};
	
	explicit OnlineDecoder(const string& configFilePath);
	~OnlineDecoder();
	
	// TODO: load settings from config file
	//bool LoadConfig(const string& configFilePath);
	void LoadWordSyms();
	void LoadPhoneSyms();
	void LoadWordBoundaryInfo();
	void LoadAcousticModel();
	void LoadFst();
	void LoadLmFst();
	
	bool LoadModel();
	void Finalize();
	
	void ReceiveData(AudioBufferSource::AudioBuffer* pBuffer ) {audio_source_->ReceiveData(pBuffer);};

	// Add callback functions
	void AddCallBack(DecoderSignal signal, DecoderSignalCallback onSignal);
	
	void StartDecoding();

	void SuspendDecoding();

	void ResumeDecoding();

	void StopDecoding();

	void WaitForEndOfDecoding();

protected:

	void ChangeState(DecoderState newState);

	// Invoke callback functions
	void InvokeCallBack(DecoderSignal signal, const char* pszResults);

	// the decode loop
	void DecodeLoop();
	
	// Generate final results and emit signal FINAL_RESULT_SIGNAL and FULL_FINAL_RESULT_SIGNAL
	void GenerateFinalResult(CompactLattice &clat, int32 *num_words, string spkr);
	
	// Generate partial results and emit signal PARTIAL_RESULT_SIGNAL
	void GeneratePartialResult(const Lattice lat);
	
	// Decode for a segment/utterance
	void DecodeSegment(bool &more_data, int32 chunk_length, BaseFloat traceback_period_secs);
	
protected:
	std::vector<PhoneAlignmentInfo> GetPhoneAlignment(const std::vector<int32>& alignment, const CompactLattice &clat);
	std::vector<WordAlignmentInfo> GetWordAlignment(const Lattice &lat, const std::vector<BaseFloat> &confidences);
	void ScaleLattice(CompactLattice &clat);
	std::string Words2String(const std::vector<int32> &words);
	std::string WordsInHyp2String(const std::vector<WordInHypothesis> &words);
	std::vector<NBestResult> GetNbestResults(CompactLattice &clat);
	std::string FullFinalResult2Json(const FullFinalResult &full_final_result);
	
protected:
	/*bool silent_;
	bool do_endpointing_;
	bool inverse_scale_;
	bool use_threaded_decoder_;
	bool do_phone_alignment_;
	
	BaseFloat lmwt_scale_;
	BaseFloat chunk_length_in_secs_;
	BaseFloat traceback_period_in_secs_;

  int32 num_nbest_;
  int32 num_phone_alignment_;
	int32 min_words_for_ivector_;
	
	std::string model_rspecifier_;
	std::string fst_rspecifier_;
	std::string word_syms_filename_;
	std::string phone_syms_filename_;
	std::string word_boundary_info_filename_;*/

	OnlineDecoderOptions *opts_;
	
	OnlineEndpointConfig *endpoint_config_;
	OnlineNnet2FeaturePipelineConfig *feature_config_;
	//OnlineNnet2DecodingThreadedConfig *nnet2_decoding_threaded_config_;
	//OnlineNnet2DecodingConfig *nnet2_decoding_config_;
	
	// support for nnet3
	nnet3::NnetSimpleLoopedComputationOptions *nnet3_decodable_opts_;
	LatticeFasterDecoderConfig *decoder_opts_;  
	
	OnlineSilenceWeightingConfig *silence_weighting_config_;
  
	AudioBufferSource* audio_source_;
	
	OnlineNnet2FeaturePipelineInfo *feature_info_;
	TransitionModel *trans_model_;
	nnet3::AmNnetSimple *am_nnet3_;
	nnet3::DecodableNnetSimpleLoopedInfo *decodable_info_nnet3_;
	fst::Fst<fst::StdArc> *decode_fst_;
	
	fst::SymbolTable *word_syms_;
	fst::SymbolTable *phone_syms_;
	WordBoundaryInfo *word_boundary_info_;
	int32 sample_rate_;

	std::mutex state_mtx_;
	std::condition_variable state_cond_;
	DecoderState state_;
	std::thread* decode_thread_;

	OnlineIvectorExtractorAdaptationState *adaptation_state_;
	
	float segment_start_time_;
	float total_time_decoded_;
	
	// The following are needed for optional LM rescoring with a "big" LM
	fst::MapFst<fst::StdArc, LatticeArc, fst::StdToLatticeMapper<BaseFloat> > *lm_fst_;
	fst::TableComposeCache<fst::Fst<LatticeArc> > *lm_compose_cache_;
	
	// callback functions
	std::map< DecoderSignal, std::vector<DecoderSignalCallback> > onDecoderSignalCallbacks_;
};

}

#endif  // KALDI_ONLINE_DECODER_H_
