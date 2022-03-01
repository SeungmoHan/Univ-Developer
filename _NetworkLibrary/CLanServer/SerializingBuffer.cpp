#include "SerializingBuffer.h"
#include <string>

namespace univ_dev
{
	Packet::Packet() : _Begin(new char[PacketSize::DefaultSize]), _End(_Begin + PacketSize::DefaultSize), _WritePointer(_Begin), _ReadPointer(_Begin), _BufferSize(PacketSize::DefaultSize) {}
	//Packet::Packet() : begin((char*)bufferFreeList.Alloc()), end(begin + PacketSize::DefaultSize), writePointer(begin), readPointer(begin), bufferSize(PacketSize::DefaultSize) {}

	Packet::Packet(int bufferSize) : _Begin(new char[bufferSize]), _End(_Begin + bufferSize), _WritePointer(_Begin), _ReadPointer(_Begin), _BufferSize(bufferSize) {}

	Packet::~Packet()
	{
		Release();
	}

	void Packet::Release()
	{
		delete[] _Begin;
	}

	void Packet::Clear()
	{
		_WritePointer = _ReadPointer = _Begin;
	}

	int Packet::GetBufferSize()
	{
		return _WritePointer - _ReadPointer;
	}

	char* Packet::GetWritePtr()
	{
		return _WritePointer;
	}

	char* Packet::GetReadPtr()
	{
		return _ReadPointer;
	}
	int Packet::MoveWritePtr(int size)
	{
		if (_WritePointer + size >= _End)
		{
			_WritePointer = _End - 1;
			return _End - _WritePointer;
		}
		_WritePointer += size;
		return size;
	}

	int Packet::MoveReadPtr(int size)
	{
		if (_ReadPointer + size >= _End)
		{
			_ReadPointer = _End - 1;
			return _End - _ReadPointer;
		}
		_ReadPointer += size;
		return size;
	}

	Packet& Packet::operator=(const Packet& other)
	{
		this->_BufferSize = other._BufferSize;
		_Begin = new char[_BufferSize];
		_End = _Begin + _BufferSize;
		memcpy_s(_Begin, other._WritePointer - other._ReadPointer, other._ReadPointer, other._WritePointer - other._ReadPointer);
		return *this;
	}

	Packet& Packet::operator>>(unsigned char& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		value = *_ReadPointer;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(char& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		value = *_ReadPointer;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(unsigned short& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		unsigned short* tempPtr = (unsigned short*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(short& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		short* tempPtr = (short*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(int& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		int* tempPtr = (int*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator>>(unsigned int& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		unsigned int* tempPtr = (unsigned int*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator>>(unsigned long& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		unsigned long* tempPtr = (unsigned long*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator>>(long& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		long* tempPtr = (long*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(float& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		float* tempPtr = (float*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(double& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		double* tempPtr = (double*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(__int64& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		__int64* tempPtr = (__int64*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator>>(unsigned __int64& value)
	{
		if (_ReadPointer + sizeof(value) - 1 >= _End)
		{
			//읽기 불가능
			return *this;
		}
		unsigned __int64* tempPtr = (unsigned __int64*)_ReadPointer;
		value = *tempPtr;
		MoveReadPtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(unsigned char value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		*_WritePointer = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(char value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		*_WritePointer = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(unsigned short value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		unsigned short* tempPtr = (unsigned short*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(short value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		short* tempPtr = (short*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(int value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		int* tempPtr = (int*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator<<(unsigned int value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		unsigned int* tempPtr = (unsigned int*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator<<(unsigned long value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		unsigned long* tempPtr = (unsigned long*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}
	Packet& Packet::operator<<(long value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		long* tempPtr = (long*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(float value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		float* tempPtr = (float*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(__int64 value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		__int64* tempPtr = (__int64*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(unsigned __int64 value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		unsigned __int64* tempPtr = (unsigned __int64*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}

	Packet& Packet::operator<<(double value)
	{
		if (_WritePointer + (sizeof(value) - 1) >= _End)
		{
			//저장이 불가능한경우.
			return *this;
		}
		double* tempPtr = (double*)_WritePointer;
		*tempPtr = value;
		MoveWritePtr(sizeof(value));
		return *this;
	}


}