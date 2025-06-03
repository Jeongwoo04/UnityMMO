#include "pch.h"
#include "ServerGlobal.h"

//DBConnectionPool*		GDBConnectionPool = nullptr;
shared_ptr<Room>		GRoom = nullptr;

void ServerGlobal::Init()
{
	//GDBConnectionPool	= new DBConnectionPool();
	GRoom				= MakeShared<Room>();
}