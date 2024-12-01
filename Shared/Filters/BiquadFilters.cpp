#include "BiquadFilters.h"
#include <cmath>

#define M_LN2 0.69314718f
#define M_PI  3.14159268f
#define M_PI2 6.28318536f
#define DN    1.0e-20f

void BiquadFilter::setLowPass(float frequency, float Q)
{
    const float omega = M_PI2 * frequency / m_SampleRate;
    const float sn = sin(omega);
    const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);

	const float tmp = 1.0f - cs;
	b0 = tmp * 0.5f;
    b1 = tmp;
    b2 = b0;
    a0 = 1.0f + alpha;
    a1 = -2.0f * cs;
    a2 = 1.0f - alpha;

    normalize();
}

void BiquadFilter::setHighPass(float frequency, float Q)
{
    const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);

	const float tmp = 1 + cs;
    b0 = tmp * 0.5f;
    b1 = -tmp;
    b2 = b0;
    a0 = 1.0f + alpha;
    a1 = -2.0f * cs;
    a2 = 1.0f - alpha;
	
	normalize();
}

void BiquadFilter::setBandPassSkirtGain(float frequency, float Q)
{
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);

    b0 = Q * alpha;
    b1 = 0.0f;
    b2 = -Q * alpha;
    a0 = 1.0f + alpha;
    a1 = -2.0f * cs;
    a2 = 1.0f - alpha;

	normalize();
}

void BiquadFilter::setBandPassPeakGain(float frequency, float Q)
{
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);

	b0 = alpha;
	b1 = 0.0f;
	b2 = -alpha;
	a0 = 1.0f + alpha;
	a1 = -2.0f * cs;
	a2 = 1.0f - alpha;

	normalize();
}

void BiquadFilter::setNotch(float frequency, float Q)
{
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);
 
 	b0 = 1.0f;
    b1 = -2.0f * cs;
    b2 = 1.0f;
    a0 = 1.0f + alpha;
    a1 = -2.0f * cs;
    a2 = 1.0f - alpha;

	normalize();
}  

void BiquadFilter::setPeak(float frequency, float Q, float gain)
{
	const float A = powf(10.0f, gain / 40.0f);
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);
	const float beta = sqrt(A + A);

	float tmp = alpha * A;
    b0 = 1.0f + tmp;
    b1 = -2.0f * cs;
    b2 = 1.0f - tmp;
	tmp = alpha / A;
    a0 = 1.0f + tmp;
    a1 = -2.0f * cs;
    a2 = 1.0f - tmp;

	normalize();
}

void BiquadFilter::setLowShelf(float frequency, float Q, float gain)
{
	const float A = powf(10.0f, gain / 40.0f);
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);
	const float beta = sqrt(A + A);

	float tmp1 = A + 1.0f;
	float tmp2 = A - 1.0f;
	b0 = A * (tmp1 - tmp2 * cs + beta * sn);
    b1 = 2 * A * (tmp2 - tmp1 * cs);
    b2 = A * (tmp1 - tmp2 * cs - beta * sn);
    a0 = tmp1 + tmp2 * cs + beta * sn;
    a1 = -2 * (tmp2 + tmp1 * cs);
    a2 = tmp1 + tmp2 * cs - beta * sn;

	normalize();
}

void BiquadFilter::setHighShelf(float frequency, float Q, float gain)
{
	const float A = powf(10.0f, gain / 40.0f);
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);
	const float beta = sqrt(A + A);

	float tmp1 = A + 1.0f;
	float tmp2 = A - 1.0f;
    b0 = A * (tmp1 + tmp2 * cs + beta * sn);
    b1 = -2 * A * (tmp2 + tmp1 * cs);
    b2 = A * (tmp1 + tmp2 * cs - beta * sn);
    a0 = tmp1 - tmp2 * cs + beta * sn;
    a1 = 2 * (tmp2 - tmp1 * cs);
    a2 = tmp1 - tmp2 * cs - beta * sn;

	normalize();
}

void BiquadFilter::normalize()
{
   b0 = b0 / a0;
   b1 = b1 / a0;
   b2 = b2 / a0;
   a1 = a1 / a0;
   a2 = a2 / a0;
}

float BiquadFilter::processDF1(float in)
{
    float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    x2 = x1;
    x1 = in;

    y2 = y1;
    y1 = out;

    return out;
}

float BiquadFilter::processDF2(float in)
{
	const float v   =     in - a1 * y1 - a2 * y2 + DN;
	const float out = b0 * v + b1 * y1 + b2 * y2;
	
	y2 = y1;
	y1 = v;

	return out;
}

float BiquadFilter::processDF1T(float in)
{
	float v = in + y2;
	float out = b0 * v + x2;

	x2 = b1 * v + x1;
	y2 = -a1 * v + y1;
	x1 = b2 * v;
	y1 = -a2 * v;

	return out;
}

float BiquadFilter::processDF2T(float in)
{
	float out = b0 * in + x2;
	
	/*x2 = b1 * in + x1 + a1 * out;
	x1 = b2 * in + a2 * out;*/

	x2 = b1 * in + x1 - a1 * out;
	x1 = b2 * in - a2 * out;

	return out;
}

//==============================================================================
/*void BiquadFilterSIMD::setLowPass(float frequency, float Q)
{
	const float omega = M_PI2 * frequency / m_SampleRate;
	const float sn = sin(omega);
	const float cs = cos(omega);
	const float alpha = sn / (2.0f * Q);

	const float tmp = 1.0f - cs;
	float b0 = tmp * 0.5f;
	float b1 = tmp;
	float b2 = b0;
	float a0 = 1.0f + alpha;
	float a1 = -2.0f * cs;
	float a2 = 1.0f - alpha;

	// Normalize
	b0 = b0 / a0;
	b1 = b1 / a0;
	b2 = b2 / a0;
	a1 = a1 / a0;
	a2 = a2 / a0;

	// Set member variables
	m_b0 = _mm256_set1_ps(b0);
	m_b1 = _mm256_set1_ps(b1);
	m_b2 = _mm256_set1_ps(b2);
	m_a1 = _mm256_set1_ps(a1);
	m_a2 = _mm256_set1_ps(a2);
	m_x1 = m_x2 = m_y1 = m_y2 = _mm256_setzero_ps();
}

__m256 BiquadFilterSIMD::processDF1(__m256 in)
{
	// Compute the output for the SIMD vector
	__m256 out = _mm256_sub_ps(
		_mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(m_b0, in), _mm256_mul_ps(m_b1, m_x1)),
			_mm256_mul_ps(m_b2, m_x2)),
		_mm256_add_ps(_mm256_mul_ps(m_a1, m_y1), _mm256_mul_ps(m_a2, m_y2))
	);

	// Update history
	m_x2 = m_x1;
	m_x1 = in;

	m_y2 = m_y1;
	m_y1 = out;

	return out;
}*/

//==============================================================================
LinkwitzRileySecondOrder::LinkwitzRileySecondOrder()
{
}

void LinkwitzRileySecondOrder::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void LinkwitzRileySecondOrder::setFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float fpi = M_PI * frequency;
	const float wc = 2.0f * fpi;
	const float wc2 = wc * wc;
	const float wc22 = 2.0f * wc2;
	const float k = wc / tanf(fpi / m_SampleRate);
	const float k2 = k * k;
	const float k22 = 2 * k2;
	const float wck2 = 2 * wc * k;
	const float tmpk = k2 + wc2 + wck2;
	
	m_b1 = (-k22 + wc22) / tmpk;
	m_b2 = (-wck2 + k2 + wc2) / tmpk;
	
	//---------------
	// low-pass
	//---------------
	m_a0_lp = wc2 / tmpk;
	m_a1_lp = wc22 / tmpk;
	m_a2_lp = wc2 / tmpk;
	
	//----------------
	// high-pass
	//----------------
	m_a0_hp = k2 / tmpk;
	m_a1_hp = -k22 / tmpk;
	m_a2_hp = k2 / tmpk;	
}

float LinkwitzRileySecondOrder::processLP(float in)
{
	const float y0 = m_a0_lp * in + m_x0_lp;
	m_x0_lp = m_a1_lp * in - m_b1 * y0 + m_x1_lp;
	m_x1_lp = m_a2_lp * in - m_b2 * y0;

	return y0;
}

float LinkwitzRileySecondOrder::processHP(float in)
{
	const float y0 = m_a0_hp * in + m_x0_hp;
	m_x0_hp = m_a1_hp * in - m_b1 * y0 + m_x1_hp;
	m_x1_hp = m_a2_hp * in - m_b2 * y0;

	return -y0;
}