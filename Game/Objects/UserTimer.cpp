#include "UserTimer.h"
#include <RenderSystem.h>
#include <ResourceManager.h>
void UserTimer::OnUpdate(float DeltaTime)
{

	m_timer += DeltaTime;
	int font = ResourceManager::GetInstance().GetFont(24, -1);
	RenderSystem::GetInstance().SubmitText("Timer: " + std::to_string(m_timer), 100, 100, 0x00FF00, font, RenderSpace::Screen, 100);

}
