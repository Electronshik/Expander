#pragma once
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include	"stdint.h"

class IMemoryLogger
{
	public:
		virtual int8_t MemoryWrite (uint32_t Address, uint8_t *Data, uint32_t Size) = 0;
		virtual int8_t MemoryRead (uint32_t Address, uint8_t *Data, uint32_t Size) = 0;
		virtual void TransmitLogToOtherHardware (uint8_t *Data, uint16_t Length) = 0;
		virtual uint32_t GetMaxAddress () = 0;
		virtual ~IMemoryLogger () {}
};

class MemoryLogger
{
	public:
		MemoryLogger (IMemoryLogger& ILog);
		void WriteToLog (char *Message, uint32_t Size);
		void TransmitLogToOtherInterface (uint32_t Length);
		void ResetLogPointer ();
		uint32_t GetLogPointer ();
		void LogPointerSave ();
		void LogPointerSave (uint32_t NewLogPointer);
	private:
		uint32_t LogPointer;
		IMemoryLogger& ILogger;
};

MemoryLogger::MemoryLogger (IMemoryLogger& ILog) : ILogger (ILog)
{
	if (ILogger.MemoryRead (0, reinterpret_cast<uint8_t *>(&LogPointer), 4); LogPointer < 4)	LogPointer = 4;
}

void MemoryLogger::WriteToLog (char *Message, uint32_t Size)
{
	ILogger.MemoryWrite (LogPointer, reinterpret_cast<uint8_t *>(Message), Size);
	if ((LogPointer + Size) > ILogger.GetMaxAddress ())
	{
		LogPointer = LogPointer + 4 + Size - ILogger.GetMaxAddress ();
	}
	else
	{
		LogPointer += Size;
	}
	LogPointerSave ();
}

void MemoryLogger::TransmitLogToOtherInterface (const uint32_t Length)
{
	if ((Length + 4) >= ILogger.GetMaxAddress())	return;
	uint8_t buffer[256];
	if (LogPointer < (Length + 4))
	{
		uint32_t len = (Length + 4) - LogPointer;
		if (len != 0)
		{
			ILogger.MemoryRead (ILogger.GetMaxAddress() - len, buffer, len);
		}
		len = Length - len;
		ILogger.MemoryRead (4, (buffer + Length - len), len);
		ILogger.TransmitLogToOtherHardware (buffer, Length);
	}
	else
	{
		ILogger.MemoryRead ((LogPointer - Length), buffer, Length);
		ILogger.TransmitLogToOtherHardware (buffer, Length);
	}
}

void MemoryLogger::ResetLogPointer ()
{
	LogPointerSave (4);
}

uint32_t MemoryLogger::GetLogPointer ()
{
	return LogPointer;
}

void MemoryLogger::LogPointerSave ()
{
	ILogger.MemoryWrite (0, reinterpret_cast<uint8_t *>(&LogPointer), 4);
}

void MemoryLogger::LogPointerSave (uint32_t NewLogPointer)
{
	LogPointer = NewLogPointer;
	ILogger.MemoryWrite (0, reinterpret_cast<uint8_t *>(&LogPointer), 4);
}

#endif
