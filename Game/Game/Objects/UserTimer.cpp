#include "UserTimer.h"
#include <RenderSystem.h>
#include <ResourceManager.h>
void UserTimer::OnUpdate(float DeltaTime)
{

	m_timer += DeltaTime;
	int font = ResourceManager::GetInstance().GetFont(24, -1);
	RenderSystem::GetInstance().SubmitText({100,100}, "Timer: " + std::to_string(m_timer), font,  0x00FF00, RenderSpace::Screen, 100);

}
