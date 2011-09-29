#ifndef INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP
#define INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Utilities for rendering Graphics in various stereographic modes

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/spatial/al_Camera.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

/// A framed area on a display screen
struct Viewport {
	float l, b, w, h;	///< left, bottom, width, height

	Viewport(float w=800, float h=600) : l(0), b(0), w(w), h(h) {}
	Viewport(float l, float b, float w, float h) : l(l), b(b), w(w), h(h) {}

	/// Get aspect ratio (width divided by height)
	float aspect() const { return (h!=0 && w!=0) ? float(w)/h : 1; }

	/// Set dimensions
	void set(float l_, float b_, float w_, float h_){ l=l_; b=b_; w=w_; h=h_; }
};


///	Higher-level utility class to manage various stereo rendering techniques
class Stereographic {
public:

	/// Stereographic mode
	enum StereoMode{
		ANAGLYPH=0,		/**< Red (left eye) / cyan (right eye) stereo */
		ACTIVE,			/**< Active quad-buffered stereo */
		DUAL,			/**< Dual side-by-side stereo */
		LEFT_EYE,		/**< Left eye only */
		RIGHT_EYE		/**< Right eye only */
	};
	
	/// Anaglyph mode
	enum AnaglyphMode {
		RED_BLUE = 0,	/**< */
		RED_GREEN,		/**< */
		RED_CYAN,		/**< */
		BLUE_RED,		/**< */
		GREEN_RED,		/**< */
		CYAN_RED		/**< */
	};
	
	Stereographic() 
	: mMode(ANAGLYPH), mAnaglyphMode(RED_CYAN), mClearColor(Color(0)), mStereo(false), mOmni(0), mSlices(24), mOmniFov(360) {}

	~Stereographic() {}

	/// Draw the scene according to the stored stereographic mode
	void draw			(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	
	/// So many different ways to draw :-)
	void drawMono		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawActive		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawAnaglyph	(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawDual		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawLeft		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	void drawRight		(Graphics& gl, const Camera& cam, const Pose& pose, const Viewport& vp, Drawable& draw);
	
	/// Blue line sync for active stereo (for those projectors that need it)
	/// add this call at the end of rendering (just before the swap buffers call)
	void drawBlueLine(double window_width, double window_height);
	
	Stereographic& clearColor(const Color& v){ mClearColor=v; return *this; }	///< Set background clear color
	Stereographic& mode(StereoMode v){ mMode=v; return *this; }					///< Set stereographic mode
	Stereographic& stereo(bool v){ mStereo=v; return *this; }					///< Set stereographic active
	Stereographic& anaglyphMode(AnaglyphMode v) { mAnaglyphMode=v; return *this; }	///< set glasses type
	/// Set omnigraphic mode
	/// slices: sets number of sub-viewport slices to render
	/// fov (degrees) sets field of view (horizontal)
	/// NOTE: cam.fovy will be ignored in omni mode
	Stereographic& omni(bool enable) { mOmni = enable; return *this; }
	Stereographic& omni(bool enable, unsigned slices, double fov=360){
		mOmni = enable; mSlices = slices; mOmniFov = fov; return *this; }
	Stereographic& omniFov( double fov ) { mOmniFov = fov; return *this; }
	Stereographic& omniSlices( int slices ) { mSlices = slices; return *this; }

	const Color& clearColor() const { return mClearColor; }		///< Get background clear color
	StereoMode mode() const { return mMode; }					///< Get stereographic mode
	bool stereo() const { return mStereo; }						///< Get stereographic active
	AnaglyphMode anaglyphMode() const { return mAnaglyphMode; }	///< get anaglyph glasses type
	bool omni() const { return mOmni; }
	
	// These accessors will be valid only during the Drawable's onDraw() event
	// they can be useful to simulate the OpenGL pipeline transforms
	//	e.g. Matrix4d::multiply(Vec4d eyespace, stereo.modelView(), Vec4d objectspace); 
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.projection(), Vec4d eyespace);
	//	e.g. Matrix4d::multiply(Vec4d clipspace, stereo.modelViewProjection(), Vec4d objectspace);
	// to convert in the opposite direction, use Matrix4::inverse(). 
	const Matrix4d& modelView() const { return mModelView; }
	const Matrix4d& projection() const { return mProjection; }
	Matrix4d modelViewProjection() const { return mProjection * mModelView; }
	
protected:
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	Color mClearColor;
	bool mStereo;
	bool mOmni;
	unsigned mSlices;	// number of omni slices
	double mOmniFov;	// field of view of omnigraphics
	
	Matrix4d mProjection, mModelView;
};

} // al::

#endif
