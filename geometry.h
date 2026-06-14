#pragma once
#include<cmath>
#include<cassert>
#include<iostream>
constexpr const double PI = 3.14159265358979323846;



template<int n> struct  vec
{
	double vdata[n] = { 0 };
	double& operator [](const int i) { assert(i >= 0 && i < n); return vdata[i]; }   //可读可写 //重载[].实现批量创建
	double operator [](const int i)const { assert(i >= 0 && i < n); return vdata[i]; }  //只读	
};



template<int n> vec<n> operator *(const vec<n>& vL, const double &val)
{
	vec<n> result = vL;
	for (int i = n; i--;result[i] *=val );
	return result;
}

template<int n> vec<n> operator *( const double& val,const vec<n>& vR)
{
	
	
	return vR*val;
}

template<int n> vec<n> operator /(const vec<n>& vL, const vec<n>& vR)
{
	vec<n> result = vL;
	for (int i = n; i--; result[i] /= vR[i]);
	return result;
}

template<int n> vec<n> operator/(const vec<n>& lhs, const double& rhs) {
	vec<n> ret = lhs;
	for (int i = n; i--; ret[i] /= rhs);
	return ret;
}

template<int n> vec<n> operator +(const vec<n>& vL, const vec<n>& vR)
{
	vec<n> result = vL;
	for (int i = n; i--; result[i] += vR[i]);
	
	return result;
}

template<int n> vec<n> operator -(const vec<n>& vL, const vec<n>& vR)
{
	vec<n> result = vL;
	for (int i = n; i--; result[i] -= vR[i]);
	
	return result;
}

template<int n> vec<n> operator~(const vec<n>& v)
{
	vec<n> r;
	for (int i = n; i--;)r[i] = 0 - v[i];
	return r;
}

template<int n> bool operator == (const vec<n>& vL, const vec<n>& vR)
{
	for (int i = n; i--;)
		if (vL[i] != vR[i]) return false;
	return true;
}


template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v)
{
	for (int i = 0; i < n; i++)
	{
		out << v[i] << " ";
		
	}
	
	return out;
}

template<int n> double norm(const vec<n>& v)
{
	return std::sqrt(dot(v ,v));
}

template<int n> vec<n> normalize(const vec<n> &v)
{
	vec<n> r;
	return r = v / norm(v);
}



template<> struct vec<2>
{
	double x = 0, y = 0;
	double& operator [](const int i) { assert(i >= 0 && i < 2); return i ? y : x; }
	double operator [](const int i) const { assert(i >= 0 && i < 2); return i ? y : x;}

};


template<> struct vec<3>
{
	double x = 0, y = 0, z = 0;
	double& operator [](const int i) { assert(i >= 0 && i < 3); return i ? (i == 1 ? y : z) : x; }
	double operator [](const int i) const { assert(i >= 0 && i < 3); return i ? (i == 1 ? y : z) : x; }

};


template<> struct vec<4>
{
	double x = 0, y = 0, z = 0 ,w=0;
	double& operator [](const int i) { assert(i >= 0 && i <4); return i < 2 ? (i ? y : x) : (2 == i ? z : w);}

	double operator [](const int i) const { assert(i >= 0 && i < 4); return i < 2 ? (i ? y : x) : (2 == i ? z : w);}
	vec<3> xyz() { return { x,y,z }; }
	vec<2> xy() { return { x,y }; }
};


typedef vec<3> vec3;
typedef vec<2> vec2;
typedef vec<4> vec4;



inline vec3 cross(const vec3& u, const vec3& v)
{
	return { u.y * v.z - u.z * v.y,     u.z*v.x  - u.x * v.z,        u.x * v.y - u.y *v.x  };
}

template<int n> inline double dot(const vec<n>& u, const vec<n>& v)
{
	double ret = 0;                         //点乘
	for (int i = n; i--; ret += u[i] * v[i]); //反式循环
	return ret;
}


template<int nrows> struct dt;


//row为行，colume为列,行优先存储

template<int nrows,int ncols > struct mat
{
	vec<ncols> rows[nrows]{ {} };
	vec<ncols>& operator [](const int rIndex) { assert(rIndex >= 0 && rIndex < nrows); return rows[rIndex]; }
	const vec<ncols>& operator [](const int rIndex) const{ assert(rIndex >= 0 && rIndex < nrows); return rows[rIndex]; }


	double det() const
	{
		return dt<ncols>::det(*this);
	}

	double cofactor(const int r, const int c) const //代数余子式的计算
	{
		mat<nrows - 1, ncols - 1> subMat;
		
		for (int i = nrows - 1; i--;)
			for (int j = ncols - 1; j--; subMat[i][j] = rows[i + int(i >= r)][j + int(j >= c)]);//子矩阵的建立

		return subMat.det() * ((r + c) % 2 ? -1 : 1);
	}


	mat<nrows, ncols> invert_transpose()const 
	{
		mat< nrows, ncols> adjugate_NoTranspose;
		for (int i = nrows; i--;)   //在使用该循环格式时，是先判断非0，再减，再执行后面语句
			for (int j = ncols; j--; adjugate_NoTranspose[i][j] = cofactor(i,j)); //伴随由余子式构成的矩阵求转置得到
		double det = dot(adjugate_NoTranspose[0], rows[0]); // 这仅在 nrows=1 时正确，否则错误！
		// 正确应使用 dt<nrows>::det(*this)
		det = dt<nrows>::det(*this);
		return adjugate_NoTranspose / det;
		
	}


	mat<ncols, nrows> transpose()const 
	{
		mat<ncols,nrows > tMat;
		for (int i = ncols; i--;)
			for (int j = nrows; j--; tMat[i][j] = rows[j][i]);
		return tMat;
	}

	mat<nrows, ncols> invert()const
	{
		return invert_transpose().transpose();
	}


};


template<int r, int c1, int c2> mat<r, c2> operator*(const mat<r, c1> m1, const mat<c1, c2> m2)
{
	mat<r, c2> r1;
	for (int i = r; i--;)
		for (int j = c2; j--;)
			for (int k = c1; k--; r1[i][j]+= m1[i][k] * m2[k][j]);
	return r1;
}

template<int nrows, int ncols> vec<ncols> operator *(const vec<nrows>& v, const mat<nrows, ncols>& m)//行向量乘以行优先排列的列矩阵
{
	return (mat<1, nrows>{{v}}*m)[0];
}

template<int nrows, int ncols> vec<nrows> operator*(const mat<nrows, ncols>& m, const vec<ncols>& v)//行优先排列的行矩阵乘以列向量
{
	vec<nrows> r;
	for (int i = nrows; i--; r[i] = dot(m[i] ,v));
	return r;
}


template<int nrows, int ncols>mat<nrows, ncols> operator/(const mat<nrows, ncols>& lhs, const double& val) {
	mat<nrows, ncols> result;
	for (int i = nrows; i--; result[i] = lhs[i] / val);
	return result;
}

template<int nrows, int ncols>mat<nrows, ncols> operator+(const mat<nrows, ncols>& lhs, const double& val) {
	mat<nrows, ncols> result;
	for (int i = nrows; i--; result[i] = lhs[i] + val);
	return result;
}

template<int nrows, int ncols>mat<nrows, ncols> operator+(const mat<nrows, ncols>& lhs, const mat<nrows, ncols>& rhs) {
	mat<nrows, ncols> result;
	for (int i = nrows; i--; )
		for (int j = ncols; j--; result[i][j] = lhs[i][j] + rhs[i][j]);
	return result;
}

template<int nrows, int ncols>mat<nrows, ncols> operator-(const mat<nrows, ncols>& lhs, const mat<nrows, ncols>& rhs) {
	mat<nrows, ncols> result;
	for (int i = nrows; i--; )
		for (int j = ncols; j--; result[i][j] = lhs[i][j] - rhs[i][j]);
	return result;
}

template<int nrows, int ncols> std::ostream& operator<<(std::ostream& out, const mat<nrows, ncols>& m) {
	for (int i = 0; i < nrows; i++) out << m[i] << std::endl;
	return out;
}

template<int nrows> struct dt
{
	static double det(const mat<nrows, nrows>& m)
	{
		double r = 0.;
		for (int j = nrows; j--; r += m[0][j] * m.cofactor(0, j)); //行列式的计算，逐层计算，最后累加

		return r;
	}
};

template<> struct dt<1>
{
	static double det(const mat<1, 1>& sr)
	{

		return sr[0][0];
	}
};

