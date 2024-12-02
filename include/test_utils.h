//
// Created by tommy on 14/11/2024.
//

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

class OSMetaClassBase {
};

class OSObject : public OSMetaClassBase {
public:
	static OSObject* create() { return new OSObject(); }
	void release() {}
	void retain() {}
};

#endif //TEST_UTILS_H
