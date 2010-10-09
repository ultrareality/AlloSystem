#ifndef INCLUDE_AL_COORDINATEFRAME_HPP
#define INCLUDE_AL_COORDINATEFRAME_HPP

#include "math/al_Vec.hpp"
#include "math/al_Quat.hpp"


namespace al {

///	A coordinate frame

///	Combines a Vec3d position with a Quat orientation
///
class Pose {
public:
	Pose(const Vec3d& v=Vec3d(0), const Quatd& q=Quatd::identity())
	:	mVec(v), mQuat(q)
	{}

	Pose operator* (const Pose& v) const { return Pose(*this)*=v; }

	/// Translate and rotate by argument
	Pose& operator*=(const Pose& v){
		mVec += v.vec();
		mQuat*= v.quat();
		return *this;
	}


	/// Get "position" vector
	const Vec3d& pos() const { return mVec; }
	
	/// Get vector component
	const Vec3d& vec() const { return mVec; }

	/// Get quaternion component
	const Quatd& quat() const { return mQuat; }

	double x() const { return mVec[0]; }
	double y() const { return mVec[1]; }
	double z() const { return mVec[2]; }

	
	/// Set position
	template <class T>
	Pose& pos(const Vec3<T>& v){ return vec(v); }
	
	/// Set position from individual components
	Pose& pos(double x, double y, double z) { return vec(Vec3d(x,y,z)); }

	/// Set vector component
	template <class T>
	Pose& vec(const Vec3<T>& v){ mVec.set(v); return *this; }

	/// Set quaternion component
	template <class T>
	Pose& quat(const Quat<T>& v){
		mQuat[0]=v[0]; mQuat[1]=v[1]; mQuat[2]=v[2]; mQuat[3]=v[3];
		return *this;
	}

	Vec3d& pos(){ return mVec; }
	Vec3d& vec(){ return mVec; }
	Quatd& quat(){ return mQuat; }

	/// Get right, up, and forward unit vectors
	template <class T>
	void unitVectors(Vec3<T>& ur, Vec3<T>& uu, Vec3<T>& uf){
		quat().normalize();
		quat().toVectorX(ur);
		quat().toVectorY(uu);
		quat().toVectorZ(uf);	
	}

	/// Set state from another Pose
	Pose& set(const Pose& v){ mVec=v.vec(); mQuat=v.quat(); return *this; }
	
	// get the azimuth, elevation & distance from this to another point
	void toAED(const Vec3d& to, double& azimuth, double& elevation, double& distance) const;

	// TODO: conversion operators for Pose->Vec3d, Pose->Quatd?	
protected:
	Vec3d mVec;		// position in 3-space
	Quatd mQuat;	// orientation of reference frame as a quaternion (relative to global axes)
};


///	A mobile coordinate frame

///	A Pose that knows how to accumulate velocities
/// Smooth navigation with adjustable linear and angular velocity
class Nav : public Pose {
public:

	Nav(const Vec3d &position = Vec3d(0), double smooth=0)
	:	Pose(position), mSmooth(smooth)
	{	updateUnitVectors(); }

	/// Get smoothing amount
	double smooth() const { return mSmooth; }

	/// Get right unit vector
	const Vec3d& ur() const { return mUR; }
	
	/// Get up unit vector
	const Vec3d& uu() const { return mUU; }
	
	/// Get forward unit vector
	const Vec3d& uf() const { return mUF; }
	
	/// Get linear and angular velocities as a Pose
	Pose vel() const {
		return Pose(mMove1, Quatd::fromEuler(mSpin1[1], mSpin1[0], mSpin1[2]));
	}

	/// Set smoothing amount [0,1)
	Nav& smooth(double v){ mSmooth=v; return *this; }
	
	void view(double azimuth, double elevation, double bank) {
		view(Quatd::fromEuler(azimuth, elevation, bank));
	}
	void view(const Quatd& v) {
		quat(v);
		updateUnitVectors();
	}
	
	void turn(const Quatd& v) {
		v.toEuler(mSpin1);
	}

	/// Set linear velocity
	void move(double dr, double du, double df) { moveR(dr); moveU(du); moveF(df); }
	void moveR(double v){ mMove0[0] = v; }
	void moveU(double v){ mMove0[1] = v; }
	void moveF(double v){ mMove0[2] = v; }

	/// Accelerate
	void push(double ddr, double ddu, double ddf) { pushR(ddr); pushU(ddu); pushF(ddf); }
	void pushR(double amount) { mMove0[0] += amount; }
	void pushU(double amount) { mMove0[1] += amount; }
	void pushF(double amount) { mMove0[2] += amount; }

	/// Set all angular velocity values from azimuth, elevation, and bank differentials
	void spin(double da, double de, double db){ spinR(de); spinU(da); spinF(db); }

	/// Set angular velocity around right vector
	void spinR(double v){ mSpin0[0] = v; }
	
	/// Set angular velocity around up vector
	void spinU(double v){ mSpin0[1] = v; }
	
	/// Set angular velocity around forward vector
	void spinF(double v){ mSpin0[2] = v; }

	/// Turn by a single increment for one step, in degrees
	void turnR(double v){ mTurn[0] = v; }
	void turnU(double v){ mTurn[1] = v; }
	void turnF(double v){ mTurn[2] = v; }
	void turn(double a, double e, double b){ turnR(e); turnU(a); turnF(b); }

	/// Stop moving and spinning
	Nav& halt(){ mMove0.set(0); mSpin0.set(0); quat().identity(); return *this; }

	/// Go to origin and reset orientation to identity
	Nav& home(){ 
		quat().identity();
		turn(0, 0, 0); 
		spin(0, 0, 0);
		vec().set(0);
		updateUnitVectors();
		return *this;
	}

	/// Update coordinate frame basis vectors based on internal quaternion
	void updateUnitVectors(){ unitVectors(mUR, mUU, mUF); }
	
	void set(const Nav& v){
		Pose::set(v);
		mMove0 = v.mMove0; mMove1 = v.mMove1;
		mSpin0 = v.mSpin0; mSpin1 = v.mSpin1;
		mTurn = v.mTurn;
		mUR = v.mUR; mUU = v.mUU; mUF = v.mUF;
		mSmooth = v.mSmooth;
	}
	
	/// Accumulate pose based on velocity
	void step(double dt=1){
		double amt = 1.-smooth();	// TODO: adjust for dt

		Vec3d angVel = mSpin0 + mTurn;
		mTurn.set(0); // turn is just a one-shot increment, so clear each frame

		// low-pass filter velocities
		mMove1.lerp(mMove0*dt, amt);
		mSpin1.lerp(angVel*dt, amt);

		mQuat *= vel().quat();
		updateUnitVectors();

		// accumulate position:
		for(int i=0; i<pos().size(); ++i){
			pos()[i] += mMove1.dot(Vec3d(ur()[i], uu()[i], uf()[i]));
		}
	}

protected:
	Vec3d mMove0, mMove1;	// linear velocities (raw, smoothed)
	Vec3d mSpin0, mSpin1;	// angular velocities (raw, smoothed)
	Vec3d mTurn;			// 
	Vec3d mUR, mUU, mUF;	// basis vectors of coordinate frame
	double mSmooth;
};



//
//class NavRef : public Nav {
//public:
//	NavRef()
//	: mParent(0)
//	{}
//
//	void parent(Nav * v){ mParent = v; }
//
//	Nav * parent(){ return mParent; }
//
//protected:
//	Nav * mParent;
//};


// Implementation --------------------------------------------------------------



} // al::

#endif
