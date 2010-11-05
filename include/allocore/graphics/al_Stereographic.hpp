#ifndef INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP
#define INCLUDE_AL_GRAPHICS_STEREOGRAPHIC_HPP

/*
 *  A collection of functions and classes related to application mainloops
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
*/

#include "allocore/protocol/al_Graphics.hpp"
#include "allocore/spatial/al_Camera.hpp"

namespace al {
namespace gfx{


struct Viewport {
	int b, l, w, h;	///! bottom, left, width, height
	
	Viewport(int l, int b, int w, int h) : l(l), b(b), w(w), h(h) {}
	Viewport(int w, int h) : b(0), l(0), w(w), h(h) {}
	
	double aspect() { return w/(double)h; }
};


///	Higher-level utility class to manage various stereo rendering techniques
class Stereographic {
public:
	enum StereoMode{
		Anaglyph=0,	/**< Red (left eye) / cyan (right eye) stereo */
		Active,		/**< Active quad-buffered stereo */
		Dual,		/**< Dual side-by-side stereo */
		LeftEye,	/**< Left eye only */
		RightEye	/**< Right eye only */
	};
	enum AnaglyphMode {
		RedBlue = 0,
		RedGreen,
		RedCyan,
		BlueRed,
		GreenRed,
		CyanRed
	};
	
	Stereographic() 
	: mMode(Anaglyph), mAnaglyphMode(RedBlue), mStereo(false) {}
	~Stereographic() {}

	///< draw the scene according to the stored stereographic mode
	void draw(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	
	/// So many different ways to draw :-)
	void drawMono(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	void drawActive(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	void drawAnaglyph(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	void drawDual(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	void drawLeft(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	void drawRight(Graphics& gl, Camera& cam, Pose& pose, Viewport& viewport, Drawable& draw);
	
	/// Blue line sync for active stereo (for those projectors that need it)
	/// add this call at the end of rendering (just before the swap buffers call)
	void drawBlueLine(double window_width, double window_height);
	
	Stereographic& mode(StereoMode v){ mMode=v; return *this; }	///< Set stereographic mode
	Stereographic& stereo(bool v){ mStereo=v; return *this; }		///< Set stereographic active
	Stereographic& anaglyphMode(AnaglyphMode v) { mAnaglyphMode=v; return *this; }	///< set glasses type
	
	StereoMode mode() const { return mMode; }				///< Get stereographic mode
	bool stereo() const { return mStereo; }					///< Get stereographic active
	AnaglyphMode anaglyphMode() const { return mAnaglyphMode; }	///< get anaglyph glasses type
	
protected:
	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;
	bool mStereo;
};

} // gfx::
} // al::

#endif /* include guard */
