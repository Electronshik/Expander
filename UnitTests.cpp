#include "pch.h"
#include "CppUnitTest.h"
#include <iostream>
#include "etl_profile.h"
#include "../Inc/logger.hpp"
#include "logger_mock.hpp"
#include <codecvt>
#include <string>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(LoggerTests)
	{
	public:
		TEST_METHOD(ConstructTest)
		{
			TestLoggerHardware TestHardware;
			MemoryLogger Logger(TestHardware);
			Logger.ResetLogPointer();
			Assert::AreEqual(Logger.GetLogPointer(), 4U);
		}
		TEST_METHOD(WriteReadTest)
		{
			TestLoggerHardware TestHardware;
			MemoryLogger Logger(TestHardware);
			Logger.ResetLogPointer();
			char str[] = "First Test";
			Logger.WriteToLog(str, 10);
			Assert::AreEqual(Logger.GetLogPointer(), 14U);
			uint8_t buff[10];
			TestHardware.MemoryRead(Logger.GetLogPointer() - 10, buff, 10);
			for (int i = 0; i < 10; i++)
			{
				Assert::AreEqual(buff[i], *(reinterpret_cast<uint8_t*>(str+i)));
			}
		}
		TEST_METHOD(TransmitLogTest)
		{
			TestLoggerHardware TestHardware;
			MemoryLogger Logger(TestHardware);
			Logger.ResetLogPointer();
			char str[] = "First Test";
			Logger.WriteToLog(str, 10);
			Assert::IsTrue(Logger.GetLogPointer() == 14);
			Logger.TransmitLogToOtherInterface(10);
			for (int i = 0; i < 10; i++)
			{
				Assert::AreEqual(TestHardware.TestOtherInterface[i], *(reinterpret_cast<uint8_t*>(str + i)));
			}
		}
		TEST_METHOD(WriteReadUpOverflowTest)
		{
			TestLoggerHardware TestHardware;
			MemoryLogger Logger(TestHardware);
			Logger.LogPointerSave(TestHardware.GetMaxAddress() - 8U);
			char str[] = "First Test!&";
			Logger.WriteToLog(str, 12);
			Assert::AreEqual(Logger.GetLogPointer(), 8U);
			uint8_t buff[12];
			TestHardware.MemoryRead(Logger.GetLogPointer() - 4U, (buff + 8), 4);
			TestHardware.MemoryRead(TestHardware.GetMaxAddress() - 8, buff, 8);

			Logger::WriteMessage("Buff: ");
			Logger::WriteMessage((char*)buff);

			Logger::WriteMessage("str: ");
			Logger::WriteMessage((char*)str);

			for (int i = 0; i < 12; i++)
			{
				Assert::IsTrue(buff[i] == *(reinterpret_cast<uint8_t*>(str + i)));
			}
		}
		TEST_METHOD(TransmitLogOverflowTest)
		{
			TestLoggerHardware TestHardware;
			MemoryLogger Logger(TestHardware);
			Logger.ResetLogPointer();
			char str[] = "First Test&!";
			Logger.WriteToLog(str, 12);
			Assert::IsTrue(Logger.GetLogPointer() == 16);
			Logger.TransmitLogToOtherInterface(12);
			for (int i = 0; i < 12; i++)
			{
				Assert::AreEqual(TestHardware.TestOtherInterface[i], *(reinterpret_cast<uint8_t*>(str + i)));
			}
			char str2[] = "End of mem";
			TestHardware.MemoryWrite(TestHardware.GetMaxAddress() - 10, reinterpret_cast<uint8_t *>(str2), 10);
			Logger.TransmitLogToOtherInterface(22);
			for (int i = 0; i < 10; i++)
			{
				Assert::AreEqual(TestHardware.TestOtherInterface[i], *(reinterpret_cast<uint8_t*>(str2 + i)));
			}
			for (int i = 0; i < 12; i++)
			{
				Assert::AreEqual(TestHardware.TestOtherInterface[i+10], *(reinterpret_cast<uint8_t*>(str + i)));
			}
		}
	};
}
