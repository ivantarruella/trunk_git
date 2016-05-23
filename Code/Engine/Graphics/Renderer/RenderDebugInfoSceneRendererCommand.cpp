#include "RenderDebugInfoSceneRendererCommand.h"
#include "RenderManager.h"
#include "Matrix44.h"
#include "Process.h"
#include "Console.h"
#include "Base.h"

CRenderDebugInfoSceneRendererCommand::CRenderDebugInfoSceneRendererCommand(CXMLTreeNode &atts)
	: CSceneRendererCommand(atts)
{
}

void CRenderDebugInfoSceneRendererCommand::Execute(CRenderManager &RM)
{
#ifdef _DEBUG	
	// Render logger
	CORE->GetLogRender()->Render(&RM, CORE->GetFontManager(), colYELLOW);
	// Render Console
	CORE->GetConsole()->Render(&RM, CORE->GetFontManager());
#endif
	//---Draw 2D text (fps,...)---
	CORE->DrawFPS();
	// Render debug process info
	CORE->GetProcess()->RenderDebug(&RM);
}
