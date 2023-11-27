#pragma once

#include <ostream>
#include <utility>

#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define CLAMP_EQ(x, a, b) (x) = CLAMP(x, a, b)

struct point
{
	int x, y;
};

inline point operator -(const point& a) { return { -a.x, -a.y }; }

inline point operator ~(const point& a) { return { a.y, a.x }; }

inline std::ostream& operator <<(std::ostream& os, const point& p)
{
	os << '(' << p.x << ',' << ' ' << p.y << ')';
	return os;
}

#define OPERATOR_POINT_ARITHMETIC(SYMBOL) inline point operator SYMBOL(const point& a, const point& b) { return { a.x SYMBOL b.x, a.y SYMBOL b.y }; }
#define OPERATOR_INT_ARITHMETIC(SYMBOL) inline point operator SYMBOL(const point& a, const int& b) { return { a.x SYMBOL b, a.y SYMBOL b }; }
#define OPERATOR_BOOL(SYMBOL, JUNCTION) inline bool operator SYMBOL(const point& a, const point& b) { return a.x SYMBOL b.x JUNCTION a.y SYMBOL b.y; }
#define OPERATOR_POINT_VOID(SYMBOL) inline void operator SYMBOL(point& a, const point& b) { a.x SYMBOL b.x; a.y SYMBOL b.y; }
#define OPERATOR_INT_VOID(SYMBOL) inline void operator SYMBOL(point& a, const int& b) { a.x SYMBOL b; a.y SYMBOL b; }

OPERATOR_POINT_ARITHMETIC(+)
OPERATOR_POINT_ARITHMETIC(-)
OPERATOR_POINT_ARITHMETIC(*)
OPERATOR_POINT_ARITHMETIC(/)
OPERATOR_POINT_ARITHMETIC(%)

OPERATOR_INT_ARITHMETIC(+)
OPERATOR_INT_ARITHMETIC(-)
OPERATOR_INT_ARITHMETIC(*)
OPERATOR_INT_ARITHMETIC(/)
OPERATOR_INT_ARITHMETIC(%)
OPERATOR_INT_ARITHMETIC(&)
OPERATOR_INT_ARITHMETIC(|)
OPERATOR_INT_ARITHMETIC(<<)
OPERATOR_INT_ARITHMETIC(>>)

OPERATOR_BOOL(==, &&)
OPERATOR_BOOL(!=, ||)
OPERATOR_BOOL(<, &&)
OPERATOR_BOOL(>, &&)
OPERATOR_BOOL(<=, &&)
OPERATOR_BOOL(>=, &&)

OPERATOR_POINT_VOID(+=)
OPERATOR_POINT_VOID(-=)
OPERATOR_INT_VOID(+=)
OPERATOR_INT_VOID(-=)
OPERATOR_INT_VOID(<<=)
OPERATOR_INT_VOID(>>=)

inline void clamp_point(point& p, const point& a, const point& b)
{
	CLAMP_EQ(p.x, a.x, b.x);
	CLAMP_EQ(p.y, a.x, b.x);
}

inline void swap_point_if(point& s, point& b)
{
	if (s.x > b.x) std::swap(s.x, b.x);
	if (s.y > b.y) std::swap(s.y, b.y);
}
