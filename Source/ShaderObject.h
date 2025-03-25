#pragma once

#include <cstdlib>
#include <string_view>
#include <filesystem>
#include "WinInclude.h"
#include <fstream>

class ShaderObject
{
public:
	ShaderObject(std::string_view name);
	~ShaderObject();

	inline const void* GetBuffer() const { return m_Data; }
	inline size_t GetSize() const { return m_Size; }

private:
	void* m_Data = nullptr;
	size_t m_Size = 0;
};
