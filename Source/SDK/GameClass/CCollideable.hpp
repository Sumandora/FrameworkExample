#ifndef SDK_COLLIDEABLE
#define SDK_COLLIDEABLE

#include "../Vector.hpp"

class CCollideable {
public:
	VIRTUAL_METHOD(1, ObbMins, Vector*, (), (this))
	VIRTUAL_METHOD(2, ObbMaxs, Vector*, (), (this))
};

#endif
