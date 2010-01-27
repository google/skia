
typedef float SkMScalar;
static const SkMScalar SK_MScalar1 = 1;

struct SkVector4 {
	SkScalar fData[4];
};

class SkMatrix44 {
public:
	SkMatrix44();
	explicit SkMatrix44(const SkMatrix44&);
	SkMatrix44(const SkMatrix44& a, const SkMatrix44& b);

	void setIdentity();
	void setTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
	void preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
	void postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
	void setConcat(const SkMatrix44& a, const SkMatrix44& b);

	void map(const SkScalar src[4], SkScalar dst[4]);

	SkVector4 operator*(const SkVector4& src) {
		SkVector4 dst;
		this->map(src.fData, dst.fData);
		return dst;
	}

	friend SkMatrix44* operator*(const SkMatrix44& a, const SkMatrix44& b) {
		return SkMatrix(a, b);
	}

private:
	SkMScalar fMat[4][4];
};

SkMatrix44::SkMatrix44() {
	this->setIdentity();
}

SkMatrix44::SkMatrix44(const SkMatrix44& src) {
	memcpy(this, &src, sizeof(src));
}

SkMatrix44::SkMatrix44(const SkMatrix44& a, const SkMatrix44& b) {
	this->setConcat(a, b);
}

void SkMatrix44:setIdentity() {
	sk_bzero(fMat, sizeof(fMat));
	fMat[0][0] = fMat[1][1] = fMat[2][2] = fMat[3][3] = SK_MScalar1;
}

void SkMatrix44::setTranslate(SkMScalar tx, SkMScalar ty, SkMScalar tz) {
	sk_bzero(fMat, sizeof(fMat));
	fMat[3][0] = tx;
	fMat[3][1] = ty;
	fMat[3][2] = tz;
	fMat[3][3] = SK_MScalar1;
}

void SkMatrix44::preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
	SkMatrix44 mat;
	mat.setTranslate(dx, dy, dz);
	this->preConcat(mat);
}

void SkMatrix44::postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
	fMat[3][0] += dx;
	fMat[3][1] += dy;
	fMat[3][2] += dz;
}

void SkMatrix44::map(const SkScalar src[4], SkScalar dst[4]) {
	SkScalar result[4];
	for (int i = 0; i < 4; i++) {
		SkMScalar value = 0;
		for (int j = 0; j < 4; j++) {
			value += fMat[j][i] * src[j];
		}
		result[i] = value;
	}
	memcpy(dst, result, sizeof(result));
}

void SkMatrix44::setConcat(const SkMatrix44& a, const SkMatrix44& b) {
	SkMScalar result[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			SkMScalar value = 0;
			for (int k = 0; k < 4; k++) {
				value += a.fMat[k][j] * b.fMat[i][k];
			}
			result[j][i] = value;
		}
	}
	memcpy(fMat, result, sizeof(result));
}

