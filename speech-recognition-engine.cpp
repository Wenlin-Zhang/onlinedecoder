#include "speech-recognition-engine.h"
#include "onlinedecoder/audio-buffer-source.h"
#include "onlinedecoder/online-decoder.h"
#include <map>
#include <string>
#include <sstream>

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

int CreateRecognizer(const std::string& conf_rxfilename)
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
		pRecogObj->StartDecoding();
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
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
	}
}

ReturnStatus AddBuffer(int engineID, string spkId, const short* pData, int size)
{
	OnlineDecoder* pDecoder = GetEngine(engineID);
	if (pDecoder != NULL)
	{
		AudioBufferSource::AudioBuffer* pBuffer = new AudioBufferSource::AudioBuffer();
		pBuffer->spkr_ = spkId;
		pBuffer->size_ = size;
		pBuffer->pData_ = new AudioBufferSource::SampleType[size];
		for (int i = 0; i < size; ++i)
			pBuffer->pData_[i] = pData[i];
		pDecoder->AddBuffer(pBuffer);
		return SUCCEED;
	}
	else
	{
		std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
	}
	
}

void AddCallback(int engineID, DecoderSignal signal, DecoderSignalCallback callback)
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
	}
	
}
