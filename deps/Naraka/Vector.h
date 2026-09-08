#pragma once
#include <cmath>
#define CHECK_VALID( _v ) 0
#define Assert(x)

#include <stdlib.h>

#define Assert( _exp ) ((void)0)

#define FastSqrt(x)			(sqrt)(x)

#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

#define M_PHI		1.61803398874989484820 // golden ratio

// NJS: Inlined to prevent floats from being autopromoted to doubles, as with the old system.
#ifndef RAD2DEG
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#endif

#ifndef DEG2RAD
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#endif


enum
{
	PITCH = 0,	// up / down
	YAW,		// left / right
	ROLL		// fall over
};

// decls for aligning data

#define DECL_ALIGN(x) __attribute__((aligned(x)))


#define ALIGN16 DECL_ALIGN(16)
#define VALVE_RAND_MAX 0x7fff
#define VectorExpand(v) (v).x, (v).y, (v).z

typedef struct _D3DMATRIXEx {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};
} D3DMATRIXEx;

struct matrix3x4_t
{
	
	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	float* operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	const float* operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	float* Base() { return &m_flMatVal[0][0]; }
	const float* Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};

class VMatrix
{
public:

	
	VMatrix(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	) {
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;

		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;

		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;

		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
	}



	// array access
	inline float* operator[](int i)
	{
		return m[i];
	}

	inline const float* operator[](int i) const
	{
		return m[i];
	}

	// Get a pointer to m[0][0]
	inline float* Base()
	{
		return &m[0][0];
	}

	inline const float* Base() const
	{
		return &m[0][0];
	}


public:
	// The matrix.
	float		m[4][4];
};


class Vector3 {
public:
	// 数据成员
	float x, y, z;

	// ================= 构造函数 =================
	constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr Vector3(float x_, float y_, float z_) noexcept
		: x(x_), y(y_), z(z_) {}

	// ================= 运算符重载 =================
	constexpr Vector3 operator+(const Vector3& rhs) const noexcept {
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}

	constexpr Vector3 operator-(const Vector3& rhs) const noexcept {
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	constexpr Vector3 operator*(float scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar };
	}

	constexpr Vector3 operator/(float scalar) const {
		if (scalar == 0) 
			return { 0.0f, 0.0f, 0.0f };
		return { x / scalar, y / scalar, z / scalar };
	}


	// 计算向量长度平方（性能优化用）
	constexpr float LengthSq() const noexcept {
		return x * x + y * y + z * z;
	}
	/// @brief 计算到另一个向量的距离
	float DistTo(const Vector3& other) const noexcept {
		return (*this - other).Length();
	}

	/// @brief 计算向量长度
	float Length() const noexcept {
		return std::sqrt(LengthSq());
	}

	/// @brief 点积计算
	constexpr float Dot(const Vector3& other) const noexcept {
		return x * other.x + y * other.y + z * other.z;
	}

	/// @brief 叉积计算
	constexpr Vector3 Cross(const Vector3& other) const noexcept {
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		};
	}


};

class Vector2
{
public:
	float x, y;
	Vector2(void);
	Vector2(float X, float Y);
	bool IsValid() const;

};
inline unsigned long& FloatBits(float& f)
{
	return *reinterpret_cast<unsigned long*>(&f);
}
inline bool IsFinite(float f)
{
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}
inline bool Vector2::IsValid() const
{
	return IsFinite(x) && IsFinite(y);
}
inline Vector2::Vector2(void)
{
#ifdef _DEBUG
	// Initialize to NAN to catch errors
	//x = y = float_NAN;
#endif
	x = y = 0.0f;
}
inline Vector2::Vector2(float X, float Y)
{
	x = X; y = Y;
	Assert(IsValid());
}





class Vector4
{
public:
	Vector4() : x(0.f), y(0.f), z(0.f), w(0.f) {}

	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

	float x;
	float y;
	float z;
	float w;


};

