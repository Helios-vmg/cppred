#pragma once

struct point2{
	int x, y;
	point2(int x = 0, int y = 0): x(x), y(y){}
	point2(const point2 &other){
		*this = other;
	}
	const point2 &operator=(const point2 &other){
		this->x = other.x;
		this->y = other.y;
		return *this;
	}
	const point2 &operator+=(const point2 &other){
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	const point2 &operator-=(const point2 &other){
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	point2 operator-() const{
		return point2(-this->x, -this->y);
	}
	const point2 &operator*=(int k){
		this->x *= k;
		this->y *= k;
		return *this;
	}
	const point2 &operator/=(int k){
		this->x /= k;
		this->y /= k;
		return *this;
	}
	point2 operator+(const point2 &other) const{
		auto ret = *this;
		ret += other;
		return ret;
	}
	point2 operator-(const point2 &other) const{
		auto ret = *this;
		ret -= other;
		return ret;
	}
	point2 operator*(int k) const{
		auto ret = *this;
		ret *= k;
		return ret;
	}
	point2 operator/(int k) const{
		auto ret = *this;
		ret /= k;
		return ret;
	}
};
