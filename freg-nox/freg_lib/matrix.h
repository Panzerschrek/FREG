	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#ifndef MATRIX_H
#define MATRIX_H

#include "vec.h"

class m_Mat4
{
public:
	float value[16];

	m_Mat4	operator*( m_Mat4& m );//���������
	m_Mat4&	operator*=( m_Mat4& m );//����������
	m_Vec3		operator*( m_Vec3& v );

	float&	operator[]( int i );
	float	operator[]( int i )const;//�������� ��������������

	 m_Mat4(){}
	~m_Mat4(){}
	void	Transpose();

	void Translate( m_Vec3& v);//���������� ������� �� ������� �����������
	void Scale( m_Vec3& v);//���������� ������� �� ������� ���������������
	void Scale( float s );//���������� ������� �� ������� ���������������


	void RotateX( float a );//�������� �� ������ ������� ������� �������� �� 3-� ����
	void RotateY( float a );
	void RotateZ( float a );
	void MakePerspective( float aspect, float fov_y, float z_near, float z_far);//������� ��� ������� �������� �����������
	void MakeProjection( float scale_x, float scale_y, float z_near, float z_far );
	void Identity();//������ ������� ���������

	/*True matrix - this
	0	1	2	3
	4	5	6	7
	8	9	10	11
	12	13	14	15
	*/

	/*OpenGL matrix
	0	4	8	12
	1	5	9	13
	2	6	10	14
	3	7	11	15
	*/
};

m_Vec3 operator*( m_Vec3& v, m_Mat4& m );

#endif//_MATRIX_H_
