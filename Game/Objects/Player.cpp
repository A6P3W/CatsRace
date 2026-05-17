#include "Player.h"
#include "InputMapper.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "Objects/DefaultScene.h"
#include <DxLib.h>
#include <ObjectManager.h>
#include "CSVMap.h"
#include <MovementComponent.h>
#include <algorithm>
APlayer::APlayer()
{

	int handle = ResourceManager::GetInstance().LoadResourceGraph("BaseFile/texture_Checker_64px.png");
	auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
	sprite->SubmitGraph(1.0, handle);
	AddComponent(std::move(sprite));


	auto movement = std::make_unique<MMovementComponent>();
	m_movement = movement.get();
	AddComponent(std::move(movement));



}
void APlayer::OnUpdate(float DeltaTime)
{
	float moveSpeed = 1000.0f;
	float rotationSpeed = 180.0f;
	if (InputMapper::GetInstance().GetKeyPressing(E_INPUT_ACTION::UP)) {
		
	}
	if (InputMapper::GetInstance().GetKeyPressing(E_INPUT_ACTION::DOWN)) {
		
	}
	if (InputMapper::GetInstance().GetKeyPressing(E_INPUT_ACTION::LEFT)) {
		
	}
	if (InputMapper::GetInstance().GetKeyPressing(E_INPUT_ACTION::RIGHT)) {
		
		
	}
	m_slider = std::clamp(m_slider, -1.0f, 1.0f);
	float velocity = std::clamp(m_movement->GetVelocitySizeSquared()-0.9f, 0.0f, 1.0f);
	AddActorRotation(rotationSpeed* m_slider*velocity * DeltaTime);
	m_movement->AddVelocityRotation(rotationSpeed * m_slider * velocity * DeltaTime);

	if (InputManager::GetInstance().GetKeyPressStart(KEY_INPUT_SPACE)) {
		int handle = ResourceManager::GetInstance().LoadResourceGraph("BaseFile/texture_Checker_64px.png");
		auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
		sprite->SubmitGraph(1.0, handle);
		sprite->SetParentComponent(this->GetRootComponent());
		sprite->AddWorldOffset({ 220.0f, -100.0f });
		sprite->SetScale(0.5f);
		AddComponent(std::move(sprite));
		ObjectManager::GetInstance().SpawnObject<ACSVMap>(GetActorLocation(), FRotator{ 0 });
	}
	if (InputManager::GetInstance().GetKeyPressStart(KEY_INPUT_Z)) {
		SceneManager::GetInstance().OpenScene<ADefaultScene>();
	}
	if (InputManager::GetInstance().GetMouseWheelUp()) {
		m_camera->SetFOV(m_camera->GetFOV() * 1.05);
	}
	if (InputManager::GetInstance().GetMouseWheelDown()) {
		m_camera->SetFOV(m_camera->GetFOV() * 0.95);
	}
}

void APlayer::OnPossesed()
{
	m_camera->SetActiveCamera();
	m_camera->SetFOV(0.2);
}


//{
//	if (Scale > 0) {
//		m_movement->AddLocalForce({ 0.0f, -2.0f });
//	}
//	else {
//		m_movement->AddLocalForce({ 0.0f, 2.0f });
//	}
//}
//
//
//{
//	if (Scale > 0) {
//		m_slider -= 0.1;
//	}
//	else {
//		m_slider += 0.1;
//	}
//}
