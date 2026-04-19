#pragma once
class GameObject {
public:
	virtual ~GameObject() {}
	virtual void Update() = 0; // 更新
	virtual void Draw() = 0;   // 描画
};