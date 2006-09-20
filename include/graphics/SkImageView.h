#ifndef SkImageView_DEFINED
#define SkImageView_DEFINED

#include "SkView.h"
#include "SkString.h"

class SkAnimator;
class SkBitmap;
struct SkMatrix;

class SkImageView : public SkView {
public:
			SkImageView();
	virtual	~SkImageView();

	void	getUri(SkString*) const;
	void	setUri(const char []);
	void	setUri(const SkString&);
	

	enum ScaleType {
		kMatrix_ScaleType,
		kFitXY_ScaleType,
		kFitStart_ScaleType,
		kFitCenter_ScaleType,
		kFitEnd_ScaleType
	};
	ScaleType	getScaleType() const { return (ScaleType)fScaleType; }
	void		setScaleType(ScaleType);
	
	bool	getImageMatrix(SkMatrix*) const;
	void	setImageMatrix(const SkMatrix*);

protected:
	// overrides
	virtual bool	onEvent(const SkEvent&);
	virtual void	onDraw(SkCanvas*);
	virtual void	onInflate(const SkDOM&, const SkDOMNode*);
	
private:
	SkString	fUri;
	SkMatrix*	fMatrix;	// nil or copy of caller's matrix ,,,,,
	union {
		SkAnimator*	fAnim;
		SkBitmap* fBitmap;
	} fData;
	U8			fScaleType;
	SkBool8		fDataIsAnim;	// as opposed to bitmap
	SkBool8		fUriIsValid;
	
	void	onUriChange();
	bool	getDataBounds(SkRect* bounds);
	bool	freeData();
	bool	ensureUriIsLoaded();

	typedef SkView INHERITED;
};

#endif
