# UnityMMO

✅ C++ 서버 전환 및 Protobuf 버전 통일

Protocol.proto 와 Enum.proto 분리. 자동화 코드 수정, PacketId 발급 부분 수정.
C# 클라이언트와 C++ 게임서버 각각의 PacketGenerator를 jinja2 py tool로 자동화

Unity C# 클라이언트 ↔ 기존 C# 서버 구조에서,
C++ IOCP 기반 서버로 전환하는 과정에서 Protobuf 버전 차이로 문제 발생.

⚠️ 문제 원인
Unity C# 클라이언트 Google.Protobuf 3.12.3

C++ 서버 Protobuf 3.6.x 낮은 버전 사용

메시지 직렬화/역직렬화 시 버전 호환 오류 발생

✅ 해결 방법

cmake 사용 빌드 -> Configure default -> Generate 체크 모두 해제

Visual Studio -> protobuf.sln -> Debug/Release 각각 빌드 (ALL_BUILD)

생성된 다음 파일들을 사용:

```diff
- libprotobuf.lib
- libprotobufd.lib
- protoc.exe
```
교체 대상

C++ 프로젝트의 기존 protobuf lib, include/google 폴더 전부 교체

protoc.exe 새로운 것으로 교체

Unity C# 쪽 기존 Google.Protobuf.dll (v3.12.3) 그대로 사용
