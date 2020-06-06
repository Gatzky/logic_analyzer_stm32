STM32 work as a logic analyzer when you connected data to pin E0.
STM32 is connected to PC and by UART data is sended to Python script which take data, processing and visualizes them
Arduno (pin13) is used as a test device, it make 1ms square signal

TODO:
1. STM32F4 is one shot: this take data, send them and do nothing it should be reapatable
2. Python dont refresh visualisation. If I send two packs of data the second one will be visualized after close first one windows
3. Sometimes 1ms square is visualise as 2ms/3ms. I dont know that this is error with send data or with processing
