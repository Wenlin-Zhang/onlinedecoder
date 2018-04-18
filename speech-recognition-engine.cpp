#include "speech-recognition-engine.h"
#include "onlinedecoder/audio-buffer-source.h"
#include "onlinedecoder/online-decoder.h"
#include <map>
#include <string>
#include <sstream>

using namespace kaldi;

static std::map<int, OnlineDecoder*> g_engine_map;

static std::string error_message;

static int GenerateID()
{
	int new_id = rand();
	while (g_engine_map.find(new_id) != g_engine_map.end())
		new_id = rand();
	return new_id;
}

static OnlineDecoder* GetEngine(int id)
{
	std::map<int, OnlineDecoder*>::iterator it = g_engine_map.find(id);
	if (it == g_engine_map.end())
		return NULL;
	else
		return it->second;
}

int CreateRecognizer(const char* conf_rxfilename)
{
	OnlineDecoder* pDecoder = new OnlineDecoder(conf_rxfilename);
	int id = GenerateID();
	g_engine_map[id] = pDecoder;
	return id;
}

ReturnStatus StartRecognizer(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->StartDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus SuspendRecognizer(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->SuspendDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus ResumeRecognizer(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->ResumeDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus StopRecognizer(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->StopDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus WaitForRecogStop(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->WaitForEndOfDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus FreeRecognizer(int engineID)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		delete pDecoder;
		g_engine_map.erase(engineID);
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
}

ReturnStatus AddBuffer(int engineID, const char* spkId, const short* pData, int size)
{
  KALDI_LOG << "21";
	OnlineDecoder* pDecoder = GetEngine(engineID);
	KALDI_LOG << "22";
	if (pDecoder != NULL)
	{
	  KALDI_LOG << "23";
		AudioBuffer* pBuffer;
		KALDI_LOG << "23";
		KALDI_LOG << "Before AudioBuffer create.";
		AudioBuffer ab;
		KALDI_LOG << "AudioBuffer create.";
		pBuffer = new AudioBuffer();
		KALDI_LOG << "24";
		pBuffer->spkr_ = spkId;
		pBuffer->size_ = size;
		KALDI_LOG << "25";
		pBuffer->pData_ = new SampleType[size];
		KALDI_LOG << "26";
		for (int i = 0; i < size; ++i)
			pBuffer->pData_[i] = pData[i];
	  
		pDecoder->ReceiveData(pBuffer);
		
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
	
}

ReturnStatus AddCallback(int engineID, DecoderSignal signal, DecoderSignalCallback callback)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		pDecoder->AddCallBack(signal, callback);
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
	}
	
}
