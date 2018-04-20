// 张; 杨
#ifndef SPEECH_RECOGNITION_ENGINE_H
#define SPEECH_RECOGNITION_ENGINE_H

// callback signals
enum DecoderSignal {
	PARTIAL_RESULT_SIGNAL,
	FINAL_RESULT_SIGNAL,
	FULL_FINAL_RESULT_SIGNAL,
	EOS_SIGNAL
};

// callback function type definitions
typedef void(*DecoderSignalCallback)(int id, const char* pszResults);

// return flag, if you get an ERROR_XXXX return status, 
// you can get more information by calling GetLastErrMsg.
enum ReturnStatus
{
	ERROR_ENGINE_NOT_FOUND,
	ERROR_UNKNOWN,
	SUCCEED,
};

// -1 for fail, >0 for a valid recognizer id
int CreateRecognizer(const char* conf_rxfilename);

// return the error message if an error have just occured
const char* GetLastErrMsg();

ReturnStatus StartRecognizer(int engineID);

ReturnStatus SuspendRecognizer(int engineID);

ReturnStatus ResumeRecognizer(int engineID);

ReturnStatus StopRecognizer(int engineID);

ReturnStatus WaitForRecogStop(int engineID);

ReturnStatus FreeRecognizer(int engineID);

ReturnStatus AddBuffer(int engineID, const char* spkId, const short* pData, int size);

ReturnStatus AddCallback(int engineID, DecoderSignal signal, DecoderSignalCallback callback);

#endif
