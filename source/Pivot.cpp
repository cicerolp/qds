#include "stdafx.h"
#include "Pivot.h"
#include "BinnedPivot.h"

inline Pivot::operator BinnedPivot() const {
   return BinnedPivot(*this);
}