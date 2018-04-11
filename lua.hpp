#pragma once
#include <cstdint>
#include "Interface.hpp"

namespace lua
{
	using number_t = double;

	enum class special
	{
		glob,
		env,
		reg
	};

	enum class type
	{
		client,
		server,
		menu
	};

	class Interface
	{
	public:

		virtual int			Top(void) = 0;
		virtual void		Push(int iStackPos) = 0;
		virtual void		Pop(int iAmt = 1) = 0;
		virtual void		GetTable(int iStackPos) = 0;
		virtual void		GetField(int iStackPos, const char* strName) = 0;
		virtual void		SetField(int iStackPos, const char* strName) = 0;
		virtual void		CreateTable() = 0;
		virtual void		SetTable(int i) = 0;
		virtual void		SetMetaTable(int i) = 0;
		virtual bool		GetMetaTable(int i) = 0;
		virtual void		Call(int iArgs, int iResults) = 0;
		virtual int			PCall(int iArgs, int iResults, int iErrorFunc) = 0;
		virtual int			Equal(int iA, int iB) = 0;
		virtual int			RawEqual(int iA, int iB) = 0;
		virtual void		Insert(int iStackPos) = 0;
		virtual void		Remove(int iStackPos) = 0;
		virtual int			Next(int iStackPos) = 0;
		virtual void*		NewUserdata(unsigned int iSize) = 0;
		virtual void		ThrowError(const char* strError) = 0;
		virtual void		CheckType(int iStackPos, int iType) = 0;
		virtual void		ArgError(int iArgNum, const char* strMessage) = 0;
		virtual void		RawGet(int iStackPos) = 0;
		virtual void		RawSet(int iStackPos) = 0;
		virtual const char*	GetString(int iStackPos = -1, unsigned int* iOutLen = nullptr) = 0;
		virtual double		GetNumber(int iStackPos = -1) = 0;
		virtual bool		GetBool(int iStackPos = -1) = 0;
		virtual void*		GetCFunction(int iStackPos = -1) = 0;
		virtual void*		GetUserdata(int iStackPos = -1) = 0;
		virtual void		PushNil() = 0;
		virtual void		PushString(const char* val, unsigned int iLen = 0) = 0;
		virtual void		PushNumber(double val) = 0;
		virtual void		PushBool(bool val) = 0;
		virtual void		PushCFunction(void* val) = 0;
		virtual void		PushCClosure(void* val, int iVars) = 0;
		virtual void		PushUserdata(void*) = 0;
		virtual int			ReferenceCreate() = 0;
		virtual void		ReferenceFree(int i) = 0;
		virtual void		ReferencePush(int i) = 0;
		virtual void		PushSpecial(special iType) = 0;
		virtual bool		IsType(int iStackPos, int iType) = 0;
		virtual int			GetType(int iStackPos) = 0;
		virtual const char*	GetTypeName(int iType) = 0;
		virtual void		CreateMetaTableType(const char* strName, int iType) = 0;
		virtual const char*	CheckString(int iStackPos = -1) = 0;
		virtual double		CheckNumber(int iStackPos = -1) = 0;
	};

	class Shared
	{
	public:
		inline Interface* create_interface(type state)
		{
			using create_interface_t = Interface * (__thiscall*)(void*, type);
			return method<create_interface_t>(6, this)(this, state);
		}
	};
}

namespace globals
{
	inline lua::Shared* lua = nullptr;
}