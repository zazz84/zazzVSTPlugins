#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class CombFilter
{
public:
	CombFilter();

	void init(int size) { m_buffer.init(size); };
	void clear() { m_buffer.clear(); };
	void setSize(int size) { m_buffer.setSize(size); };
	void setFeedback(float feedback) { m_feedback = feedback; }
	float process(float in);

protected:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};

//==============================================================================
class Allpass : public CombFilter
{
public:
	Allpass();

	float process(float in);
};

//==============================================================================
class NestedCombFilter : public CombFilter
{
public:
	NestedCombFilter();

	void init(int combFilterSize, int allPassSize, int sampleRate)
	{ 
		m_buffer.init(combFilterSize);
		m_allPass.init(allPassSize);
		m_filter.init(sampleRate);
	};
	void setSize(int combFilterSize, int allPassSize)
	{ 
		m_buffer.setSize(combFilterSize); 
		m_allPass.setSize(allPassSize);
	};
	void setFeedback(float combFilterFeedback, float allPassFeedback)
	{ 
		m_feedback = combFilterFeedback;
		m_allPass.setFeedback(allPassFeedback);
	};
	void setDamping(float damping)
	{
		m_filter.setHighShelf(2000.0f, 0.7f, damping * -12.0f);
	};

	float process(float in);

protected:
	Allpass m_allPass;
	BiquadFilter m_filter;
};

//==============================================================================
class CircularCombFilter
{
public:
	CircularCombFilter();

	void init(int complexity, int* size);
	void setSize(const int* size);
	void setFeedback(const float* feedback);

	float process(float in);

protected:
	CircularBuffer* m_buffer;
	float* m_feedback;
	int m_complexity = 0;
};

//==============================================================================
class CircularCombFilterAdvanced : public CircularCombFilter
{
public:
	CircularCombFilterAdvanced();

	void setDampingFrequency(const float* dampingFrequency);
	void setAllPassSize(const int* size);
	void setAllPassFeedback(const float* feedback);

	void init(int complexity, int* size, int* allPassSize);
	float process(float in);

private:
	BiquadFilter* m_filter;
	Allpass* m_allPass;
};