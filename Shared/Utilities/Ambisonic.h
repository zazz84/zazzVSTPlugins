/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cmath>
#include <array>
#include <functional>

namespace Ambisonic
{

#define DEG2RAD 0.017453292519943295f;

	constexpr float speakers50Deg[5] =	{
											 -30.0f,  // Left
											  30.0f,  // Right
											   0.0f,  // Center
											-110.0f,  // Left Surround
											 110.0f   // Right Surround
										};

	// IEM / AmbiX / SPARTA Typical Coefficients (Empirical/Optimized)
	/*constexpr float decoderMatrix[5][4] = {
		//   W       X       Y       Z
		{ 0.707f,  0.866f, -0.500f,  0.0f },  // Left
		{ 0.707f,  0.866f,  0.500f,  0.0f },  // Right
		{ 0.707f,  1.000f,  0.000f,  0.0f },  // Center
		{ 0.707f, -0.342f, -0.940f,  0.0f },  // Left Surround
		{ 0.707f, -0.342f,  0.940f,  0.0f }   // Right Surround
	};*/

	struct BFormat
	{
		float W = 0.0;
		float X = 0.0;
		float Y = 0.0;
		float Z = 0.0;
	};

	inline void encodeToAmbisonics(float in, float azimuthDeg, float elevationDeg, BFormat& out, const float gain = 1.0f)
	{
		const auto azimuthRad = azimuthDeg * DEG2RAD;
		const auto elevationRad = elevationDeg * DEG2RAD;

		auto tmp = in * gain;
		out.W += tmp;
		out.Z += tmp * std::sin(elevationRad);
		tmp *=  std::cos(elevationRad);
		out.X += tmp * std::cos(azimuthRad) ;
		out.Y += tmp * std::sin(azimuthRad);
	};

	inline void encodeToAmbisonics2D(float in, float azimuthDeg, BFormat& out, const float gain = 1.0f)
	{
		const auto azimuthRad = azimuthDeg * DEG2RAD;

		const auto tmp = in * gain;
		out.W += tmp;
		out.X += tmp * std::cos(azimuthRad);
		out.Y += tmp * std::sin(azimuthRad);
	};

	inline float decodeToSpeaker(BFormat& in, float azimuthDeg, float elevationDeg)
	{	
		const auto azimuthRad = azimuthDeg * DEG2RAD;
		const auto elevationRad = azimuthDeg * DEG2RAD;

		const float cosAz = std::cos(azimuthRad);
		const float sinAz = std::sin(azimuthRad);
		const float cosEl = std::cos(elevationRad);
		const float sinEl = std::sin(elevationRad);

		return 0.707f * in.W + in.X * cosAz * cosEl + in.Y * sinAz * cosEl + in.Z * sinEl;
	};

	inline float decodeToSpeaker2D(BFormat& in, float azimuthDeg)
	{
		const auto azimuthRad = azimuthDeg * DEG2RAD;

		const float cosAz = std::cos(azimuthRad);
		const float sinAz = std::sin(azimuthRad);

		return 0.707f * in.W + in.X * cosAz + in.Y * sinAz;
	};
}