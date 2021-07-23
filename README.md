#### В репозитории расположены:
###### logger.hpp
- класс MemoryLogger, записывающий логи через интерфейс IMemoryLogger (внедренная зависимость)
###### logger_hardware.hpp
- класс MemoryLoggerHardware, железная реализация IMemoryLogger для конкретной микросхемы I2C памяти
###### logger_mock.hpp
- класс TestLoggerHardware, реализация IMemoryLogger в виде буфера для тестирования без железа
###### UnitTests.cpp
- unit-тесты в Microsoft Visual Studio

