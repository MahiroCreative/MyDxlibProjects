#include "Camera.h"

Camera::Camera()
	: m_x(0)
	, m_y(0)
{
}

int Camera::WorldToScreenX(int worldX) const
{
	return worldX - m_x;
}

int Camera::WorldToScreenY(int worldY) const
{
	return worldY - m_y;
}

int Camera::ScreenToWorldX(int screenX) const
{
	return screenX + m_x;
}

int Camera::ScreenToWorldY(int screenY) const
{
	return screenY + m_y;
}
