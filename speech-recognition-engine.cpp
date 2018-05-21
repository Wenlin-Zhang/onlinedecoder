// 张; 杨
#include "speech-recognition-engine.h"
#include "onlinedecoder/audio-buffer-source.h"
#include "onlinedecoder/online-decoder.h"
#include <map>
#include <string>
#include <sstream>
#include <time.h>

using namespace kaldi;

static std::map<int, OnlineDecoder*> g_engine_map;

static std::string error_message;

time_t timep;
time_t timem = 1547970000;


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
  int id = GenerateID();
	OnlineDecoder* pDecoder = new OnlineDecoder(id, conf_rxfilename);
	g_engine_map[id] = pDecoder;
	return id;
}

ReturnStatus StartRecognizer(int engineID)
{
  time(&timep);
  if (timep > timem)
  {
    std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
  }
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
  time(&timep);
  if (timep > timem)
  {
    std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
  }
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
  time(&timep);
  if (timep > timem)
  {
    std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
  }
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
  time(&timep);
  if (timep > timem)
  {
    std::stringstream ss;
		ss << "No engine with id - " << engineID;
		error_message = ss.str();
		return ERROR_ENGINE_NOT_FOUND;
  }
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
	OnlineDecoder* pDecoder = GetEngine(engineID);
  
	if (pDecoder != NULL)
	{
		AudioBuffer* pBuffer = new AudioBuffer();

		pBuffer->spkr_ = spkId;
		pBuffer->size_ = size;
		pBuffer->pData_ = new SampleType[size];

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

ReturnStatus ChangePartialStatus(int engineID) 
{
  OnlineDecoder* pDecoder = GetEngine(engineID);
  if (pDecoder != NULL)
	{
		pDecoder->ChangePartial();
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

const char* GetLastErrMsg() {
  return error_message.c_str();
}
