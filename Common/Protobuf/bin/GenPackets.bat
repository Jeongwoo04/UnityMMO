@echo off
pushd %~dp0

REM --- C# protobuf 컴파일 ---
protoc.exe -I=./ --csharp_out=./ ./Enum.proto
IF ERRORLEVEL 1 (
    echo protoc C# compile Enum.proto failed
    PAUSE
    exit /b 1
)

protoc.exe -I=./ --csharp_out=./ ./Protocol.proto
IF ERRORLEVEL 1 (
    echo protoc C# compile failed
    PAUSE
    exit /b 1
)

REM --- CPacketGenerator 실행 (Enum.proto를 인자로) ---
START /WAIT ../../../CPacketGenerator/bin/Debug/net6.0/CPacketGenerator.exe ./Enum.proto
IF ERRORLEVEL 1 (
    echo CPacketGenerator failed
    PAUSE
    exit /b 1
)

REM --- C# 결과물 복사 ---
XCOPY /Y Enum.cs "../../../Client/Assets/Scripts/Packet"
XCOPY /Y Protocol.cs "../../../Client/Assets/Scripts/Packet"
XCOPY /Y ClientPacketManager.cs "../../../Client/Assets/Scripts/Packet"

REM --- C++ protobuf 컴파일 ---
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
IF ERRORLEVEL 1 (
    echo protoc C++ compile Enum.proto failed
    PAUSE
    exit /b 1
)
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
IF ERRORLEVEL 1 (
    echo protoc C++ compile Struct.proto failed
    PAUSE
    exit /b 1
)
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
IF ERRORLEVEL 1 (
    echo protoc C++ compile Protocol.proto failed
    PAUSE
    exit /b 1
)

REM --- GenPackets 실행 (Protocol.proto 인자) ---
GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
IF ERRORLEVEL 1 (
    echo GenPackets failed
    PAUSE
    exit /b 1
)

REM --- C++ 결과물 복사 ---
XCOPY /Y Enum.pb.h "../../../GameServer"
XCOPY /Y Enum.pb.cc "../../../GameServer"
XCOPY /Y Struct.pb.h "../../../GameServer"
XCOPY /Y Struct.pb.cc "../../../GameServer"
XCOPY /Y Protocol.pb.h "../../../GameServer"
XCOPY /Y Protocol.pb.cc "../../../GameServer"
XCOPY /Y ClientPacketHandler.h "../../../GameServer"

REM --- 임시 파일 삭제 ---
DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h
DEL /Q /F *.cs

popd