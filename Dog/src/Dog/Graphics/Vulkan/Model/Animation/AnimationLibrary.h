#pragma once

namespace Dog
{
	class Animator;
	class Animation;
	class Model; // <- temporary, animation should not need model ideally.

	class AnimationLibrary
	{
	public:
		AnimationLibrary();
		~AnimationLibrary();

		uint32_t AddAnimation(const std::string& animPath, Model* model);
        Animation* GetAnimation(uint32_t index);
        Animator* GetAnimator(uint32_t index);
		const std::vector<glm::mat4>& GetAnimationMatrices();
		void UpdateAnimations(float dt);
		void UpdateAnimation(uint32_t index, float dt);

		uint32_t GetAnimationCount() const { return static_cast<uint32_t>(mAnimation.size()); }

		const static uint32_t INVALID_ANIMATION_INDEX;

	private:
		friend class Model;

        std::vector<std::unique_ptr<Animation>> mAnimation;
        std::vector<std::unique_ptr<Animator>> mAnimators;

        std::unordered_map<std::string, uint32_t> mAnimationMap;
        std::vector<glm::mat4> mAnimationMatrices;
	};
}
