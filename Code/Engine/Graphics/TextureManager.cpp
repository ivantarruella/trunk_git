#include "TextureManager.h"
#include "StaticMeshManager.h"
#include "Base.h"

CTextureManager::CTextureManager()
{
}

CTextureManager::~CTextureManager()
{
	Destroy();
}

void CTextureManager::Reload ()
{
	TMapResource::const_iterator it = m_Resources.begin();
	for (; it != m_Resources.end(); ++it)
	{
		it->second->Reload();
	}
}

CTexture * CTextureManager::GetTexture(const std::string &Filename)
{
#if MULTITHREADED_LOAD			
	std::lock_guard<std::mutex> lk(CORE->GetStaticMeshManager()->GetMutex());
	// don't allow acces from multiple threads in this code:
#endif

	CTexture* l_pTex = GetResource(Filename);
	if(l_pTex == NULL)
	{
		l_pTex = new CTexture;
		l_pTex->Load(Filename);
		AddResource(Filename, l_pTex);
	}
	return l_pTex;
}


void CTextureManager::LoadTextures(const std::string& FolderName, const std::vector<std::string>& TexturesVect, size_t from, size_t size)
{
	for (size_t m = from; (m < from + size) && (m < TexturesVect.size()); m++)
	{
		std::string path = FolderName + '\\' + TexturesVect[m];
		CTexture* text = new CTexture;
		text->Load(path);
		AddResource(TexturesVect[m], text);
	}
}
