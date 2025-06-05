# UnityMMO

## 🧩 문제: Unity C# ↔ C++ 간 Protobuf 버전 불일치

- **상황**: Unity C# 클라이언트 ↔ C++ IOCP 서버 전환 중 메시지 직렬화 오류 발생
- **원인**: 
  - Unity C# 클라이언트: Google.Protobuf 3.12.3 사용
  - C++ 서버: Protobuf 3.6.x 사용
- **현상**: Protobuf 메시지 역직렬화 실패 → 패킷 파싱 불가

---

## ✅ 해결: C++ Protobuf 버전 업그레이드 및 통일

- C++ 서버 Protobuf 최신 빌드 절차:
  - `cmake` → Configure 시 기본 옵션 해제
  - `cmake` → Generate 해제
  - Visual Studio에서 `protobuf.sln` 열고 Debug/Release 모두 `ALL_BUILD`
- 필요한 파일:
  - `libprotobuf.lib`
  - `libprotobufd.lib`
  - `protoc.exe`
- 교체:
  - 기존 C++ 프로젝트의 `lib`, `include/google` 디렉터리 전체 교체
  - `protoc.exe`도 교체
- Unity C# 쪽은 기존 `Google.Protobuf.dll`(v3.12.3) 유지

---

## 🛠️ 추가 개선 사항

- `Protocol.proto`, `Enum.proto` 분리
- `PacketGenerator` 자동화 (jinja2 + Python 기반 스크립트)
- C# 클라이언트용, C++ 서버용 각각 생성 지원
- PacketId 자동 발급 방식 정비


# Room 기반 MMO 서버 구조 개선 기록

## 🧩 문제: 멀티스레드 환경에서 GameObject 제거 중 충돌 발생
- **상황**: Room Update는 전용 스레드 1개, 나머지는 Worker 스레드 5개로 동작
- **현상**: Arrow Update 중 충돌 발생. 제거된 객체에 접근하여 크래시 발생
- **해결**: `LeaveGame()` 시 즉시 제거하지 않고 `ReserveRemoveObjects()`를 통해 예약 → Room::Update() 내에서 일괄 처리

---

## 🧩 문제: LeaveGame과 EnterGame 간 순서 꼬임
- **상황**: 객체 사망 → LeaveGame → EnterGame(Respawn) 호출 시점이 꼬여 리스폰 후 멈추는 현상 발생
- **현상**: 객체가 다시 살아났지만 Room에 완전히 제거되지 않아 상태 불일치
- **해결**: `EnterGame()`만 `DoAsync()`로 처리하여 Room 내부 JobQueue에서 실행 → LeaveGame → Remove → Enter 순서 보장

---

## 🧩 문제: `LeaveGame()`을 DoAsync로 넘겼을 때 상태 꼬임 발생
- **상황**: LeaveGame을 Room 큐에 넘기면 그 사이 EnterGame이 먼저 실행되어 상태 충돌
- **해결**: `LeaveGame()`은 즉시 예약만 하고, `RemoveObjects()`는 `Update()`에서 처리되도록 구조 변경

---

## ✅ 최종 구조 요약

- `LeaveGame()` → `ReserveRemoveObjects()` 호출 (즉시 제거 X)
- Room 전용 `Update()` 내에서 `RemoveObjects()` 실행
- `EnterGame()`은 `DoAsync()`로 큐에 넣어 순서 보장
- Room 내 `_players`, `_monsters`, `_projectiles`에 대한 접근은 모두 Room 내부 JobQueue에서만 허용

---

## 💡 멀티스레드 & 비동기 처리에 대한 고찰

- 객체 제거, 생성 같은 **상태 변화는 오직 Room 전용 스레드(Queue) 안에서 처리**해야 안정적임
- **비동기 처리는 순서를 보장하지 않기 때문에**, 의존 순서를 갖는 로직은 **JobQueue로 넘겨 순서를 통제**
- `Reserve → 실제 처리` 구조는 **Update 루프를 중심으로 안정적인 오브젝트 관리 패턴**을 제공
- 순회를 하는 도중 삭제에 민감해질 것. 루프가 돌아가는 도중 참조가 사라지게 되면 굉장히 위험함
- 무조건 DoAsync보다, **무엇을 DoAsync로 넘길지에 대한 명확한 기준**이 중요함 (예: Enter는 큐로, Leave는 예약만)


