#include "ShaderObject.h"

ShaderObject::ShaderObject(std::string_view name)
{
	static std::filesystem::path shaderDir;

	if (shaderDir.empty())
	{
		wchar_t moduleFileName[MAX_PATH];
		GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);

		shaderDir = moduleFileName;
		shaderDir.remove_filename();
	}

	std::ifstream shaderIn(shaderDir / name, std::ios::binary);
	if (shaderIn.is_open())
	{
		shaderIn.seekg(0, std::ios::end);
		m_Size = shaderIn.tellg();
		shaderIn.seekg(0, std::ios::beg);
		
		m_Data = malloc(m_Size);

		if (m_Data)
		{
			shaderIn.read((char*)m_Data, m_Size);
		}
	}
}

ShaderObject::~ShaderObject()
{
	if (m_Data)
	{
		free(m_Data);
	}
}
