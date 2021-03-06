/*****************************************************************//**
 * \file   Camera.h
 *
 * \date   September 2021
 *********************************************************************/

#pragma once

#include "ModernOpenGL/Shader.h"
#include "Transform.h"


	/**
	 * @brief Data on a camera to view the game from
	*/
struct Camera
{
		/**
		* @brief Defines whether the camera is operating in perspective or orthographic mode
		*/
	enum class CameraProjection
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

		/**
			* @brief The field of view of the camera, ie the angle of its width
		*/
	float FOV;

		/**
		 * @brief The near clipping frame, which objects will not be rendered in front of
		*/
	float NearClip;

		/**
		 * @brief The far clipping frame, which objects will not be rendered behind
		*/
	float FarClip;

		/**
		 * @brief Whether the camera is perspective or orthographic
		*/
	CameraProjection Projection;

		/**
		 * @brief default constructor
		*/
	Camera();
};
