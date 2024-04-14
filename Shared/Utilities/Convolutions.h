#include "CircularBuffers.h"
#include <vector>

class Convolution
{
public:
	Convolution();

	void init(int size);
	void clear();
	void setImpulseResponse(std::vector<float> impulseResponse);
	inline void setImpulseResponseLenght(int impulseResponseLenght) { m_impulseResponseSize = (impulseResponseLenght > m_bufferSize) ? m_bufferSize : impulseResponseLenght; };
	float process(float in);
	
protected:
	CircularBuffer m_buffer = {};
	int m_bufferSize = 0;
	std::vector<float> m_impulseResponse;
	int m_impulseResponseSize = 0;
};