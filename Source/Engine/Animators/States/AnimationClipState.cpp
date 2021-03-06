#include "AnimationClipState.h"
#include "AnimationClipNodeBase.h"

USING_AWAY_NAMESPACE

AnimationClipState::AnimationClipState(IAnimator* animator, AnimationClipNodeBase* animationClipNode) : AnimationStateBase(animator, animationClipNode)
{

}

float AnimationClipState::getBlendWeight()
{
	if (m_framesDirty)
		updateFrames();

	return m_blendWeight;
}

unsigned int AnimationClipState::getCurrentFrame()
{
	if (m_framesDirty)
		updateFrames();

	return m_currentFrame;
}

unsigned int AnimationClipState::getNextFrame()
{
	if (m_framesDirty)
		updateFrames();

	return m_nextFrame;
}

void AnimationClipState::update(float time)
{
	AnimationClipNodeBase* animationClipNode = static_cast<AnimationClipNodeBase*>(m_animationNode);
	if (!animationClipNode->getLooping())
	{
		float totalDuration = animationClipNode->getTotalDuration();
		if (time > m_startTime + totalDuration)
			time = m_startTime + totalDuration;
		else if (time < m_startTime)
			time = m_startTime;
	}

	if (m_time != time - m_startTime)
		updateTime(time);
}

void AnimationClipState::phase(float value)
{
	float time = value * static_cast<AnimationClipNodeBase*>(m_animationNode)->getTotalDuration() + m_startTime;
	if (m_time != time - m_startTime)
		updateTime(time);
}

void AnimationClipState::updateTime(float time)
{
	m_framesDirty = true;
	m_timeDir = time - m_startTime > m_time;
	AnimationStateBase::updateTime(time);
}

void AnimationClipState::updateFrames()
{
	m_framesDirty = false;

	AnimationClipNodeBase* animationClipNode = static_cast<AnimationClipNodeBase*>(m_animationNode);
	bool looping = animationClipNode->getLooping();
	float totalDuration = animationClipNode->getTotalDuration();
	unsigned int lastFrame = animationClipNode->getLastFrame();
	float time = m_time;

	if (looping && (time >= totalDuration || time < 0))
	{
		time = std::fmod(time, totalDuration);
		if (time < 0)
			time += totalDuration;
	}

	if (!looping && time >= totalDuration)
	{
		//notifyPlaybackComplete();
		m_currentFrame = lastFrame;
		m_nextFrame = lastFrame;
		m_blendWeight = 0;
	}
	else if (!looping && time <= 0)
	{
		m_currentFrame = 0;
		m_nextFrame = 0;
		m_blendWeight = 0;
	}
	else if (animationClipNode->m_fixedFrameRate)
	{
		float t = time / totalDuration * lastFrame;
		m_currentFrame = (unsigned int)t;
		m_blendWeight = t - m_currentFrame;
		m_nextFrame = m_currentFrame + 1;
	}
	else
	{
		m_currentFrame = 0;
		m_nextFrame = 0;

		float dur = 0, frameTime;
		std::vector<float>& durations = animationClipNode->getDurations();

		do
		{
			frameTime = dur;
			dur += durations[getNextFrame()];
			m_currentFrame = m_nextFrame++;
		} while (time > dur);

		if (m_currentFrame == lastFrame)
		{
			m_currentFrame = 0;
			m_nextFrame = 1;
		}

		m_blendWeight = (time - frameTime) / durations[m_currentFrame];
	}
}