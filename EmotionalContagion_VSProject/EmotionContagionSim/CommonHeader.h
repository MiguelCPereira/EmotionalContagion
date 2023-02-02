#pragma once
#include <cmath>
#include <algorithm>

const float PI = 3.14159265359f;

struct Vector2f
{
	Vector2f() : X(0.f), Y(0.f) {}
	Vector2f(float x, float y) : X(x), Y(y) {}

	Vector2f operator+(const Vector2f& otherVec) const { return Vector2f(X + otherVec.X, Y + otherVec.Y); }
	Vector2f operator-(const Vector2f& otherVec) const { return Vector2f(X - otherVec.X, Y - otherVec.Y); }
	Vector2f operator*(float f) const { return Vector2f(X * f, Y * f); }
	Vector2f operator/(float f) const { return Vector2f(X / f, Y / f); }

	void operator+=(const Vector2f& otherVec) { X += otherVec.X; Y += otherVec.Y; }
	void operator-=(const Vector2f& otherVec) { X -= otherVec.X; Y -= otherVec.Y; }
	void operator*=(float f) { X *= f; Y *= f; }
	void operator/=(float f) { X /= f; Y /= f; }

	bool operator==(const Vector2f& otherVec) const { return otherVec.X == X && otherVec.Y == Y; }
	bool operator!=(const Vector2f& otherVec) const { return otherVec.X != X || otherVec.Y != Y; }

	bool NearEqual(const Vector2f& otherVec, float errorMargin = 0.001f) const
	{
		return (X - otherVec.X > -errorMargin && (X - otherVec.X) < errorMargin) &&
			(Y - otherVec.Y > -errorMargin && (Y - otherVec.Y) < errorMargin);
	}

	bool IsWithinRectArea(Vector2f bottomLeft, Vector2f size) const
	{
		if (X >= bottomLeft.X && X <= bottomLeft.X + size.X)
			return Y >= bottomLeft.Y && Y <= bottomLeft.Y + size.Y;

		return false;
	}

	void Normalize()
	{
		const auto m = std::sqrtf(std::powf(X, 2.f) + std::powf(Y, 2.f));
		if (m > 0.f)
		{
			X /= m;
			Y /= m;
		}
	}

	float Dot(Vector2f otherVec) const
	{
		return  X * otherVec.X + Y * otherVec.Y;
	}

	float Determinant(Vector2f otherVec) const
	{
		return X * otherVec.Y - Y * otherVec.X;
	}

	float Distance(Vector2f otherVec) const
	{
		auto distX = otherVec.X - X;
		if (distX < 0.f) distX *= -1.f;

		auto distY = otherVec.Y - Y;
		if (distY < 0.f) distY *= -1.f;

		return std::sqrtf(std::powf(distX, 2.f) + std::powf(distY, 2.f));
	}

	float Distance(Vector2f bottomLeft, Vector2f size, Vector2f& pointOfContact) const
	{
		const auto alignedVert = (X >= bottomLeft.X && X <= bottomLeft.X + size.X);
		const auto alignedHoriz = (Y >= bottomLeft.Y && Y <= bottomLeft.Y + size.Y);
		
		auto toTheBot = Y < bottomLeft.Y;
		auto toTheLeft = X < bottomLeft.X;

		if (alignedVert)
		{
			// Aligned In Both Axis
			if (alignedHoriz)
			{
				pointOfContact = Vector2f(X, Y);
				return 0.f;
			}

			// Aligned Only Vertically
			if (toTheBot)
			{
				pointOfContact = Vector2f(X, bottomLeft.Y);
				return bottomLeft.Y - Y;
			}

			pointOfContact = Vector2f(X, bottomLeft.Y + size.Y);
			return Y - (bottomLeft.Y + size.Y);
		}
		
		// Aligned Only Horizontally
		if (alignedHoriz)
		{
			if (toTheLeft)
			{
				pointOfContact = Vector2f(bottomLeft.X, Y);
				return bottomLeft.X - X;
			}

			pointOfContact = Vector2f(bottomLeft.X + size.X, Y);
			return X - (bottomLeft.X + size.X);
		}
		
		// Not Aligned In Any Axis
		if (toTheBot && toTheLeft)
		{
			pointOfContact = Vector2f(bottomLeft.X, bottomLeft.Y);
			return Distance(bottomLeft);
		}
		if (toTheBot && !toTheLeft)
		{
			pointOfContact = Vector2f(bottomLeft.X + size.X, bottomLeft.Y);
			return Distance(Vector2f(bottomLeft.X + size.X, bottomLeft.Y));
		}
		if (!toTheBot && toTheLeft)
		{
			pointOfContact = Vector2f(bottomLeft.X, bottomLeft.Y + size.Y);
			return Distance(Vector2f(bottomLeft.X, bottomLeft.Y + size.Y));
		}
		pointOfContact = Vector2f(bottomLeft.X + size.X, bottomLeft.Y + size.Y);
		return Distance(Vector2f(bottomLeft.X + size.X, bottomLeft.Y + size.Y));

	}

	bool IsWithinCircleArea(Vector2f circleCenter, float circleRadius) const
	{
		return Distance(circleCenter) <= circleRadius;
	}
	
	
	float X = 0.f;
	float Y = 0.f;
};

struct Quad
{
	Quad() : BottomLeft(), Size() {}
	Quad(Vector2f bottomLeft, Vector2f size) : BottomLeft(bottomLeft), Size(size) {}

	bool NearEqual(const Quad& otherQuad, float errorMargin = 0.001f) const
	{
		return BottomLeft.NearEqual(otherQuad.BottomLeft, errorMargin) && Size.NearEqual(otherQuad.Size, errorMargin);
	}

	bool IsOverlappingOtherRect(const Quad& otherQuad) const
	{
		return !(BottomLeft.X >= otherQuad.BottomLeft.X + otherQuad.Size.X ||
			BottomLeft.X + Size.X <= otherQuad.BottomLeft.X ||
			BottomLeft.Y >= otherQuad.BottomLeft.Y + otherQuad.Size.Y ||
			BottomLeft.Y + Size.Y <= otherQuad.BottomLeft.Y);
	}

	bool IsOverlappingCircle(Vector2f circlePosition, float circleRadius) const
	{
		// Get the nearest point to the circle
		const auto nearestPoint = Vector2f(std::fmax(BottomLeft.X, std::fmin(circlePosition.X, BottomLeft.X + Size.X)),
			std::fmax(BottomLeft.Y, std::fmin(circlePosition.Y, BottomLeft.Y + Size.Y)));

		const auto distToNearestPoint = circlePosition.Distance(nearestPoint);
		return distToNearestPoint <= circleRadius;
	}

	Vector2f BottomLeft;
	Vector2f Size;
};

struct Vector3f
{
	Vector3f() : X(0.f), Y(0.f), Z(0.f) {}
	Vector3f(float x, float y, float z) : X(x), Y(y), Z(z) {}

	Vector3f operator+(const Vector3f& otherVec) const { return Vector3f(X + otherVec.X, Y + otherVec.Y, Z + otherVec.Z); }
	Vector3f operator-(const Vector3f& otherVec) const { return Vector3f(X - otherVec.X, Y - otherVec.Y, Z - otherVec.Z); }
	Vector3f operator*(float f) const { return Vector3f(X * f, Y * f, Z * f); }
	Vector3f operator/(float f) const { return Vector3f(X / f, Y / f, Z / f); }

	void operator+=(const Vector3f& otherVec) { X += otherVec.X; Y += otherVec.Y; Z += otherVec.Z; }
	void operator-=(const Vector3f& otherVec) { X -= otherVec.X; Y -= otherVec.Y; Z -= otherVec.Z; }
	void operator*=(float f) { X *= f; Y *= f; Z *= f; }
	void operator/=(float f) { X /= f; Y /= f; Z /= f; }

	bool operator==(const Vector3f& otherVec) const { return otherVec.X == X && otherVec.Y == Y && otherVec.Z == Z; }
	bool operator!=(const Vector3f& otherVec) const { return otherVec.X != X || otherVec.Y != Y || otherVec.Z != Z; }

	bool NearEqual(const Vector3f& otherVec, float errorMargin = 0.001f) const
	{
		return (X - otherVec.X > -errorMargin && (X - otherVec.X) < errorMargin) &&
			(Y - otherVec.Y > -errorMargin && (Y - otherVec.Y) < errorMargin) &&
			(Z - otherVec.Z > -errorMargin && (Z - otherVec.Z) < errorMargin);
	}

	void Normalize()
	{
		const auto m = std::sqrtf(std::powf(X, 2.f) + std::powf(Y, 2.f) + std::powf(Z, 2.f));
		if (m > 0.f)
		{
			X /= m;
			Y /= m;
			Z /= m;
		}
	}

	float Distance(Vector3f otherVec) const
	{
		auto distX = otherVec.X - X;
		if (distX < 0.f) distX *= -1.f;

		auto distY = otherVec.Y - Y;
		if (distY < 0.f) distY *= -1.f;

		auto distZ = otherVec.Z - Z;
		if (distZ < 0.f) distZ *= -1.f;

		return std::sqrtf(std::powf(distX, 2.f) + std::powf(distY, 2.f) + std::powf(distZ, 2.f));
	}


	float X = 0.f;
	float Y = 0.f;
	float Z = 0.f;
};

struct OCEANProfile
{
	OCEANProfile() : Openess(0.f), Conscientiousness(0.f), Extraversion(0.f), Agreeableness(0.f), Neuroticism(0.f) {}
	OCEANProfile(float openess, float conscientiousness, float extraversion, float agreeableness, float neuroticism)
		: Openess(openess), Conscientiousness(conscientiousness), Extraversion(extraversion), Agreeableness(agreeableness), Neuroticism(neuroticism) {}

	float Openess = 0.f;
	float Conscientiousness = 0.f;
	float Extraversion = 0.f;
	float Agreeableness = 0.f;
	float Neuroticism = 0.f;
};

enum class MovementSpeed
{
	Slow = 0,
	Regular = 1,
	Sprint = 2
};

struct SteeringOutput
{
	SteeringOutput()
		: MovementDirection(Vector2f()), FacingDirection(Vector2f()), DesiredSpeed(MovementSpeed::Regular), DesiredSpeedAmount(1.f), IsValid(false) {}
	SteeringOutput(Vector2f movDir, Vector2f faceDir, MovementSpeed desiredSpeed, float desiredSpeedAmount, bool isValid)
		: MovementDirection(movDir), FacingDirection(faceDir), DesiredSpeed(desiredSpeed), DesiredSpeedAmount(desiredSpeedAmount), IsValid(isValid){}

	SteeringOutput& operator=(const SteeringOutput& other)
	{
		MovementDirection = other.MovementDirection;
		FacingDirection = other.FacingDirection;
		DesiredSpeed = other.DesiredSpeed;
		DesiredSpeedAmount = other.DesiredSpeedAmount;
		IsValid = other.IsValid;

		return *this;
	}

	SteeringOutput& operator+(const SteeringOutput& other)
	{
		MovementDirection += other.MovementDirection;
		MovementDirection.Normalize();
		FacingDirection += other.FacingDirection;
		FacingDirection.Normalize();
		if (int(DesiredSpeed) < int(other.DesiredSpeed)) DesiredSpeed = other.DesiredSpeed;
		DesiredSpeedAmount += other.DesiredSpeedAmount;
		DesiredSpeedAmount /= 2.f;
		IsValid = IsValid || other.IsValid;

		return *this;
	}

	SteeringOutput& operator-(const SteeringOutput& other)
	{
		MovementDirection -= other.MovementDirection;
		MovementDirection.Normalize();
		FacingDirection -= other.FacingDirection;
		FacingDirection.Normalize();
		if (int(DesiredSpeed) < int(other.DesiredSpeed)) DesiredSpeed = other.DesiredSpeed;
		DesiredSpeedAmount += other.DesiredSpeedAmount;
		DesiredSpeedAmount /= 2.f;
		IsValid = IsValid || other.IsValid;

		return *this;
	}

	Vector2f MovementDirection = { 0.f,0.f };
	Vector2f FacingDirection = { 0.f, 0.f };
	MovementSpeed DesiredSpeed = MovementSpeed::Regular;
	float DesiredSpeedAmount = 1.f;
	bool IsValid = true;
};
