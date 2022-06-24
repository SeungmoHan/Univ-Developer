#pragma once
#ifndef __NETSERVER_CORE_CLASS_DEF__
#define __NETSERVER_CORE_CLASS_DEF__
#define __UNIV_DEVELOPER_

#include "CoreBase.h"
#include "NetCoreErrorDefine.h"
#include "Session.h"
#include <HardWareMoniteringClass.h>
#include <ProcessMoniteringClass.h>

#include <stack>

//#define dfMAX_NUM_OF_THREAD 16

//new delete는... 문제가 없는데
//어째서 object pool은 문제가 되는가...
//일단 네트워크 라이브러리화 시켜놓자 내일은

namespace univ_dev
{
	class CNetServer
	{
	public:
		friend class BaseServer;
		~CNetServer();
		CNetServer();
	public:
		void	InitNetServer(USHORT port, DWORD backlogQueueSize, DWORD threadPoolSize, DWORD runningThread, DWORD nagleOff, ULONGLONG maxSessionCounts, DWORD timeOutClock);
		bool	GetNetCoreInitializeFlag() { return this->_ServerOnFlag; }
		DWORD	GetLastCoreErrno() { return this->_ErrorCode; }
		DWORD	GetLastAPIErrno() { return this->_APIErrorCode; }
		void	Run(HANDLE* threadArr, size_t size);
		void	SendToMoniteringSession(Packet* packet);
		void	DisconnectSession(ULONGLONG sessionID);
		void	SendPacket(ULONGLONG sessionID, Packet* packet);

	private:
		//------------------------------------------------------------------------------------------------
		// 서버 시작 종료함수
		void Startup();
		void Cleanup();
		//------------------------------------------------------------------------------------------------
	private:
		//init 함수들
		bool InitEvents();
		bool CreateMoniteringThread();
		bool CreateTimeOutThread();
		bool CreateWorkerThread();
		bool InitListenSocket();
		bool CreateAcceptThread();


		//------------------------------------------------------------------------------------------------
		// _beginthreadex함수에 전달되는 함수포인터들 param 은 this
		friend unsigned __stdcall _NET_WorkerThread(void* param);
		friend unsigned __stdcall _NET_AcceptThread(void* param);
		friend unsigned __stdcall _NET_TimeOutThread(void* param);
		friend unsigned __stdcall _NET_MoniteringConnectThread(void* param);
		friend unsigned __stdcall _NET_SendThread(void* param);


		// 실제 러닝 스레드들이 호출할 함수들
		unsigned int CNetServerWorkerThread(void* param);
		unsigned int CNetServerAcceptThread(void* param);
		unsigned int CNetServerTimeOutThread(void* param);
		unsigned int CNetServerMoniteringThread(void* param);
		unsigned int CNetServerSendThread(void* param);
		//------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------
		// 모니터링 서버 연결함수 -> 끊어지면 재연결 계속 반복
		void ConnectMonitoringSession();
		void RecvFromMonitoringSession();


	protected:

		//------------------------------------------------------------------------------------------------
		//가상함수 -> 오버라이딩 필수
		virtual void OnRecv(ULONGLONG sessionID, Packet* recvPacket) = 0;
		virtual void OnErrorOccured(DWORD errorCode, const WCHAR* error, LogClass::LogLevel level) = 0;
		virtual bool OnConnectionRequest(WCHAR* ipStr, DWORD ip, USHORT port) = 0;
		virtual void OnClientJoin(WCHAR* ipStr, DWORD ip, USHORT port, ULONGLONG sessionID) = 0;
		virtual void OnClientLeave(ULONGLONG sessionID) = 0; // Release후 호출
		virtual void OnTimeOut(ULONGLONG sessionID) = 0;

		virtual void OnSend(ULONGLONG sessionID) = 0;

		inline void	PostOnClientLeave(ULONGLONG sessionID)
		{
			PostQueuedCompletionStatus(this->_IOCP, 0, sessionID, (LPOVERLAPPED)0xffffffff);
		}
	protected:
		//------------------------------------------------------------------------------------------------
		// 에러났을때 에러코드 저장하면서 OnErrorOccured 호출
		inline void DispatchError(DWORD errorCode, DWORD APIErrorCode, const WCHAR* errorStr)
		{
			//라이브러리 자체 에러코드 등록 및 API에러 코드 등록
			this->_ErrorCode = errorCode;
			this->_APIErrorCode = APIErrorCode;
			// OnErrorOccured 함수는 라이브러리 에러코드를 전달하므로 GetLastAPIErrorCode 함수 호출해서 값을 얻어가야됨.
			this->OnErrorOccured(errorCode, errorStr, LogClass::LogLevel::LOG_LEVEL_LIBRARY);
		}
		//------------------------------------------------------------------------------------------------
	private:

		//------------------------------------------------------------------------------------------------
		//Send완료통지, Recv완료통지시 수행
		void		RecvProc(Session* session, DWORD byteTransfered);
		void		SendProc(Session* session, DWORD byteTransfered);
		//------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------
		//실제 WSASend, WSARecv호출하는 함수
		void		RecvPost(Session* session);
		void		SendPost(Session* session);
		//------------------------------------------------------------------------------------------------


		//------------------------------------------------------------------------------------------------
		// Accept 수행후 수행 불가능이면 clientSocket이 INVALID_SOCKET임, false 반환시에는 AcceptThread 종료
		BOOL		TryAccept(SOCKET& clientSocket, sockaddr_in& clientAddr);
		// 하나의 완성된 패킷을 얻어내는 함수
		BOOL		TryGetCompletedPacket(Session* session, Packet* packet, NetServerHeader& header);


		//------------------------------------------------------------------------------------------------
		// 세션ID -> 세션포인터 
		Session* FindAndLockSession(ULONGLONG sessionID);
		inline Session* FindSession(ULONGLONG sessionID)
		{
			return &this->_SessionArr[sessionID & 0xffff];
		}
		//------------------------------------------------------------------------------------------------


		//------------------------------------------------------------------------------------------------
		// 헬퍼함수 sockaddr_in 구조체를 넣으면 wide string 으로 반환
		inline void GetStringIP(WCHAR* str, DWORD bufferLen, sockaddr_in& addr)
		{
			wsprintf(str, L"%d.%d.%d.%d", addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2, addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4);
		}
		//------------------------------------------------------------------------------------------------


		//------------------------------------------------------------------------------------------------
		// 세션 컨테이너에 대한 락
		void		SessionMapLock();
		void		SessionMapUnlock();
		//------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------
		// 세션 사용권 획득 및 반환 함수
		Session*	AcquireSession(ULONGLONG SessionID);
		//void		ReturnSession(ULONGLONG SessionID);
		void		ReturnSession(Session* session);
		//------------------------------------------------------------------------------------------------
		inline void SetSessionTimer(Session* session)
		{
			InterlockedExchange(&session->_TimeOutTimer, timeGetTime());
		}

		//------------------------------------------------------------------------------------------------
		// 세션 생성및 삭제함수
		Session*	CreateSession(SOCKET key, sockaddr_in clientaddr, ULONGLONG sessionID);
		void		ReleaseSession(Session* session);
		//------------------------------------------------------------------------------------------------
	private:

		//------------------------------------------------------------------------------------------------

		//------------------------------------------------------------------------------------------------
		// 서버에서 사용하는 변수들
		// ReadOnly Variable
		HANDLE									_IOCP = nullptr;
		volatile SOCKET							_ListenSocket = INVALID_SOCKET;
		USHORT									_ServerPort;
		//Thread Handler
		HANDLE*									_WorkerThreads = nullptr;
		HANDLE									_AcceptThread = nullptr;
		HANDLE									_TimeOutThread = nullptr;
		HANDLE									_SendThread = nullptr;
		DWORD									_ThreadPoolSize;
		DWORD									_RunningThreadCount;
		DWORD									_NagleOff;
		ULONGLONG								_MaxSessionCounts;
		DWORD									_TimeOutClock;
		//Server Status
	public:
		volatile BOOL							_ShutDownFlag;
	protected:
		HANDLE									_ThreadStartEvent = nullptr;
	private:
		DWORD									_BackLogQueueSize;
		
		DWORD									_ServerTime;

		HANDLE									_MonitoringSignal = nullptr;
		HANDLE									_MoniteringConnectThread = nullptr;
		BOOL									_MoniteringFlag = false;
		Session									_MonitoringServerSession;

	private:
		//Error and codes
		volatile static DWORD					_ServerOnFlag;
		volatile DWORD							_ErrorCode;
		volatile DWORD							_APIErrorCode;
		//------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------
		//세션 관련 객체들
		CRITICAL_SECTION						_SessionMapLock;
		Session*								_SessionArr;
		LockFreeStack<DWORD>					_SessionIdx;


		//Logging Class
		LogClass								_LibraryLog;

		inline bool PopSessionIndex(DWORD& ret)
		{
			return this->_SessionIdx.pop(ret);
		}
		inline void PushSessionIndex(DWORD idx)
		{
			return this->_SessionIdx.push(idx);
		}

	public:
		struct MoniteringInfo
		{
			DWORD								_WorkerThreadCount;
			DWORD								_RunningThreadCount;
			DWORD								_CurrentSessionCount;

			ULONGLONG							_TotalProcessedBytes;

			ULONGLONG							_TotalAcceptSessionCount;
			ULONGLONG							_TotalReleaseSessionCount;

			ULONGLONG							_TotalSendPacketCount;
			ULONGLONG							_TotalRecvPacketCount;

			DWORD								_SessionSendQueueSize;
			DWORD								_SessionSendQueueCapacity;
			DWORD								_SessionSendQueueMax;

			DWORD								_SessionIndexStackSize;
			DWORD								_SessionIndexStackCapacity;
		};
	protected:
		inline void WaitForMoniteringSignal()
		{
			::WaitForSingleObject(this->_MonitoringSignal, 2000);
		}
		
		void PostServerStop()
		{
			this->_ShutDownFlag = true;
			closesocket(this->_ListenSocket);
		}

		alignas(64) ULONGLONG					_BeginTime = 0;

		// 패킷 처리 수치 및 패킷처리 완료 바이트수
		alignas(64) ULONGLONG					_CurSessionCount = 0;

		alignas(64) ULONGLONG					_TotalSendPacketCount = 0;
		alignas(64) ULONGLONG					_TotalRecvPacketCount = 0;

		alignas(64) ULONGLONG					_TotalProcessedBytes = 0;

		alignas(64) ULONGLONG					_TotalAcceptSessionCount = 0;
		alignas(64) ULONGLONG					_TotalReleaseSessionCount = 0;

		void GetMoniteringInfo(MoniteringInfo& info);
		DWORD GetBeginTime()const { return _BeginTime; }
	};
}



#endif // !__NETSERVER_CORE_CLASS_DEF__
