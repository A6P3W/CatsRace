#include "Map.h"
#include "SpriteComponent.h"
#include "ResourceManager.h"
#include "RenderSystem.h"
#include "UMath.h"
AMap::AMap(FVector2D location, FRotator rotation)
{
	SetActorLocation(location);
	SetActorScale(FScale(5.0f));
	int handle = ResourceManager::GetInstance().LoadResourceGraph("Image/Asama_Test_Course.png");
	auto sprite = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite->SubmitGraph( handle);
	AddComponent(std::move(sprite));

}
